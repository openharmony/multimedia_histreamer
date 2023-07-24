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

#include "gtest/gtest.h"
#include "plugin/common/any.h"
#include "plugin/plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"
#define private public
#define protected public

using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin;

HWTEST(ChannelLayoutTest, test_convert_from_ffmpeg_mono, TestSize.Level1)
{
    int channels = 1;
    uint64_t ffChannelLayout = 0x4;

    AudioChannelLayout channelLayout = Ffmpeg::ConvertChannelLayoutFromFFmpeg(channels, ffChannelLayout);
    EXPECT_EQ(AudioChannelLayout::MONO, channelLayout);
}

HWTEST(ChannelLayoutTest, test_convert_from_ffmpeg_stereo, TestSize.Level1)
{
    int channels = 2;
    uint64_t ffChannelLayout = 0x3;

    AudioChannelLayout channelLayout = Ffmpeg::ConvertChannelLayoutFromFFmpeg(channels, ffChannelLayout);
    EXPECT_EQ(AudioChannelLayout::STEREO, channelLayout);
}
} // namespace Test
} // namespace Media
} // namespace OHOS