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
#include "plugin/plugins/demuxer/aac_demuxer/aac_demuxer_plugin.h"

using namespace testing::ext;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Plugin::AacDemuxer;

namespace OHOS {
namespace Media {
namespace Test {
std::shared_ptr<AACDemuxerPlugin> AacDemuxerPluginCreate(const std::string& name)
{
    return std::make_shared<AACDemuxerPlugin>(name);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_process, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("process");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    auto resetStatus = aacDemuxerPlugin->Reset();
    ASSERT_TRUE(resetStatus == Status::OK);
    auto initStatus = aacDemuxerPlugin->Init();
    ASSERT_TRUE(initStatus == Status::OK);
    auto prepareStatus = aacDemuxerPlugin->Prepare();
    ASSERT_TRUE(prepareStatus == Status::OK);
    auto startStatus = aacDemuxerPlugin->Start();
    ASSERT_TRUE(startStatus == Status::OK);
    auto stopStatus = aacDemuxerPlugin->Stop();
    ASSERT_TRUE(stopStatus == Status::OK);
    auto freeStatus = aacDemuxerPlugin->Deinit();
    ASSERT_TRUE(freeStatus == Status::OK);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_get_parameter, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("get parameter");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    ValueType para;
    auto channelLayoutStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_CHANNEL_LAYOUT, para);
    ASSERT_TRUE(channelLayoutStatus == Status::ERROR_UNIMPLEMENTED);
    auto mediaTypeStatus =  aacDemuxerPlugin->GetParameter(Tag::MEDIA_TYPE, para);
    ASSERT_TRUE(mediaTypeStatus == Status::ERROR_UNIMPLEMENTED);
    auto sampleRateStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_RATE, para);
    ASSERT_TRUE(sampleRateStatus == Status::ERROR_UNIMPLEMENTED);
    auto bitrateStatus =  aacDemuxerPlugin->GetParameter(Tag::MEDIA_BITRATE, para);
    ASSERT_TRUE(bitrateStatus == Status::ERROR_UNIMPLEMENTED);
    auto channelsStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_CHANNELS, para);
    ASSERT_TRUE(channelsStatus == Status::ERROR_UNIMPLEMENTED);
    auto trackIdStatus =  aacDemuxerPlugin->GetParameter(Tag::TRACK_ID, para);
    ASSERT_TRUE(trackIdStatus == Status::ERROR_UNIMPLEMENTED);
    auto mimeStatus =  aacDemuxerPlugin->GetParameter(Tag::MIME, para);
    ASSERT_TRUE(mimeStatus == Status::ERROR_UNIMPLEMENTED);
    auto streamFormatStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_AAC_STREAM_FORMAT, para);
    ASSERT_TRUE(streamFormatStatus == Status::ERROR_UNIMPLEMENTED);
    auto mpegVersionStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_MPEG_VERSION, para);
    ASSERT_TRUE(mpegVersionStatus == Status::ERROR_UNIMPLEMENTED);
    auto sampleFormatStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_FORMAT, para);
    ASSERT_TRUE(sampleFormatStatus == Status::ERROR_UNIMPLEMENTED);
    auto samplePerFrameStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, para);
    ASSERT_TRUE(samplePerFrameStatus == Status::ERROR_UNIMPLEMENTED);
    auto profileStatus =  aacDemuxerPlugin->GetParameter(Tag::AUDIO_AAC_PROFILE, para);
    ASSERT_TRUE(profileStatus == Status::ERROR_UNIMPLEMENTED);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_set_parameter, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("get parameter");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO)
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::MEDIA_TYPE, MediaType::AUDIO) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::TRACK_ID, 0) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::MIME, MEDIA_MIME_AUDIO_AAC) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat::S16)
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, 1024) // sample per frame: 1024
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::AUDIO_AAC_PROFILE, AudioAacProfile::LC)
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(aacDemuxerPlugin->SetParameter(Tag::AUDIO_AAC_STREAM_FORMAT, AudioAacStreamFormat::MP4ADTS)
        == Status::ERROR_UNIMPLEMENTED);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_get_allocator, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("get allocator");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    auto allocator =  aacDemuxerPlugin->GetAllocator();
    ASSERT_TRUE(allocator == nullptr);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_set_callback, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("set callback");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    Callback* cb = new Callback();
    auto status = aacDemuxerPlugin->SetCallback(cb);
    ASSERT_TRUE(status == Status::OK);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_get_track_count, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("get track count");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    ASSERT_TRUE(aacDemuxerPlugin->GetTrackCount() == 0);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_select_track, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("select track");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    auto selectStatus = aacDemuxerPlugin->SelectTrack(0);
    ASSERT_TRUE(selectStatus == Status::OK);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_unselect_track, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("unselect track");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    auto unselectStatus = aacDemuxerPlugin->UnselectTrack(0);
    ASSERT_TRUE(unselectStatus == Status::OK);
}

HWTEST(TestAacDemuxerPlugin, find_aac_demuxer_plugins_get_select_track, TestSize.Level1)
{
    std::shared_ptr<AACDemuxerPlugin> aacDemuxerPlugin = AacDemuxerPluginCreate("get select track");
    ASSERT_TRUE(aacDemuxerPlugin != nullptr);
    std::vector<int32_t> trackIds = new std::vector<int32_t>[1];
    auto selectStatus = aacDemuxerPlugin->GetSelectedTracks(trackIds);
    ASSERT_TRUE(selectStatus == Status::OK);
}

} // namespace Test
} // namespace Media
} // namespace OHOS