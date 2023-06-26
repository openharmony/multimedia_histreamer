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
#include <chrono>
#include <fcntl.h>
#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#define O_BINARY 0 // which is not defined for Linux
#endif
#include <math.h>
#include <thread>
#include "foundation/log.h"
#include "helper/test_player.hpp"
#include "testngpp/testngpp.hpp"
#include "test_single_video_player_fast_1.h"

using namespace OHOS::Media::Test;

// @fixture(tags=video_play_fast)
FIXTURE(dataDrivenSingleVideoPlayerTestFast3)
{
    DATA_PROVIDER(myfdurl, 2,
                  DATA_GROUP(std::string(RESOURCE_DIR "/MP4/H264_AAC.mp4"), 1894335),
                  DATA_GROUP(std::string(RESOURCE_DIR "/../demo_resource/video/1h264_320x240_60.3gp"), 494522));
    static const int64_t NEXT_FRAME_TIME {8300};
    static const int64_t PREV_FRAME_TIME {4166};
    std::string FilePathToFd(std::string url, int32_t fileSize)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        int32_t fd = open(url.c_str(), O_RDONLY|O_BINARY);
        std::string fdStr = std::to_string(fd);
        uri.insert(5, fdStr); // 5 ---fd:://
        return uri;
    }
    bool CheckTimeEquality(int32_t expectValue, int32_t currentValue)
    {
        MEDIA_LOG_I("expectValue : %d, currentValue : %d", expectValue, currentValue);
        return fabs(expectValue - currentValue) < 100; // if open debug log, should use value >= 1000
    }
    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_0100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource create, reset)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_0200
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_0300
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_0400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, pause, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_0500
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, stop, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_0700
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, release, 700)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_0800
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_1000
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource setSource, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_RELEASE_CALLBACK_1200
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, reset, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource create, seek, release)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0200
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, seek, release)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0300
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, release, 300)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, pause, seek, release)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0500
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, stop, seek, release)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0600
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, reset, seek, release)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->Release());
    }

    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0700
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, setvolume, seek, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        int64_t  seekPos {5000};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_0900
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, release, 900)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_1100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, seek, seek, release)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        seekPos = 5000;
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(PREV_FRAME_TIME, currentMS));
        seekPos = 5000;
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_1300
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, -1, release)
    {
        int64_t seekPos {-1};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SEEK_CALLBACK_1400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, durationTime + 1000, release)
    {
        int64_t seekPos {0};
        int64_t currentMS {0};
        int64_t durationMs {0};
        int64_t realSeekTime {8300};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->GetDuration(durationMs));
        seekPos = durationMs + 1000;
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(realSeekTime, currentMS));
        ASSERT_EQ(0, player->Release());
    }
};