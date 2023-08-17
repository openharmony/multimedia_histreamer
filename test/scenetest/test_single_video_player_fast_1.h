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
#ifndef TEST_SINGLE_VIDEO_PLAYER_FAST_1_H
#define TEST_SINGLE_VIDEO_PLAYER_FAST_1_H

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

using namespace OHOS::Media::Test;

// @fixture(tags=video_play_fast)
FIXTURE(dataDrivenSingleVideoPlayerTestFast1)
{
    DATA_PROVIDER(myurls, 1,
                  DATA_GROUP(std::string(RESOURCE_DIR "/MP4/H264_AAC.mp4")));
    DATA_PROVIDER(myfdurl, 2,
                  DATA_GROUP(std::string(RESOURCE_DIR "/MP4/H264_AAC.mp4"), 1894335),
                  DATA_GROUP(std::string(RESOURCE_DIR "/../demo_resource/video/1h264_320x240_60.3gp"), 494522));
    static const int64_t NEXT_FRAME_TIME {8300};
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

    // @test(data="myurls", tags=video_play_fast)
    PTEST((std::string url), Test single player play url video, and finished automatically)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test single player play fd source, and finished automatically)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_fdSrc_CALLBACK_0100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test single player wrong fd)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        uri.insert(5, "-1"); // 5 ---fd:://
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_fdSrc_CALLBACK_0200
    // @test(data="myurls", tags=video_play_fast)
    PTEST((std::string url), Test fdsource prepare, play, pause, release)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_fdSrc_CALLBACK_0300
    // @test(data="myurls", tags=video_play_fast)
    PTEST((std::string url), Test fdsource prepare, play, pause, release)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_fdSrc_CALLBACK_0400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare then release)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        uri.insert(5, "-123456789"); // 5 ---fd:://
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_0100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare then release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_0200
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, prepare, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_0300
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, pause, prepare, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_0400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, stop, prpeare, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_0500
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, reset, setsource, prepare, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_0600/0700
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, Seek, prepare, release)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_0800
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, setvolume, prepare, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_1000
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PREPARE_CALLBACK_1200
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, prepare, prepare, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_NE(0, player->Prepare());
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource create, play, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    // // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0200
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

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0300
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, pause, play, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, stop, play, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0500
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, reset, play, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0600
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, release, 600)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0800
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

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_1000
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource  play, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_1200
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, play, play, release)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_NE(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_NE(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Release());
    }
};
#endif