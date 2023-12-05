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
#include "plugin/plugins/demuxer/minimp4_demuxer/minimp4_demuxer_plugin.h"

using namespace testing::ext;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Plugin::Minimp4;

namespace OHOS {
namespace Media {
namespace Test {
std::shared_ptr<MiniMP4DemuxerPlugin> MiniMP4DemuxerPluginCreate(const std::string& name)
{
    return std::make_shared<MiniMP4DemuxerPlugin>(name);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_process, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("process");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    auto resetStatus = minimp4DemuxerPlugin->Reset();
    ASSERT_TRUE(resetStatus == Status::OK);
    auto initStatus = minimp4DemuxerPlugin->Init();
    ASSERT_TRUE(initStatus == Status::OK);
    auto prepareStatus = minimp4DemuxerPlugin->Prepare();
    ASSERT_TRUE(prepareStatus == Status::OK);
    auto startStatus = minimp4DemuxerPlugin->Start();
    ASSERT_TRUE(startStatus == Status::OK);
    auto stopStatus = minimp4DemuxerPlugin->Stop();
    ASSERT_TRUE(stopStatus == Status::OK);
    auto freeStatus = minimp4DemuxerPlugin->Deinit();
    ASSERT_TRUE(freeStatus == Status::OK);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_get_parameter, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("get parameter");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    ValueType para;
    auto channelLayoutStatus =  minimp4DemuxerPlugin->GetParameter(Tag::AUDIO_CHANNEL_LAYOUT, para);
    ASSERT_TRUE(channelLayoutStatus == Status::ERROR_UNIMPLEMENTED);
    auto mediaTypeStatus =  minimp4DemuxerPlugin->GetParameter(Tag::MEDIA_TYPE, para);
    ASSERT_TRUE(mediaTypeStatus == Status::ERROR_UNIMPLEMENTED);
    auto sampleRateStatus =  minimp4DemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_RATE, para);
    ASSERT_TRUE(sampleRateStatus == Status::ERROR_UNIMPLEMENTED);
    auto bitrateStatus =  minimp4DemuxerPlugin->GetParameter(Tag::MEDIA_BITRATE, para);
    ASSERT_TRUE(bitrateStatus == Status::ERROR_UNIMPLEMENTED);
    auto channelsStatus =  minimp4DemuxerPlugin->GetParameter(Tag::AUDIO_CHANNELS, para);
    ASSERT_TRUE(channelsStatus == Status::ERROR_UNIMPLEMENTED);
    auto trackIdStatus =  minimp4DemuxerPlugin->GetParameter(Tag::TRACK_ID, para);
    ASSERT_TRUE(trackIdStatus == Status::ERROR_UNIMPLEMENTED);
    auto mimeStatus =  minimp4DemuxerPlugin->GetParameter(Tag::MIME, para);
    ASSERT_TRUE(mimeStatus == Status::ERROR_UNIMPLEMENTED);
    auto mpegVersionStatus =  minimp4DemuxerPlugin->GetParameter(Tag::AUDIO_MPEG_VERSION, para);
    ASSERT_TRUE(mpegVersionStatus == Status::ERROR_UNIMPLEMENTED);
    auto sampleFormatStatus =  minimp4DemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_FORMAT, para);
    ASSERT_TRUE(sampleFormatStatus == Status::ERROR_UNIMPLEMENTED);
    auto samplePerFrameStatus =  minimp4DemuxerPlugin->GetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, para);
    ASSERT_TRUE(samplePerFrameStatus == Status::ERROR_UNIMPLEMENTED);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_set_parameter, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("get parameter");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    ASSERT_TRUE(minimp4DemuxerPlugin->SetParameter(Tag::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO)
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(minimp4DemuxerPlugin->SetParameter(Tag::MEDIA_TYPE, MediaType::AUDIO) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(minimp4DemuxerPlugin->SetParameter(Tag::TRACK_ID, 0) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(minimp4DemuxerPlugin->SetParameter(Tag::MIME, MEDIA_MIME_AUDIO_RAW) == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(minimp4DemuxerPlugin->SetParameter(Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat::WAVE_FORMAT_PCM)
        == Status::ERROR_UNIMPLEMENTED);
    ASSERT_TRUE(minimp4DemuxerPlugin->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, 8192) // sample per frame: 8192
        == Status::ERROR_UNIMPLEMENTED);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_get_allocator, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("get allocator");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    auto allocator =  minimp4DemuxerPlugin->GetAllocator();
    ASSERT_TRUE(allocator == nullptr);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_set_callback, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("set callback");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    Callback* cb = new Callback();
    auto status = minimp4DemuxerPlugin->SetCallback(cb);
    ASSERT_TRUE(status == Status::OK);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_get_track_count, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("get track count");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    ASSERT_TRUE(minimp4DemuxerPlugin->GetTrackCount() == 0);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_select_track, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("select track");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    auto selectStatus = minimp4DemuxerPlugin->SelectTrack(0);
    ASSERT_TRUE(selectStatus == Status::OK);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_unselect_track, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("unselect track");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    auto unselectStatus = minimp4DemuxerPlugin->UnselectTrack(0);
    ASSERT_TRUE(unselectStatus == Status::OK);
}

HWTEST(TestMiniMp4DemuxerPlugin, find_minimp4_demuxer_plugins_get_select_track, TestSize.Level1)
{
    std::shared_ptr<MiniMP4DemuxerPlugin> minimp4DemuxerPlugin = MiniMP4DemuxerPluginCreate("get select track");
    ASSERT_TRUE(minimp4DemuxerPlugin != nullptr);
    std::vector<int32_t> trackIds = new std::vector<int32_t>[1];
    auto selectStatus = minimp4DemuxerPlugin->GetSelectedTracks(trackIds);
    ASSERT_TRUE(selectStatus == Status::OK);
}

} // namespace Test
} // namespace Media
} // namespace OHOS