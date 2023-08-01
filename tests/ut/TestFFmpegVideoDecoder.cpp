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
#include "plugin/core/plugin_register.h"
#include "plugin/plugins/ffmpeg_adapter/video_decoder/video_ffmpeg_decoder_plugin.h"
#include "plugin/common/plugin_caps_builder.h"
#include "plugin/core/plugin_manager.h"

namespace OHOS {
namespace Media {
namespace Test {
    using namespace Plugin;
    using namespace Ffmpeg;
    using namespace testing::ext;
    std::shared_ptr<CodecPlugin> VideoFfmpegDecoderCreator(const std::string& name)
    {
        return std::make_shared<VideoFfmpegDecoderPlugin>(name);
    }

HWTEST(VideoFfmpegDecoderPluginTest, test_State, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> videoDecoderPlugin = VideoFfmpegDecoderCreator("VideoFfmpegDecoderPluginTest");
    ASSERT_TRUE(videoDecoderPlugin != nullptr);

    auto status = videoDecoderPlugin->Init();
    ASSERT_TRUE(status == Status::ERROR_UNSUPPORTED_FORMAT);

    auto prepareStatus = videoDecoderPlugin->Prepare();
    ASSERT_TRUE(prepareStatus == Status::ERROR_WRONG_STATE);

    auto deInitStatus = videoDecoderPlugin->Deinit();
    ASSERT_TRUE(deInitStatus == Status::OK);

    auto resetStatus = videoDecoderPlugin->Reset();
    ASSERT_TRUE(resetStatus == Status::OK);

    auto startStatus = videoDecoderPlugin->Start();
    ASSERT_FALSE(startStatus == Status::OK);

    auto flushStatus = videoDecoderPlugin->Flush();
    ASSERT_TRUE(flushStatus == Status::OK);
}

HWTEST(VideoFfmpegDecoderPluginTest, test_Parameter, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> videoDecoderPlugin = VideoFfmpegDecoderCreator("VideoFfmpegDecoderPluginTest");
    ASSERT_TRUE(videoDecoderPlugin != nullptr);

    auto status = videoDecoderPlugin->SetParameter(Tag::VIDEO_FRAME_RATE, 30);
    ASSERT_TRUE(status == Status::OK);

    ValueType parameterValue;
    auto prepareStatus = videoDecoderPlugin->GetParameter(Tag::VIDEO_FRAME_RATE, parameterValue);
    ASSERT_TRUE(prepareStatus == Status::OK);
}

HWTEST(VideoFfmpegDecoderPluginTest, test_QueueInputBuffer, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> videoDecoderPlugin = VideoFfmpegDecoderCreator("VideoFfmpegDecoderPluginTest");
    std::shared_ptr<Buffer> inputBuffer = std::make_shared<Buffer>(BufferMetaType::VIDEO);
    int32_t timeoutMs = 100;
    ASSERT_EQ(Status::ERROR_INVALID_DATA, videoDecoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
    uint32_t size = 16;
    inputBuffer->AllocMemory(nullptr, size);
    ASSERT_EQ(Status::ERROR_WRONG_STATE, videoDecoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
    inputBuffer->flag = 1;
    ASSERT_EQ(Status::ERROR_WRONG_STATE, videoDecoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
}

} //namespace Test
} //namespace Media
} //namespace OHOS

