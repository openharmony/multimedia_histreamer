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

// @fixture(tags=3daAudio_player_fast)
FIXTURE(DataDriven3DaSinglePlayerTestFast)
{
    bool CheckTimeEquality(int32_t expectValue, int32_t currentValue)
    {
        MEDIA_LOG_I("expectValue : %d, currentValue : %d", expectValue, currentValue);
        return fabs(expectValue - currentValue) < 1000; // if open debug log, should use value >= 1000
    }
    DATA_PROVIDER(myTsUrls, 1,
    DATA_GROUP(std::string(RESOURCE_DIR "/TS/h264_aac_270p_10r_voiced.ts")));

    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_SRC_API_0200)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Play_API_0200)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Play_API_0300)
    {
        int64_t timeMs = 5000;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Seek(timeMs));
        ASSERT_EQ(0, player->Release());
    }

    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Pause_API_0100)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Pause_API_0200)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Pause_API_0300)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Pause_API_0400)
    {
        int64_t timeMs = 5000;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Seek(timeMs));
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Stop_API_0100)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Stop_API_0200)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Stop_API_0300)
    {
        int64_t timeMs = 5000;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Seek(timeMs));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Seek_API_0100)
    {
        int64_t timeMs = 5000;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Seek(timeMs));
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Seek_API_0200)
    {
        int64_t timeMs = 5000;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Seek(timeMs));
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Seek_API_0300)
    {
        int64_t timeMs = 0;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Seek(timeMs));
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Reset_API_0100)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Reset_API_0200)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_SetVolume_API_0100/0200)
    {
        float lv = 0.5;
        float rv = 0.5;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->SetVolume(lv, rv));
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_SetVolume_API_0300)
    {
        float lv = 0.5;
        float rv = 0.5;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->SetVolume(lv, rv));
        ASSERT_EQ(0, player->Release());
    }

    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Release_API_0100/0200)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Release_API_0300)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Release_API_0400)
    {
        int64_t timeMs = 5000;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Seek(timeMs));
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Release_API_0500)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Time_API_0100)
    {
        int64_t durationMs;
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_TRUE(CheckTimeEquality(3010, durationMs)); // 3010---duration of audio
        ASSERT_EQ(0, player->Release());
    }
    // @test(data="myTsUrls", tags=3da_audio_play_fast)
    PTEST((std::string url), SUB_MEDIA_PLAYER_AudioPlayer_Play_API_0400)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }
};

