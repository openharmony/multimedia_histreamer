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
#include "plugin/plugins/sink/audio_server_sink/audio_server_sink_plugin.h"

using namespace testing::ext;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Plugin::AuSrSinkPlugin;

namespace OHOS {
namespace Media {
namespace Test {
std::shared_ptr<AudioSinkPlugin> AudioServerSinkPluginCreate(const std::string& name)
{
    return std::make_shared<AudioServerSinkPlugin>(name);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_process, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("process");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    auto initStatus = audioSinkPlugin->Init();
    ASSERT_TRUE(initStatus == Status::OK);
    auto freeStatus = audioSinkPlugin->Deinit();
    ASSERT_TRUE(freeStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_get_parameter, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("get parameter");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    ValueType para;
    auto sampleRateStatus =  audioSinkPlugin->GetParameter(Tag::AUDIO_SAMPLE_RATE, para);
    ASSERT_TRUE(sampleRateStatus == Status::OK);
    auto outputChannelsStatus =  audioSinkPlugin->GetParameter(Tag::AUDIO_OUTPUT_CHANNELS, para);
    ASSERT_TRUE(outputChannelsStatus == Status::OK);
    auto outputLayoutStatus =  audioSinkPlugin->GetParameter(Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT, para);
    ASSERT_TRUE(outputLayoutStatus == Status::OK);
    auto bitrateStatus =  audioSinkPlugin->GetParameter(Tag::MEDIA_BITRATE, para);
    ASSERT_TRUE(bitrateStatus == Status::OK);
    auto sampleFormatStatus =  audioSinkPlugin->GetParameter(Tag::AUDIO_SAMPLE_FORMAT, para);
    ASSERT_TRUE(sampleFormatStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_get_allocator, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("get allocator");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    auto allocator =  audioSinkPlugin->GetAllocator();
    ASSERT_TRUE(allocator == nullptr);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_get_mute, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("get mute");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    bool mute = false;
    auto muteStatus = audioSinkPlugin->GetMute(mute);
    ASSERT_TRUE(muteStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_set_mute, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("set mute");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    bool mute = false;
    auto muteStatus = audioSinkPlugin->SetMute(mute);
    ASSERT_TRUE(muteStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_get_speed, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("get speed");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    float speed;
    auto speedStatus = audioSinkPlugin->GetSpeed(speed);
    ASSERT_TRUE(speedStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_set_speed, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("set speed");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    float speed = 0.0f;
    auto speedStatus = audioSinkPlugin->SetSpeed(speed);
    ASSERT_TRUE(speedStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_get_frame_size, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("get frame size");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    size_t frameSize;
    auto frameSizeStatus =  audioSinkPlugin->GetFrameSize(frameSize);
    ASSERT_TRUE(frameSizeStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_set_frame_count, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("set frame count");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    uint32_t frameCount;
    auto frameCountStatus =  audioSinkPlugin->GetFrameCount(frameCount);
    ASSERT_TRUE(frameCountStatus == Status::OK);
}

HWTEST(TestAudioSinkPlugin, find_audio_sink_plugins_get_latency, TestSize.Level1)
{
    std::shared_ptr<AudioSinkPlugin> audioSinkPlugin = AudioServerSinkPluginCreate("get latency");
    ASSERT_TRUE(audioSinkPlugin != nullptr);
    uint64_t hstTime;
    auto latencyStatus =  audioSinkPlugin->GetLatency(hstTime);
    ASSERT_TRUE(latencyStatus == Status::OK);
}

} // namespace Test
} // namespace Media
} // namespace OHOS