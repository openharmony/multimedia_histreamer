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
    uint32_t level = aacAudioConfigParser.GetLevel();
    EXPECT_EQ(2, level);
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
    EXPECT_EQ(false, status);
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
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_13, TestSize.Level1)
{
    const uint8_t config[2] = {0b10000111, 0b10110110};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_14, TestSize.Level1)
{
    const uint8_t config[2] = {0b10000111, 0b01011111};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
}

HWTEST(AACAudioConfigParserTest, test_aac_audio_config_parser_15, TestSize.Level1)
{
    const uint8_t config[2] = {0b10000111, 0b01100101};
    AACAudioConfigParser aacAudioConfigParser(config, 2);
    bool status = aacAudioConfigParser.ParseConfigs();
    EXPECT_EQ(false, status);
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

HWTEST(UtilsTest, testAVStrError, TestSize.Level1)
{
    const int number = 1;
    auto res = AVStrError(number);
    ASSERT_STREQ(res.c_str(), "No error information");
}

HWTEST(UtilsTest, testConvertTimeFromFFmpeg, TestSize.Level1)
{
    int64_t number = 1000;
    AVRational rational = av_make_q(1000, 500);
    auto res = ConvertTimeFromFFmpeg(number, rational);
    ASSERT_EQ(res, 2000000000000);
    number = ((int64_t)UINT64_C(0x8000000000000000));
    auto res2 = ConvertTimeFromFFmpeg(number, rational);
    ASSERT_EQ(res2, -1);
}

HWTEST(UtilsTest, testConvertTimeToFFmpeg, TestSize.Level1)
{
    int64_t number = 1045566545;
    AVRational rational = av_make_q(0, 50);
    auto res = ConvertTimeToFFmpeg(number, rational);
    ASSERT_EQ(res, ((int64_t)UINT64_C(0x8000000000000000)));
    rational = av_make_q(10, 500);
    auto res2 = ConvertTimeToFFmpeg(number, rational);
    ASSERT_EQ(res2, 52);
}

HWTEST(UtilsTest, testFillAVPicture, TestSize.Level1)
{
    AVFrame* frame = av_frame_alloc();
    uint8_t ptr = 10;
    auto res = FillAVPicture(frame, &ptr, AVPixelFormat::AV_PIX_FMT_ABGR, 1920, 1080);
    ASSERT_EQ(res, 0);
}

HWTEST(UtilsTest, testGetAVPictureSize, TestSize.Level1)
{
    AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_ABGR;
    int height = 1920;
    int width = 1080;
    auto res = GetAVPictureSize(format, height, width);
    ASSERT_EQ(res, 0);
}

HWTEST(UtilsTest, testRemoveDelimiter, TestSize.Level1)
{
    const char* str = "hello";
    char ch = 'o';
    auto res = RemoveDelimiter(str, ch);
    ASSERT_STREQ(res.c_str(), "hell");
}

HWTEST(UtilsTest, testRemoveDelimiter2, TestSize.Level1)
{
    std::string str = "hello";
    char ch = 'l';
    RemoveDelimiter(ch, str);
    ASSERT_STREQ(str.c_str(), "heo");
}

HWTEST(UtilsTest, testReplaceDelimiter, TestSize.Level1)
{
    std::string limit = "he";
    char ch = 'e';
    std::string str = "hello";
    ReplaceDelimiter(limit, ch, str);
    ASSERT_STREQ(str.c_str(), "eeeee");
    char ch2 = 'o';
    std::string str2 = "hello";
    ReplaceDelimiter(limit, ch2, str2);
    ASSERT_STREQ(str2.c_str(), "hello");
}

HWTEST(UtilsTest, testSplitString, TestSize.Level1)
{
    const char* limit = "hello";
    char ch = 'l';
    std::vector<std::string> res = SplitString(limit, ch);
    ASSERT_EQ(res.size(), 1);
    ASSERT_STREQ(res[0].c_str(), "he");
}

HWTEST(UtilsTest, testConvertChannelLayoutFromFFmpeg, TestSize.Level1)
{
    for (int index = 0; index <= 24; index++) {
        auto res = ConvertChannelLayoutFromFFmpeg(index, 0);
        switch (index) {
            case 1:
                ASSERT_EQ(AudioChannelLayout::MONO, res);
                break;
            case 2:
                ASSERT_EQ(AudioChannelLayout::STEREO, res);
                break;
            case 4:
                ASSERT_EQ(AudioChannelLayout::CH_4POINT0, res);
                break;
            case 6:
                ASSERT_EQ(AudioChannelLayout::CH_5POINT1, res);
                break;
            case 8:
                ASSERT_EQ(AudioChannelLayout::CH_5POINT1POINT2, res);
                break;
            case 10:
                ASSERT_EQ(AudioChannelLayout::CH_7POINT1POINT2, res);
                break;
            case 12:
                ASSERT_EQ(AudioChannelLayout::CH_7POINT1POINT4, res);
                break;
            case 14:
                ASSERT_EQ(AudioChannelLayout::CH_9POINT1POINT4, res);
                break;
            case 16:
                ASSERT_EQ(AudioChannelLayout::CH_9POINT1POINT6, res);
                break;
            case 24:
                ASSERT_EQ(AudioChannelLayout::CH_22POINT2, res);
                break;
            default:
                ASSERT_EQ(AudioChannelLayout::UNKNOWN, res);
                break;
        }
    }
}

} // namespace Test
} // namespace Media
} // namespace OHOS