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
#include <chrono>
#include <math.h>
#include <thread>
#include "helper/test_player.hpp"
#include "testngpp/testngpp.hpp"
#include "foundation/log.h"

using namespace OHOS::Media::Test;
FIXTURE(DataDrivenSinglePlayerTestFast)
{
    bool CheckTimeEquality(int32_t expectValue, int32_t currentValue)
    {
        MEDIA_LOG_I("expectValue : %d, currentValue : %d", expectValue, currentValue);
        return fabs(expectValue - currentValue) < 1000; // if open debug log, should use value >= 1000
    }

    DATA_PROVIDER(myurls, 100,
    DATA_GROUP(std::string(RESOURCE_DIR "/MP3/MP3_LONG_48000_32.mp3")),
    DATA_GROUP(std::string(RESOURCE_DIR "/MP3/MP3_48000_32_SHORT.mp3")),
    DATA_GROUP(std::string(RESOURCE_DIR "/AAC/AAC_48000_32_SHORT.aac")),
    DATA_GROUP(std::string(RESOURCE_DIR "/FLAC/vorbis_48000_32_SHORT.flac")),
    DATA_GROUP(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_SHORT.m4a")),
    DATA_GROUP(std::string(RESOURCE_DIR "/WAV/vorbis_48000_32_SHORT.wav")),
    DATA_GROUP(std::string("http://localhost/resource-src/media/MP3/MP3_48000_32_SHORT.mp3")));

    DATA_PROVIDER(shortMusicUrls, 100,
    DATA_GROUP(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_SHORT.m4a")),
    DATA_GROUP(std::string(RESOURCE_DIR "/WAV/vorbis_48000_32_SHORT.wav")),
    DATA_GROUP(std::string(RESOURCE_DIR "/AAC/AAC_48000_32_SHORT.aac")),
    DATA_GROUP(std::string(RESOURCE_DIR "/FLAC/vorbis_48000_32_SHORT.flac")),
    DATA_GROUP(std::string(RESOURCE_DIR "/MP3/MP3_LONG_48000_32.mp3")),
    DATA_GROUP(std::string(RESOURCE_DIR "/WAV/vorbis_48000_32_SHORT.wav")),
    DATA_GROUP(std::string("http://localhost/resource-src/media/MP3/MP3_48000_32_SHORT.mp3")));

    DATA_PROVIDER(longMusicUrls, 100,
    DATA_GROUP(std::string(RESOURCE_DIR "/MP3/MP3_LONG_48000_32.mp3")),
    DATA_GROUP(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_LONG.m4a")),
    DATA_GROUP(std::string("http://img.51miz.com/preview/sound/00/26/73/51miz-S267356-423D33372.mp3")),
    DATA_GROUP(std::string("http://localhost/resource-src/media/MP3/MP3_LONG_48000_32.mp3")));

    DATA_PROVIDER(durationTestMusicUrls, 100,
    DATA_GROUP(std::string(RESOURCE_DIR "/MP3/MP3_LONG_48000_32.mp3"), 30000),
    DATA_GROUP(std::string(RESOURCE_DIR "/AAC/AAC_LONG_48000_32.aac"), 30000),
    DATA_GROUP(std::string(RESOURCE_DIR "/FLAC/FLAC_48000_32_LONG.flac"), 30000),
    DATA_GROUP(std::string(RESOURCE_DIR "/WAV/vorbis_48000_32_SHORT.wav"), 5000),
    DATA_GROUP(std::string(RESOURCE_DIR "/M4A/MPEG-4_48000_32_LONG.m4a"), 30000));

    // @test(data="shortMusicUrls", tags=fast)
    PTEST((std::string url), Test single player play url music, and finished automatically)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    // @test(data="longMusicUrls", tags=fast)
    PTEST((std::string url), Test single player play url music, and finished manually)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="durationTestMusicUrls", tags=fast)
    PTEST((std::string url, int32_t expectDuration), Can get music duration)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        int64_t duration;
        ASSERT_EQ(0, player->GetDuration(duration));
        ASSERT_TRUE(CheckTimeEquality(expectDuration, duration));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="longMusicUrls", tags=fast)
    PTEST((std::string url), Can get current time)
    {
        int64_t currentMS {0};
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_TRUE(CheckTimeEquality(2000, currentMS));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="longMusicUrls", tags=fast)
    PTEST((std::string url), Can seek to the position specified)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(seekPos, currentMS));
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="longMusicUrls", tags=fast)
    PTEST((std::string url), Can seek after pause)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Seek(seekPos));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_TRUE(CheckTimeEquality(seekPos, currentMS)); // pause + seek still pause
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="longMusicUrls", tags=fast)
    PTEST((std::string url), Can pause,then seek,then play,final stop)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Seek(seekPos));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_TRUE(CheckTimeEquality(seekPos, currentMS)); // pause + seek still pause
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // pause + seek + play still play
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="longMusicUrls", tags=fast)
    PTEST((std::string url), Can play after pause)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Pause());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="shortMusicUrls", tags=fast)
    PTEST((std::string url), Can single Loop)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->SetSingleLoop(true));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::seconds(20));
        ASSERT_EQ(0, player->Stop());
    }
};
