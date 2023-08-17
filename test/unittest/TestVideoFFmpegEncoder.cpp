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

#include <memory>
#include <string>
#include "gtest/gtest.h"
#include "plugin/plugins/ffmpeg_adapter/video_encoder/video_ffmpeg_encoder_plugin.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;
using namespace testing::ext;

class TestFFmpegVideoEncoder : public ::testing::Test {
public:
    std::shared_ptr<Ffmpeg::VideoFfmpegEncoderPlugin> videoFfmpegEncoderPlugin;
    void SetUp() override
    {
        videoFfmpegEncoderPlugin = std::make_shared<Ffmpeg::VideoFfmpegEncoderPlugin>("test");
    }

    void TearDown() override
    {
    }
};

HWTEST_F(TestFFmpegVideoEncoder, test_init, TestSize.Level1)
{
    auto status = videoFfmpegEncoderPlugin->Init();
    EXPECT_EQ(Status::ERROR_UNSUPPORTED_FORMAT, status);
}

HWTEST_F(TestFFmpegVideoEncoder, test_prepare, TestSize.Level1)
{
    auto status = videoFfmpegEncoderPlugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, status);
}

HWTEST_F(TestFFmpegVideoEncoder, test_reset, TestSize.Level1)
{
    auto status = videoFfmpegEncoderPlugin->Reset();
    EXPECT_EQ(Status::OK, status);
}

HWTEST_F(TestFFmpegVideoEncoder, test_start, TestSize.Level1)
{
    auto status = videoFfmpegEncoderPlugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, status);
}

HWTEST_F(TestFFmpegVideoEncoder, test_set_get_parameter, TestSize.Level1)
{
    Tag tag = Tag::SECTION_VIDEO_UNIVERSAL_START;
    ValueType value = 2;
    auto status = videoFfmpegEncoderPlugin->GetParameter(tag, value);
    EXPECT_EQ(Status::ERROR_WRONG_STATE, status);
    status = videoFfmpegEncoderPlugin->SetParameter(tag, value);
    EXPECT_EQ(Status::OK, status);
    status = videoFfmpegEncoderPlugin->GetParameter(tag, value);
    EXPECT_EQ(Status::OK, status);
}

HWTEST_F(TestFFmpegVideoEncoder, test_deinit_status, TestSize.Level1)
{
    auto status = videoFfmpegEncoderPlugin->Deinit();
    EXPECT_EQ(Status::OK, status);
}
} // namespace Test
} // namespace Media
} // namespace OHOS
