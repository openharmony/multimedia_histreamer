/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#include <chrono>
#include <fcntl.h>
#include <math.h>
#include <thread>
#include "foundation/log.h"
#include "../st/helper/test_player.hpp"

#ifndef WIN32
#define RESOURCE_DIR "/data/test/media/"
using namespace testing::ext;
#endif

using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Test;

namespace OHOS {
namespace Media {
namespace Test {
    class UtTestFastPlayer : public ::testing::Test {
    public:
        virtual void SetUp() override
        {
        }
        virtual void TearDown() override
        {
        }
    };
    void pushShortMusicUrlsSource() {
        std::vector<std::string> vecSource;
        vecSource.push_back(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_SHORT.m4a"));
        vecSource.push_back(std::string(RESOURCE_DIR "/WAV/vorbis_48000_32_SHORT.wav"));
        vecSource.push_back(std::string(RESOURCE_DIR "/AAC/AAC_48000_32_SHORT.aac"));
        vecSource.push_back(std::string(RESOURCE_DIR "/FLAC/vorbis_48000_32_SHORT.flac"));
        vecSource.push_back(std::string(RESOURCE_DIR "/MP3/MP3_LONG_48000_32.mp3"));
        vecSource.push_back(std::string(RESOURCE_DIR "/WAV/vorbis_48000_32_SHORT.wav"));
        vecSource.push_back(std::string("http://localhost/resource-src/media/MP3/MP3_48000_32_SHORT.mp3"));
    }
    void pushLongMusicUrlsSource() {
        std::vector<std::string> vecSource;
        vecSource.push_back(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_SHORT.m4a"));
        vecSource.push_back(std::string(RESOURCE_DIR "/MP3/MP3_LONG_48000_32.mp3"));
        vecSource.push_back(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_LONG.m4a"));
        vecSource.push_back(std::string("http://img.51miz.com/preview/sound/00/26/73/51miz-S267356-423D33372.mp3"));
        vecSource.push_back(std::string("http://localhost/resource-src/media/MP3/MP3_LONG_48000_32.mp3"));
    }
    bool CheckTimeEquality(int32_t expectValue, int32_t currentValue)
    {
        MEDIA_LOG_I("expectValue : %d, currentValue : %d", expectValue, currentValue);
        return fabs(expectValue - currentValue) < 1000; // if open debug log, should use value >= 1000
    }

    void Test_single_player_automatically(std::string url)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

#ifndef WIN32
    HWTEST_F(UtTestFastPlayer, Test_single_player_automatical, TestSize.Level1)
#else
    TEST_F(UtTestFastPlayer, Test_single_player_automatical)
#endif
    {
        std::vector<std::string> vecSource;
        vecSource.push_back(std::string(RESOURCE_DIR "/resources/MP3_LONG_48000_32.mp3"));
       /* vecSource.push_back(std::string(RESOURCE_DIR "/MP3/MP3_48000_32_SHORT.mp3"));
        vecSource.push_back(std::string(RESOURCE_DIR "/AAC/AAC_48000_32_SHORT.aac"));
        vecSource.push_back(std::string(RESOURCE_DIR "/FLAC/vorbis_48000_32_SHORT.flac"));
        vecSource.push_back(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_SHORT.m4a"));
        vecSource.push_back(std::string(RESOURCE_DIR "/WAV/vorbis_48000_32_SHORT.wav"));
        vecSource.push_back(std::string("http://localhost/resource-src/media/MP3/MP3_48000_32_SHORT.mp3"));*/
        for(auto url:vecSource)
        {
            Test_single_player_automatically(url);
        }
    }

} // namespace Test
} // namespace Media
} // namespace OHOS
