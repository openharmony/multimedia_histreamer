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
#include "plugin/plugins/ffmpeg_adapter/utils/aac_audio_config_parser.h"
#include "plugin/plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"

#define private public
#define protected public

using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin;
using namespace Plugin::Ffmpeg;

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

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_1, TestSize.Level1)
{
    const uint8_t config[2] = {0x8A, 0xE6};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_2, TestSize.Level1)
{
    const uint8_t config[2] = {0x8F, 0xE6};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_3, TestSize.Level1)
{
    const uint8_t config[5] = {0x8F, 0xE6, 0x47, 0x6F, 0x5B};
    AACAudioConfigParser aacAudioConfigParser(config, 5);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_4, TestSize.Level1)
{
    const uint8_t config[2] = {0x8E, 0xE6};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_5, TestSize.Level1)
{
    const uint8_t config[2] = {0x2A, 0xE6};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_6, TestSize.Level1)
{
    const uint8_t config[2] = {0x8A, 0xE6};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_7, TestSize.Level1)
{
    const uint8_t config[2] = {0b00000000, 0b00000000};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_8, TestSize.Level1)
{
    const uint8_t config[2] = {0b00010001, 0b10001001};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_9, TestSize.Level1)
{
    const uint8_t config[2] = {0b00011010, 0b10010010};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_10, TestSize.Level1)
{
    const uint8_t config[2] = {0b00100011, 0b00011011};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_11, TestSize.Level1)
{
    const uint8_t config[2] = {0b00001100, 0b10100100};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_12, TestSize.Level1)
{
    const uint8_t config[2] = {0b10000110, 0b10101101};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_13, TestSize.Level1)
{
    const uint8_t config[2] = {0b10000111, 0b10110110};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_14, TestSize.Level1)
{
    const uint8_t config[2] = {0b10000111, 0b01011111};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_15, TestSize.Level1)
{
    const uint8_t config[2] = {0b10000111, 0b01100101};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(true, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_16, TestSize.Level1)
{
    const uint8_t config[1] = {0xFA};
    AACAudioConfigParser aacAudioConfigParser(config, 1);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_17, TestSize.Level1)
{
    const uint8_t config[2] = {0xFA, 0xE6};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

} // namespace Test
} // namespace Media
} // namespace OHOS