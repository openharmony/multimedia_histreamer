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
FIXTURE(dataDrivenSingleVideoPlayerTestFast4)
{
    DATA_PROVIDER(myfdurl, 2,
                  DATA_GROUP(std::string(RESOURCE_DIR "/MP4/H264_AAC.mp4"), 1894335),
                  DATA_GROUP(std::string(RESOURCE_DIR "/../demo_resource/video/1h264_320x240_60.3gp"), 494522));
    std::string FilePathToFd(std::string url, int32_t fileSize)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        int32_t fd = open(url.c_str(), O_RDONLY|O_BINARY);
        std::string fdStr = std::to_string(fd);
        uri.insert(5, fdStr); // 5 ---fd:://
        return uri;
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_0100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_0200
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_PLAY_CALLBACK_0300
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

    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_0400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, pause, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_0500
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, stop, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_0600
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, reset, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_0700
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, seek, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        int64_t seekPos {5000};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_0900
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource setsourse, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_1100
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, setvolume, setvolume, setvolume, release)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_1300
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, setVolume, -1, release)
    {
        float leftVolume {-1};
        float rightVolume {-1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_NE(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_SETVOLUME_CALLBACK_1400
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, play, setVolume, 2, release)
    {
        float leftVolume {2};
        float rightVolume {2};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_NE(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    //  SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_FUNCTION_CALLBACK_LOOP
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, setsingleloop true, play, seek, durationtime
            3 times, setsingleloop flase, release)
    {
        int64_t durationMs {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->SetSingleLoop(true));
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(8000));
        ASSERT_EQ(0, player->SetSingleLoop(false));
        ASSERT_EQ(0, player->Release());
    }

    // SUB_MULTIMEDIA_MEDIA_VIDEO_PLAYER_FUNCTION_CALLBACK_BASE
    // @test(data="myfdurl", tags=video_play_fast)
    PTEST((std::string url, int32_t fileSize), Test fdsource prepare, setsingleloop true, play, seek, set fd, seek
            2 times, setsingleloop false, release)
    {
        int64_t durationMs {0};
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Reset());
        std::string uri = FilePathToFd(url, fileSize);
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->SetSingleLoop(true));
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs/2));
        ASSERT_EQ(0, player->Seek(0));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(8000));
        ASSERT_EQ(0, player->SetSingleLoop(false));
        ASSERT_EQ(0, player->Release());
    }
};