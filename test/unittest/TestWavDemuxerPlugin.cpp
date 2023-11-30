/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "plugin/plugins/demuxer/wav_demuxer/wav_demuxer_plugin.h"

using namespace testing::ext;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Plugin::WavPlugin;

namespace OHOS {
namespace Media {
namespace Test {
std::shared_ptr<WavDemuxerPlugin> WavDemuxerPluginCreate(const std::string& name)
{
    return std::make_shared<WavDemuxerPlugin>(name);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_process, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("process");
    ASSERT_TRUE(WavDemuxerPlugin != nullptr);
    auto resetStatus = wavDemuxerPlugin->Reset();
    ASSERT_TRUE(resetStatus == Status::OK);
    auto initStatus = wavDemuxerPlugin->Init();
    ASSERT_TRUE(initStatus == Status::OK);
    auto prepareStatus = wavDemuxerPlugin->Prepare();
    ASSERT_TRUE(prepareStatus == Status::OK);
    auto startStatus = wavDemuxerPlugin->Start();
    ASSERT_TRUE(startStatus == Status::OK);
    auto stopStatus = wavDemuxerPlugin->Stop();
    ASSERT_TRUE(stopStatus == Status::OK);
    auto freeStatus = wavDemuxerPlugin->Deinit();
    ASSERT_TRUE(freeStatus == Status::OK);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_get_parameter, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("get parameter");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    ValueType para;
    auto channelLayoutStatus =  wavDemuxerPlugin->GetParameter(Tag::AUDIO_CHANNEL_LAYOUT, para);
    ASSERT_TRUE(channelLayoutStatus == Status::ERROR_UNIMPLEMENTED);
    auto mediaTypeStatus =  wavDemuxerPlugin->GetParameter(Tag::MEDIA_TYPE, para);
    ASSERT_TRUE(mediaTypeStatus == Status::ERROR_UNIMPLEMENTED);
    auto sampleRateStatus =  wavDemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_RATE, para);
    ASSERT_TRUE(sampleRateStatus == Status::ERROR_UNIMPLEMENTED);
    auto bitrateStatus =  wavDemuxerPlugin->GetParameter(Tag::MEDIA_BITRATE, para);
    ASSERT_TRUE(bitrateStatus == Status::ERROR_UNIMPLEMENTED);
    auto channelsStatus =  wavDemuxerPlugin->GetParameter(Tag::AUDIO_CHANNELS, para);
    ASSERT_TRUE(channelsStatus == Status::ERROR_UNIMPLEMENTED);
    auto trackIdStatus =  wavDemuxerPlugin->GetParameter(Tag::TRACK_ID, para);
    ASSERT_TRUE(trackIdStatus == Status::ERROR_UNIMPLEMENTED);
    auto mimeStatus =  wavDemuxerPlugin->GetParameter(Tag::MIME, para);
    ASSERT_TRUE(mimeStatus == Status::ERROR_UNIMPLEMENTED);
    auto mpegVersionStatus =  wavDemuxerPlugin->GetParameter(Tag::AUDIO_MPEG_VERSION, para);
    ASSERT_TRUE(mpegVersionStatus == Status::ERROR_UNIMPLEMENTED);
    auto sampleFormatStatus =  wavDemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_FORMAT, para);
    ASSERT_TRUE(sampleFormatStatus == Status::ERROR_UNIMPLEMENTED);
    auto samplePerFrameStatus =  wavDemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, para);
    ASSERT_TRUE(samplePerFrameStatus == Status::ERROR_UNIMPLEMENTED);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_set_parameter, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("get parameter");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    ASSERT_TRUE(wavDemuxerPlugin->SetParameter(Tag::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO)
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(wavDemuxerPlugin->SetParameter(Tag::MEDIA_TYPE, MediaType::AUDIO) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(wavDemuxerPlugin->SetParameter(Tag::TRACK_ID, 0) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(wavDemuxerPlugin->SetParameter(Tag::MIME, MEDIA_MIME_AUDIO_RAW) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(wavDemuxerPlugin->SetParameter(Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat::WAVE_FORMAT_PCM)
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(wavDemuxerPlugin->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, 8192) // sample per frame: 8192
        == Status::ERROR_UNIMPLEMENTED);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_get_allocator, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("get allocator");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    auto allocator =  wavDemuxerPlugin->GetAllocator();
    ASSERT_TRUE(allocator == nullptr);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_set_callback, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("set callback");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    Callback* cb = new Callback();
    auto status = wavDemuxerPlugin->SetCallback(cb);
    ASSERT_TRUE(status == Status::OK);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_get_track_count, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("get track count");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    ASSERT_TRUE(wavDemuxerPlugin->GetTrackCount() == 0);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_select_track, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("select track");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    auto selectStatus = wavDemuxerPlugin->SelectTrack(0);
    ASSERT_TRUE(selectStatus == Status::OK);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_unselect_track, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("unselect track");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    auto unselectStatus = wavDemuxerPlugin->UnselectTrack(0);
    ASSERT_TRUE(unselectStatus == Status::OK);
}

HWTEST(TestWavDemuxerPlugin, find_wav_demuxer_plugins_get_select_track, TestSize.Level1)
{
    std::shared_ptr<WavDemuxerPlugin> wavDemuxerPlugin = WavDemuxerPluginCreate("get select track");
    ASSERT_TRUE(wavDemuxerPlugin != nullptr);
    std::vector<int32_t> trackIds = new std::vector<int32_t>[1];
    auto selectStatus = wavDemuxerPlugin->GetSelectedTracks(trackIds);
    ASSERT_TRUE(selectStatus == Status::OK);
}

} // namespace Test
} // namespace Media
} // namespace OHOS