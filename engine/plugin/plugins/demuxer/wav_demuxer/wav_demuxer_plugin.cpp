/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define HST_LOG_TAG "WavDemuxerPlugin"

#include "wav_demuxer_plugin.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <new>
#include "foundation/log.h"
#include "osal/thread/scoped_lock.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/plugin_time.h"
#include "osal/utils/util.h"
#include "utils/constants.h"

struct WavHeadAttr {
    uint8_t  chunkID[4];     // 4 byte RIFF Chunk descriptor
    uint32_t chunkSize;      // 4 byte RIFF Chunk Size
    uint8_t  format[4];      // 4 byte The format concern here is "WAVE",
    // which requires two or three sub-chunks:"fmt", "fact"(optional),and "data"

    uint8_t  subChunk1ID[4]; // 4 byte The "fmt" sub-chunk describes the sound information in the data sub-chunk
    uint32_t subChunk1Size;  // 4 byte Size of the fmt chunk  16 or 18  18 stand for has auxiliary information
    uint16_t audioFormat;    // 2 byte Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t numChannels;    // 2 byte Number of channels 1=Mono 2=Sterio
    uint32_t sampleRate;     // 4 byte Sampling Frequency in Hz
    uint32_t byteRate;       // 4 byte bytes per second
    uint16_t blockAlign;     // 2 byte 2=16-bit mono, 4=16-bit stereo
    uint16_t bitsPerSample;  // 2 byte Number of bits per sample

    uint8_t  subChunk2ID[4]; // 4 byte (optional) The "fact" sub-chunk exists in non-PCM format
    uint32_t subChunk2Size;  // 4 byte (optional)
    uint32_t dataFactSize;   // 4 byte (optional)

    uint8_t  subChunk3ID[4]; // 4 byte The "data" sub-chunk indicates the size of the sound information
    uint32_t subChunk3Size;  // 4 byte Sampled data length
}; // 根据wav协议构建的结构体，轻易勿动

namespace OHOS {
namespace Media {
namespace Plugin {
namespace WavPlugin {
namespace {
constexpr uint8_t  MAX_RANK = 100;
constexpr uint8_t  PROBE_READ_LENGTH  = 4;
constexpr uint32_t WAV_PER_FRAME_SIZE = 8192;
constexpr uint32_t WAV_HEAD_INFO_LEN = sizeof(WavHeadAttr);
bool WavSniff(const uint8_t *inputBuf);
std::map<uint32_t, AudioSampleFormat> g_WavAudioSampleFormatPacked = {
    {8, AudioSampleFormat::U8},
    {16, AudioSampleFormat::S16},
    {32, AudioSampleFormat::S32},
};

enum class WavAudioFormat {
    WAVE_FORMAT_PCM = 0x0001,
    WAVE_FORMAT_IEEE_FLOAT = 0x0003,
    WAVE_FORMAT_ALAW = 0x0006,
    WAVE_FORMAT_MULAW = 0x0007,
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
};
int Sniff(const std::string& pluginName, std::shared_ptr<DataSource> dataSource);
Status RegisterPlugin(const std::shared_ptr<Register>& reg);
}

WavDemuxerPlugin::WavDemuxerPlugin(std::string name)
    : DemuxerPlugin(std::move(name)),
      fileSize_(0),
      ioContext_(),
      dataOffset_(0),
      isSeekable_(true)
{
    MEDIA_LOG_I("WavDemuxerPlugin, plugin name: " PUBLIC_LOG_S, pluginName_.c_str());
}

WavDemuxerPlugin::~WavDemuxerPlugin()
{
    MEDIA_LOG_I("~WavDemuxerPlugin");
}

Status WavDemuxerPlugin::SetDataSource(const std::shared_ptr<DataSource>& source)
{
    ioContext_.dataSource = source;
    if (ioContext_.dataSource != nullptr) {
        ioContext_.dataSource->GetSize(fileSize_);
    }
    if (fileSize_ == 0) {
        isSeekable_ = false;
    }
    MEDIA_LOG_I("fileSize_ " PUBLIC_LOG_ZU, fileSize_);
    return Status::OK;
}

Status WavDemuxerPlugin::GetMediaInfo(MediaInfo& mediaInfo)
{
    auto buffer = std::make_shared<Buffer>();
    auto bufData = buffer->AllocMemory(nullptr, WAV_HEAD_INFO_LEN);
    Status status = ioContext_.dataSource->ReadAt(0, buffer, WAV_HEAD_INFO_LEN);
    if (status != Status::OK) {
        return status;
    }
    uint8_t *dataPtr = const_cast<uint8_t *>(bufData->GetReadOnlyData());
    WavHeadAttr *wavHeader = reinterpret_cast<WavHeadAttr *>(dataPtr);
    if (wavHeader == nullptr) {
        return Status::ERROR_UNKNOWN;
    }
    dataOffset_  = WAV_HEAD_INFO_LEN;
    if (wavHeader->audioFormat == static_cast<uint16_t>(WavAudioFormat::WAVE_FORMAT_PCM)) {
        dataOffset_ -= 12; // 12 = subChunk2ID(optional)+subChunk2Size(optional)+dataFactSize(optional)
    }
    MEDIA_LOG_D("wavHeadLength_ " PUBLIC_LOG_U32, dataOffset_);
    mediaInfo.tracks.resize(1);
    if (wavHeader->numChannels == 1) {
        mediaInfo.tracks[0].insert({Tag::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::MONO});
    } else {
        mediaInfo.tracks[0].insert({Tag::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO});
    }
    mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_RATE, static_cast<uint32_t>(wavHeader->sampleRate)});
    mediaInfo.tracks[0].insert({Tag::MEDIA_BITRATE, static_cast<int64_t>(wavHeader->byteRate) * 8}); // 8  byte to bit
    mediaInfo.tracks[0].insert({Tag::AUDIO_CHANNELS, static_cast<uint32_t>(wavHeader->numChannels)});
    mediaInfo.tracks[0].insert({Tag::TRACK_ID, static_cast<uint32_t>(0)});
    mediaInfo.tracks[0].insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_RAW)});
    mediaInfo.tracks[0].insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(1)});
    mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_PER_FRAME, WAV_PER_FRAME_SIZE});
    if (wavHeader->audioFormat == static_cast<uint16_t>(WavAudioFormat::WAVE_FORMAT_PCM)
        || wavHeader->audioFormat == static_cast<uint16_t>(WavAudioFormat::WAVE_FORMAT_EXTENSIBLE)) {
        mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_FORMAT,
                                    g_WavAudioSampleFormatPacked[static_cast<uint32_t>(wavHeader->bitsPerSample)]});
    } else if (wavHeader->audioFormat == static_cast<uint16_t>(WavAudioFormat::WAVE_FORMAT_IEEE_FLOAT)){
        mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat::F32});
    } else {
        mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat::NONE});
    }
    mediaInfo.tracks[0].insert({Tag::BITS_PER_CODED_SAMPLE, static_cast<uint32_t>(wavHeader->bitsPerSample)});
    return Status::OK;
}

Status WavDemuxerPlugin::ReadFrame(Buffer& outBuffer, int32_t timeOutMs)
{
    std::shared_ptr<Buffer> outBufferPtr(&outBuffer, [](Buffer *) {});
    if (outBuffer.IsEmpty()) {
        outBuffer.AllocMemory(nullptr, WAV_PER_FRAME_SIZE);
    }
    Status retResult = ioContext_.dataSource->ReadAt(dataOffset_, outBufferPtr, WAV_PER_FRAME_SIZE);
    dataOffset_ +=  outBuffer.GetMemory()->GetSize();
    if (retResult != Status::OK) {
        MEDIA_LOG_E("Read Data Error");
    }
    return retResult;
}

Status WavDemuxerPlugin::SeekTo(int32_t trackId, int64_t hstTime, SeekMode mode)
{
    if (fileSize_ <= 0) {
        return Status::ERROR_INVALID_OPERATION;
    }
    return Status::ERROR_UNIMPLEMENTED;
}

Status WavDemuxerPlugin::Reset()
{
    dataOffset_ = 0;
    fileSize_ = 0;
    isSeekable_ = true;
    return Status::OK;
}

Status WavDemuxerPlugin::GetParameter(Tag tag, ValueType &value)
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status WavDemuxerPlugin::SetParameter(Tag tag, const ValueType &value)
{
    return Status::ERROR_UNIMPLEMENTED;
}

std::shared_ptr<Allocator> WavDemuxerPlugin::GetAllocator()
{
    return nullptr;
}

Status WavDemuxerPlugin::SetCallback(Callback* cb)
{
    return Status::OK;
}

size_t WavDemuxerPlugin::GetTrackCount()
{
    return 0;
}
Status WavDemuxerPlugin::SelectTrack(int32_t trackId)
{
    return Status::OK;
}
Status WavDemuxerPlugin::UnselectTrack(int32_t trackId)
{
    return Status::OK;
}
Status WavDemuxerPlugin::GetSelectedTracks(std::vector<int32_t>& trackIds)
{
    return Status::OK;
}

namespace {
bool WavSniff(const uint8_t *inputBuf)
{
    // 解析数据起始位置的值，判断是否为wav格式文件
    return ((inputBuf[0] != 'R') || (inputBuf[1] != 'I') || (inputBuf[2] != 'F') || (inputBuf[3] != 'F')); // 0 1 2 3
}
int Sniff(const std::string& name, std::shared_ptr<DataSource> dataSource)
{
    MEDIA_LOG_I("Sniff in");
    Status status = Status::ERROR_UNKNOWN;
    auto buffer = std::make_shared<Buffer>();
    auto bufData = buffer->AllocMemory(nullptr, PROBE_READ_LENGTH);
    status = dataSource->ReadAt(0, buffer, PROBE_READ_LENGTH);
    if (status != Status::OK) {
        MEDIA_LOG_E("Sniff Read Data Error");
        return 0;
    }
    if (WavSniff(bufData->GetReadOnlyData())) {
        return 0;
    }
    return MAX_RANK;
}

Status RegisterPlugin(const std::shared_ptr<Register>& reg)
{
    MEDIA_LOG_I("RegisterPlugin called.");
    if (!reg) {
        MEDIA_LOG_I("RegisterPlugin failed due to nullptr pointer for reg.");
        return Status::ERROR_INVALID_PARAMETER;
    }

    std::string pluginName = "WavDemuxerPlugin";
    DemuxerPluginDef regInfo;
    regInfo.name = pluginName;
    regInfo.description = "adapter for wav demuxer plugin";
    regInfo.rank = MAX_RANK;
    regInfo.creator = [](const std::string &name) -> std::shared_ptr<DemuxerPlugin> {
        return std::make_shared<WavDemuxerPlugin>(name);
    };
    regInfo.sniffer = Sniff;
    auto rtv = reg->AddPlugin(regInfo);
    if (rtv != Status::OK) {
        MEDIA_LOG_I("RegisterPlugin AddPlugin failed with return " PUBLIC_LOG_D32, static_cast<int>(rtv));
    }
    return Status::OK;
}
}

PLUGIN_DEFINITION(WavDemuxer, LicenseType::APACHE_V2, RegisterPlugin, [] {});
} // namespace WavPlugin
} // namespace Plugin
} // namespace Media
} // namespace OHOS
