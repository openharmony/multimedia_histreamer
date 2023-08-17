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

#include <string>
#include "gtest/gtest.h"
#include "plugin/plugins/ffmpeg_adapter/video_encoder/ffmpeg_vid_enc_config.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;
using namespace testing::ext;

HWTEST(TestFFmpegVidEncConfig, test_video_encoder_parameter, TestSize.Level1)
{
    std::map<Tag, ValueType> meta {};
    AVCodecContext avCodecContext {};
    ValueType  value;
    meta.insert(std::make_pair(Tag::SECTION_VIDEO_UNIVERSAL_START, value));
    avCodecContext.codec_type = AVMEDIA_TYPE_VIDEO;
    Ffmpeg::ConfigVideoEncoder((AVCodecContext &) avCodecContext, meta);
    ValueType val = 3;
    auto status = Ffmpeg::GetVideoEncoderParameters(avCodecContext, Tag::SECTION_VIDEO_UNIVERSAL_START, val);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, status);
}
} // namespace Test
} // namespace Media
} // namespace OHOS
