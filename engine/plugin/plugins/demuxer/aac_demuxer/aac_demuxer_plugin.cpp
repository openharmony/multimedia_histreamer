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

#define HST_LOG_TAG "AACDemuxerPlugin"

#include "aac_demuxer_plugin.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <new>
#include <securec.h>
#include "core/plugin_manager.h"
#include "foundation/log.h"
#include "osal/thread/scoped_lock.h"
#include "plugin/common/plugin_buffer.h"
#include "constants.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace {
    constexpr uint32_t PROBE_READ_LENGTH = 2;
    constexpr uint32_t GET_INFO_READ_LEN = 7;
    constexpr uint32_t MEDIA_IO_SIZE = 2048;
    int samplingRateMap[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};
    int IsAACPattern(const uint8_t *data);
    int Sniff(const std::string& name, std::shared_ptr<DataSource> dataSource);
    Status RegisterPlugin(const std::shared_ptr<Register>& reg);
    void UpdatePluginDefinition(CodecPluginDef& definition);
}

AACDemuxerPlugin::AACDemuxerPlugin(std::string name)
        : DemuxerPlugin(std::move(name)),
          ioContext_(),
          mediaIOSize_(MEDIA_IO_SIZE),
          fileSize_(0)
{
    memset_s(&aacDemuxerRst_, sizeof(aacDemuxerRst_), 0x00, sizeof(AACDemuxerRst));
    MEDIA_LOG_I("AACDemuxerPlugin, plugin name: %s", pluginName_.c_str());
}

AACDemuxerPlugin::~AACDemuxerPlugin()
{
    MEDIA_LOG_I("~AACDemuxerPlugin");
}

Status AACDemuxerPlugin::SetDataSource(const std::shared_ptr<DataSource>& source)
{
    ioContext_.dataSource = source;
    if (ioContext_.dataSource != nullptr) {
        ioContext_.dataSource->GetSize(fileSize_);
    }
    MEDIA_LOG_I("fileSize_ %d", fileSize_);
    return Status::OK;
}

Status AACDemuxerPlugin::GetMediaInfo(MediaInfo& mediaInfo)
{
    Status status = Status::ERROR_UNKNOWN;
    auto buffer = std::make_shared<Buffer>();
    auto bufData = buffer->AllocMemory(nullptr, GET_INFO_READ_LEN);
    uint8_t *inputDataPtr = nullptr;
    auto result = ioContext_.dataSource->ReadAt(ioContext_.offset, buffer, static_cast<size_t>(GET_INFO_READ_LEN));
    inputDataPtr = (uint8_t *)bufData->GetReadOnlyData();
    int ret = AudioDemuxerAACPrepare(inputDataPtr, bufData->GetSize(), &aacDemuxerRst_);

    if (ret == 0) {
        mediaInfo.tracks.resize(1);
        if (aacDemuxerRst_.frameChannels == 1) {
            mediaInfo.tracks[0].insert({Tag::AUDIO_CHANNEL_LAYOUT,      AudioChannelLayout::MONO}           );
        } else {
            mediaInfo.tracks[0].insert({Tag::AUDIO_CHANNEL_LAYOUT,      AudioChannelLayout::STEREO}         );
        }
        mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_RATE,       (uint32_t)aacDemuxerRst_.frameSampleRate}  );
        mediaInfo.tracks[0].insert({Tag::MEDIA_BITRATE,           (uint32_t)aacDemuxerRst_.frameBitrateKbps} );
        mediaInfo.tracks[0].insert({Tag::AUDIO_CHANNELS,          (uint32_t)aacDemuxerRst_.frameChannels}    );
        mediaInfo.tracks[0].insert({Tag::TRACK_ID,                (uint32_t)0}                               );
        mediaInfo.tracks[0].insert({Tag::MIME,                    std::string(MEDIA_MIME_AUDIO_AAC)}         );
        mediaInfo.tracks[0].insert({Tag::AUDIO_MPEG_VERSION,      (uint32_t)aacDemuxerRst_.mpegVersion}      );
        mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_FORMAT,     AudioSampleFormat::S16P}                   );
        mediaInfo.tracks[0].insert({Tag::AUDIO_SAMPLE_PER_FRAME,  (uint32_t)(1024)}                          );
        mediaInfo.tracks[0].insert({Tag::AUDIO_AAC_PROFILE,       AudioAacProfile::LC}                       );
        mediaInfo.tracks[0].insert({Tag::AUDIO_AAC_STREAM_FORMAT, AudioAacStreamFormat::MP4ADTS}             );
        return Status::OK;
    } else {
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
}

Status AACDemuxerPlugin::ReadFrame(Buffer& outBuffer, int32_t timeOutMs)
{
    int  status  = -1;
    std::shared_ptr<Memory> aacFrameData;
    auto buffer  = std::make_shared<Buffer>();
    auto bufData = buffer->AllocMemory(nullptr, mediaIOSize_);
    auto result  = ioContext_.dataSource->ReadAt(ioContext_.offset, buffer, static_cast<size_t>(mediaIOSize_));
    if (result != Status::OK) {
        ioContext_.eos = true;
        ioContext_.offset = 0;
        MEDIA_LOG_I("result is %d", result);
        return result;
    }

    uint8_t *inputPtr = (uint8_t *)bufData->GetReadOnlyData();
    status = AudioDemuxerAACProcess(inputPtr, bufData->GetSize(), &aacDemuxerRst_);

    if (outBuffer.IsEmpty()) {
        aacFrameData = outBuffer.AllocMemory(nullptr, aacDemuxerRst_.frameLength);
    } else {
        aacFrameData = outBuffer.GetMemory();
    }
    switch(status) {
        case 0:
            ioContext_.offset += aacDemuxerRst_.usedInputLength;
            aacFrameData->Write(aacDemuxerRst_.frameBuffer, aacDemuxerRst_.frameLength);
            if (aacDemuxerRst_.frameBuffer) {
                free(aacDemuxerRst_.frameBuffer);
                aacDemuxerRst_.frameBuffer = nullptr;
            }
            break;
        case -1:
        default:
            if (aacDemuxerRst_.frameBuffer) {
                free(aacDemuxerRst_.frameBuffer);
                aacDemuxerRst_.frameBuffer = nullptr;
            }
            return Status::ERROR_UNKNOWN;
    }

    return Status::OK;
}

Status AACDemuxerPlugin::SeekTo(int32_t trackId, int64_t timeStampUs, SeekMode mode)
{
    return Status::OK;
}

Status AACDemuxerPlugin::Init()
{
    return Status::OK;
}
Status AACDemuxerPlugin::Deinit()
{
    return Status::OK;
}

Status AACDemuxerPlugin::Prepare()
{
    return Status::OK;
}

Status AACDemuxerPlugin::Reset()
{
    ioContext_.eos = false;
    ioContext_.dataSource.reset();
    ioContext_.offset = 0;
    return Status::OK;
}

Status AACDemuxerPlugin::Start()
{
    return Status::OK;
}

Status AACDemuxerPlugin::Stop()
{
    return Status::OK;
}

bool AACDemuxerPlugin::IsParameterSupported(Tag tag)
{
    return false;
}

Status AACDemuxerPlugin::GetParameter(Tag tag, ValueType &value)
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status AACDemuxerPlugin::SetParameter(Tag tag, const ValueType &value)
{
    return Status::ERROR_UNIMPLEMENTED;
}

std::shared_ptr<Allocator> AACDemuxerPlugin::GetAllocator()
{
    return nullptr;
}

Status AACDemuxerPlugin::SetCallback(const std::shared_ptr<Callback>& cb)
{
    return Status::OK;
}

size_t AACDemuxerPlugin::GetTrackCount()
{
    return 0;
}

Status AACDemuxerPlugin::SelectTrack(int32_t trackId)
{
    return Status::OK;
}

Status AACDemuxerPlugin::UnselectTrack(int32_t trackId)
{
    return Status::OK;
}

Status AACDemuxerPlugin::GetSelectedTracks(std::vector<int32_t>& trackIds)
{
    return Status::OK;
}

int AACDemuxerPlugin::getFrameLength(const uint8_t *data)
{
    return ((data[3] & 0x03) << 11) | (data[4] << 3) | ((data[5] & 0xE0) >> 5);
}

int AACDemuxerPlugin::AudioDemuxerAACOpen(AudioDemuxerUserArg *userArg)
{
    return 0;
}

int AACDemuxerPlugin::AudioDemuxerAACClose()
{
    return 0;
}

int AACDemuxerPlugin::AudioDemuxerAACPrepare(const uint8_t *buf, uint32_t len, AACDemuxerRst *rst)
{
    if (IsAACPattern(buf)) {
        int mpegVersionIndex  = ((buf[1] & 0x0F) >> 3);
        int mpegVersion = -1;
        if (mpegVersionIndex == 0) {
            mpegVersion = 4;
        } else if (mpegVersionIndex == 1) {
            mpegVersion = 2;
        } else {
            return -1;
        }

        int sampleIndex = ((buf[2] & 0x3C) >> 2);
        int channelCount = ((buf[2] & 0x01) << 2) | ((buf[3] & 0xC0) >> 6);

        int sample = samplingRateMap[sampleIndex];

        rst->frameChannels = channelCount;
        rst->frameSampleRate = sample;
        rst->mpegVersion = mpegVersion;
        MEDIA_LOG_D("channel %d sample %d", rst->frameChannels, rst->frameSampleRate);
        return 0;
    } else {
        MEDIA_LOG_D("Err:IsAACPattern");
        return -1;
    }
}

int AACDemuxerPlugin::AudioDemuxerAACProcess(const uint8_t *buffer, uint32_t bufferLen, AACDemuxerRst *rst)
{
    if (rst == nullptr || buffer == nullptr) {
        return -1;
    }
    rst->frameLength = 0;
    rst->frameBuffer = NULL;
    rst->usedInputLength = 0;

    unsigned int length = 0;
    do {
        if (IsAACPattern(buffer) == 0) {
            MEDIA_LOG_D("Err: IsAACPattern");
            break;
        }

        length = (unsigned int)getFrameLength(buffer);
        if (length + 2 > bufferLen) {
            rst->usedInputLength = bufferLen;
            return 0;
        }

        if (length == 0) {
            MEDIA_LOG_D("length = 0 error");
            return -1;
        }

        if (IsAACPattern(buffer + length)) {
            rst->frameBuffer = (uint8_t *)malloc(length);
            if (rst->frameBuffer) {
                memcpy(rst->frameBuffer, buffer, length);
                rst->frameLength = length;
                rst->usedInputLength = length;
            } else {
                MEDIA_LOG_D("Err:malloc length %d\n", length);
            }
        } else {
            MEDIA_LOG_D("can't find next aac, length %d is error\n", length);
            break;
        }

        return 0;
    } while (0);

    rst->usedInputLength = 1;
    return 0;
}

int AACDemuxerPlugin::AudioDemuxerAACFreeFrame(uint8_t *frame)
{
    if (frame) {
        free(frame);
    }
    return 0;
}


namespace {

    int IsAACPattern(const uint8_t *data)
    {
        return data[0] == 0xff && (data[1] & 0xf0) == 0xf0 && (data[1] & 0x06) == 0x00;
    }

    int Sniff(const std::string& name, std::shared_ptr<DataSource> dataSource)
    {
        Status status = Status::ERROR_UNKNOWN;
        auto buffer = std::make_shared<Buffer>();
        auto bufData = buffer->AllocMemory(nullptr, PROBE_READ_LENGTH);
        int processLoop = 1;
        uint8_t *inputDataPtr = nullptr;
        auto result = dataSource->ReadAt(0, buffer, static_cast<size_t>(PROBE_READ_LENGTH));
        if (result != Status::OK) {
            return 0;
        }
        inputDataPtr = (uint8_t *)bufData->GetReadOnlyData();

        if (IsAACPattern(inputDataPtr) == 0) {
            return 0;
        }

        return 100;
    }

    Status RegisterPlugin(const std::shared_ptr<Register>& reg)
    {
        MEDIA_LOG_I("RegisterPlugin called.");
        if (!reg) {
            MEDIA_LOG_I("RegisterPlugin failed due to nullptr pointer for reg.");
            return Status::ERROR_INVALID_PARAMETER;
        }

        std::string pluginName = "AACDemuxerPlugin";
        DemuxerPluginDef regInfo;
        regInfo.name = pluginName;
        regInfo.description = "adapter for aac demuxer plugin";
        regInfo.rank = 100;
        regInfo.creator = [](const std::string &name) -> std::shared_ptr<DemuxerPlugin> {
            return std::make_shared<AACDemuxerPlugin>(name);
        };
        regInfo.sniffer = Sniff;
        auto rtv = reg->AddPlugin(regInfo);
        if (rtv != Status::OK) {
            MEDIA_LOG_I("RegisterPlugin AddPlugin failed with return %d", static_cast<int>(rtv));
        }
        return Status::OK;
    }
}

PLUGIN_DEFINITION(AACDemuxer, LicenseType::APACHE_V2, RegisterPlugin, [] {});

} // namespace Plugin
} // namespace Media
} // namespace OHOS