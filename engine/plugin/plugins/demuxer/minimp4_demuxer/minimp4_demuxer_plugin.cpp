/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "Minimp4DemuxerPlugin"

#include "minimp4_demuxer_plugin.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <new>

#include <securec.h>
#include "core/plugin_manager.h"
#include "foundation/log.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/plugin_time.h"
#include "utils/constants.h"
#include "utils/memory_helper.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace {
std::vector<int> sampleRateVec {
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350
};

constexpr int8_t ADTS_HEADER_SIZE = 7;
constexpr int8_t MP4_HEADER_OFFSET = 4;
constexpr int8_t RANK_MAX = 100;
constexpr uint32_t DEFAULT_AUDIO_SAMPLE_PER_FRAME = 1024;

int ReadCallback(int64_t offset, void *buffer, size_t size, void *token);

int Sniff(const std::string &name, std::shared_ptr<DataSource> dataSource);

Status RegisterPlugins(const std::shared_ptr<Register> &reg);

void UpdatePluginDefinition(CodecPluginDef &definition);
}

MiniMP4DemuxerPlugin::MiniMP4DemuxerPlugin(std::string name)
    : DemuxerPlugin(std::move(name)),
      ioContext_(),
      callback_(nullptr),
      mediaInfo_(nullptr),
      fileSize_(0),
      sampleIndex_(0)
{
    memset_s(&miniMP4_, sizeof(MP4D_demux_t), 0, sizeof(MP4D_demux_t));
    MEDIA_LOG_I("MiniMP4DemuxerPlugin, plugin name: %s", pluginName_.c_str());
}

MiniMP4DemuxerPlugin::~MiniMP4DemuxerPlugin()
{
    MEDIA_LOG_I("~MiniMP4DemuxerPlugin");
}

Status MiniMP4DemuxerPlugin::SetDataSource(const std::shared_ptr<DataSource> &source)
{
    ioContext_.dataSource = source;
    source->GetSize(fileSize_);
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::Init()
{
    MEDIA_LOG_I("Init called");
    Reset();
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::Deinit()
{
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::Prepare()
{
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::Reset()
{
    MP4D_close(&miniMP4_);
    ioContext_.offset = 0;
    ioContext_.eos = false;
    mediaInfo_.reset();
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::Start()
{
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::Stop()
{
    return Status::OK;
}

bool MiniMP4DemuxerPlugin::IsParameterSupported(Tag tag)
{
    (void)tag;
    return false;
}

Status MiniMP4DemuxerPlugin::GetParameter(Tag tag, ValueType &value)
{
    (void)tag;
    (void)value;
    return Status::ERROR_UNIMPLEMENTED;
}

Status MiniMP4DemuxerPlugin::SetParameter(Tag tag, const ValueType &value)
{
    (void)tag;
    (void)value;
    return Status::ERROR_UNIMPLEMENTED;
}


std::shared_ptr<Allocator> MiniMP4DemuxerPlugin::GetAllocator()
{
    return nullptr;
}

Status MiniMP4DemuxerPlugin::SetCallback(const std::shared_ptr<Callback> &cb)
{
    callback_ = cb;
    return Status::OK;
}

size_t MiniMP4DemuxerPlugin::GetTrackCount()
{
    size_t trackCnt = 0;
    if (mediaInfo_) {
        trackCnt = mediaInfo_->tracks.size();
    }
    return trackCnt;
}

Status MiniMP4DemuxerPlugin::SelectTrack(int32_t trackId)
{
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::UnselectTrack(int32_t trackId)
{
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::GetSelectedTracks(std::vector<int32_t> &trackIds)
{
    trackIds.clear();
    trackIds.push_back(1);
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::GetMediaInfo(MediaInfo &mediaInfo)
{
    if (fileSize_ == 0 || ioContext_.dataSource == nullptr) {
        return Status::ERROR_UNKNOWN;
    }
    MemoryHelper::make_unique<MediaInfo>().swap(mediaInfo_);
    mediaInfo_->general.clear();
    mediaInfo_->tracks.clear();

    if (MP4D_open(&miniMP4_, ReadCallback, reinterpret_cast<void *>(this), fileSize_) == 0) {
        MEDIA_LOG_E("MP4D_open IS ERROR");
        mediaInfo_ = nullptr;
        return Status::ERROR_MISMATCHED_TYPE;
    }
    if (AudioAdapterForDecoder() != Status::OK) {
        mediaInfo_ = nullptr;
        return Status::ERROR_UNKNOWN;
    }

    TagMap tagPair { { Tag::AUDIO_SAMPLE_RATE,
        static_cast<uint32_t>(miniMP4_.track->SampleDescription.audio.samplerate_hz) },
        {Tag::MEDIA_BITRATE, static_cast<int64_t>(miniMP4_.track->avg_bitrate_bps) },
        {Tag::AUDIO_CHANNELS, static_cast<uint32_t>(miniMP4_.track->SampleDescription.audio.channelcount) },
        {Tag::TRACK_ID, static_cast<uint32_t>(0) },
        {Tag::MIME, std::string(MEDIA_MIME_AUDIO_AAC) },
        {Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(4) },
        {Tag::AUDIO_AAC_PROFILE, AudioAacProfile::LC },
        {Tag::AUDIO_AAC_STREAM_FORMAT, AudioAacStreamFormat::MP4ADTS },
        {Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat::S16P },
        {Tag::AUDIO_SAMPLE_PER_FRAME, DEFAULT_AUDIO_SAMPLE_PER_FRAME },
        {Tag::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO } };
    mediaInfo_->tracks.push_back(tagPair);
    mediaInfo = *mediaInfo_;
    return Status::OK;
}

void MiniMP4DemuxerPlugin::FillADTSHead(std::shared_ptr<Memory> &data, uint32_t frameSize)
{
    uint8_t adtsHeader[ADTS_HEADER_SIZE] = {0};
    uint32_t channelConfig = miniMP4_.track->SampleDescription.audio.channelcount;
    uint32_t packetLen = frameSize + 7;
    uint32_t samplerateIndex = 0;
    /* 按格式读取信息帧 */
    uint8_t objectTypeIndication = miniMP4_.track->object_type_indication;
    samplerateIndex = ((miniMP4_.track->dsi[0] & 0x7) << 1) + (miniMP4_.track->dsi[1] >> 7); // 1,7 按协议取信息帧
    adtsHeader[0] = static_cast<uint8_t>(0xFF);
    adtsHeader[1] = static_cast<uint8_t>(0xF1);
    adtsHeader[2] = static_cast<uint8_t>(objectTypeIndication) + (samplerateIndex << 2) + (channelConfig >> 2); // 2
    adtsHeader[3] = static_cast<uint8_t>(((channelConfig & 0x3) << 6) + (packetLen >> 11)); // 3,6,11 按协议取信息帧
    adtsHeader[4] = static_cast<uint8_t>((packetLen & 0x7FF) >> 3); // 4, 3 按协议取信息帧
    adtsHeader[5] = static_cast<uint8_t>(((packetLen & 0x7) << 5) + 0x1F); // 5 按协议取信息帧
    adtsHeader[6] = static_cast<uint8_t>(0xFC); // 6 按协议取信息帧
    data->Write(adtsHeader, ADTS_HEADER_SIZE, 0);
}

Status MiniMP4DemuxerPlugin::ReadFrame(Buffer &info, int32_t timeOutMs)
{
    if (!mediaInfo_) {
        return Status::ERROR_INVALID_PARAMETER;
    }

    if (sampleIndex_ >= miniMP4_.track->sample_count) {
        return Status::END_OF_STREAM;
    }
    uint32_t frameSize = 0;
    uint32_t timeStamp = 0;
    uint32_t duration = 0;
    uint64_t offset = MP4D_frame_offset(&miniMP4_, 0, sampleIndex_, &frameSize, &timeStamp, &duration);
    if (offset > fileSize_) {
        return Status::ERROR_UNKNOWN;
    }
    sampleIndex_++;
    auto buffer = std::make_shared<Buffer>();
    auto bufData = buffer->AllocMemory(nullptr, frameSize);
    if (bufData == nullptr) {
        return Status::ERROR_UNKNOWN;
    }
    auto result = ioContext_.dataSource->ReadAt(offset, buffer, static_cast<size_t>(frameSize));
    if (result != Status::OK) {
        return Status::ERROR_UNKNOWN;
    }
    auto data = info.AllocMemory(nullptr, frameSize + ADTS_HEADER_SIZE);
    auto writePtr = bufData->GetWritableData(frameSize);
    if ((data != nullptr) && (writePtr != nullptr)) {
        FillADTSHead(data, frameSize);
        size_t writeSize = data->Write(writePtr, frameSize, ADTS_HEADER_SIZE);
        ASSERT_CONDITION(writeSize == frameSize, "copy data failed");
    }
    return Status::OK;
}

Status MiniMP4DemuxerPlugin::SeekTo(int32_t trackId, int64_t hstTime, SeekMode mode)
{
    uint32_t frameSize = 0;
    uint32_t timeStamp = 0;
    uint32_t duration = 0;
    uint64_t offsetStart = MP4D_frame_offset(&miniMP4_, 0, 0, &frameSize, &timeStamp, &duration);
    uint64_t offsetEnd =
        MP4D_frame_offset(&miniMP4_, 0, miniMP4_.track->sample_count - 1, &frameSize, &timeStamp, &duration);
    uint64_t targetPos = (Plugin::HstTime2Us(hstTime) * static_cast<int64_t>(miniMP4_.track->avg_bitrate_bps)) / 8 +
        offsetStart;
    if (targetPos >= offsetEnd) {
        sampleIndex_ = miniMP4_.track->sample_count;
        return Status::OK;
    }
    sampleIndex_ = 0;
    uint64_t tempPos = 0;
    while (sampleIndex_ < miniMP4_.track->sample_count) {
        tempPos = MP4D_frame_offset(&miniMP4_, 0, sampleIndex_, &frameSize, &timeStamp, &duration);
        if (tempPos <= targetPos) {
            sampleIndex_++;
        } else {
            break;
        }
    }
    return Status::OK;
}

size_t MiniMP4DemuxerPlugin::GetFileSize()
{
    return fileSize_;
}

std::shared_ptr<DataSource> MiniMP4DemuxerPlugin::GetInputBuffer()
{
    return ioContext_.dataSource;
}

Status MiniMP4DemuxerPlugin::AudioAdapterForDecoder()
{
    if (miniMP4_.track == nullptr) {
        return Status::ERROR_UNKNOWN;
    }
    /* 适配解码协议 */
    size_t sampleRateIndex = (static_cast<uint32_t>(miniMP4_.track->dsi[0] & 0x7) << 1) +
        (static_cast<uint32_t>(miniMP4_.track->dsi[1]) >> 7);

    if ((sampleRateVec.size() <= sampleRateIndex) || (miniMP4_.track->dsi_bytes >= 20)) { // 20 按协议适配解码器
        return Status::ERROR_MISMATCHED_TYPE;
    }
    miniMP4_.track->SampleDescription.audio.samplerate_hz = sampleRateVec[sampleRateIndex];
    miniMP4_.track->SampleDescription.audio.channelcount = (miniMP4_.track->dsi[1] & 0x7F) >> 3; // 3 按协议适配解码器
    return Status::OK;
}
namespace {
int ReadCallback(int64_t offset, void *buffer, size_t size, void *token)
{
    MiniMP4DemuxerPlugin *mp4Demuxer = (MiniMP4DemuxerPlugin *)token;
    uint32_t file_size = mp4Demuxer->GetFileSize();
    if (offset >= file_size) {
        MEDIA_LOG_E("ReadCallback offset is bigger");
        return -1;
    }
    std::shared_ptr<DataSource> inputBuffer = mp4Demuxer->GetInputBuffer();
    auto bufferObj = std::make_shared<Buffer>();
    auto bufData = bufferObj->WrapMemory(reinterpret_cast<uint8_t*>(buffer), size, size);
    if (inputBuffer->ReadAt(offset, bufferObj, size) != Status::OK) {
        return -1;
    }
    return 0;
}

int Sniff(const std::string &name, std::shared_ptr<DataSource> dataSource)
{
    unsigned char m4aCheck[] = {'f', 't', 'y', 'p'};
    auto buffer = std::make_shared<Buffer>();
    auto bufData = buffer->AllocMemory(nullptr, sizeof(m4aCheck));
    if (dataSource->ReadAt(MP4_HEADER_OFFSET, buffer, static_cast<size_t>(sizeof(m4aCheck))) != Status::OK) {
        return 0;
    }
    if (memcmp(bufData->GetReadOnlyData(), &m4aCheck, sizeof(m4aCheck)) != 0) {
        MEDIA_LOG_E("memcmp m4aCheck is error");
        return 0;
    }
    return RANK_MAX;
}

Status RegisterPlugins(const std::shared_ptr<Register> &reg)
{
    MEDIA_LOG_D("RegisterPlugins called");
    if (!reg) {
        MEDIA_LOG_E("RegisterPlugins fail due to null pointer for reg");
        return Status::ERROR_INVALID_PARAMETER;
    }
    std::string pluginName = "MiniMP4DemuxerPlugin";
    DemuxerPluginDef regInfo;
    regInfo.name = pluginName;
    regInfo.description = "adapter for minimp4 demuxer plugin";
    regInfo.rank = RANK_MAX;
    regInfo.creator = [](const std::string &name) -> std::shared_ptr<DemuxerPlugin> {
        return std::make_shared<MiniMP4DemuxerPlugin>(name);
    };
    regInfo.sniffer = Sniff;
    auto ret = reg->AddPlugin(regInfo);
    if (ret != Status::OK) {
        MEDIA_LOG_E("RegisterPlugin AddPlugin failed with return %d", static_cast<int>(ret));
    }
    return Status::OK;
}
}
PLUGIN_DEFINITION(MiniMP4Demuxer, LicenseType::CC0, RegisterPlugins, [] {});
} // namespace Plugin
} // namespace Media
} // namespace OHOS