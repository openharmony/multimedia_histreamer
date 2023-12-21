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
#include <cmath>
#include <thread>
#include "foundation/log.h"
#include "../helper/test_player.hpp"

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#define O_BINARY 0 // which is not defined for Linux
#define RESOURCE_DIR "/data/test/media/"
using namespace testing::ext;
#endif

using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Test;

namespace OHOS {
namespace Media {
namespace Test {
    class UtTestVedioFastPlayer : public ::testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }
        std::vector<std::string> vecSource{std::string(RESOURCE_DIR "/MP4/MPEG2_MP3.mp4")};
    };

    constexpr int64_t FILE_SIZE = 1894335;
    
    std::string FilePathToFd(std::string url, int32_t fileSize)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        int32_t fd = open(url.c_str(), O_RDONLY | O_BINARY);
        std::string fdStr = std::to_string(fd);
        uri.insert(5, fdStr); // 5 ---fd:://
        return uri;
    }

    void TestPlayerFinishedAutomatically(std::string url)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::vector<OHOS::Media::Format> videoTrack;
        ASSERT_EQ(0, player->GetVideoTrackInfo(videoTrack));
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        }
    }

    void TestSinglePlayerFdSourceFinishedAutomatically(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        }
    }

    void TestSinglePlayerWrongFd(std::string url, int32_t fileSize)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        uri.insert(5, "-1"); // 5 ---fd:://
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPauseRelease(std::string url)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPauseThenRelease(std::string url)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepareWrongFdThenRelease(std::string url, int32_t fileSize)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        uri.insert(5, "-123456789"); // 5 ---fd:://
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepareThenRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPrepareRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPausePrepareRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayStopPrepareRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayResetSetSourcePrepareRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekPrepareRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySetvolumePrepareRelease(std::string url, int32_t fileSize)
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

    void TestPrepareRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void Test3PrepareRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_NE(0, player->Prepare());
        ASSERT_NE(0, player->Prepare());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPausePlayRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayStopPlayRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayResetPlayRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000}; // 5000 MS
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySetvolumeRelease(std::string url, int32_t fileSize)
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

    void TestPlayRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Play());
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepare3PlayRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_NE(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        ASSERT_NE(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Release());
    }
     // fast2
    void TestCreatePauseRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePauseRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayStopPauseRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayResetPauseRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekPauseRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySetvolumePauseRelease(std::string url, int32_t fileSize)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestCreateSetSourcePauseRelease(std::string url, int32_t fileSize)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlay3PauseRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_NE(0, player->Pause());
        ASSERT_NE(0, player->Pause());
        ASSERT_EQ(0, player->Release());
    }

    void TestCreateStopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepareStopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayStopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPauseStopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayResetStopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekStopRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySetvolumeStopRelease(std::string url, int32_t fileSize)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySpeedStopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->SetPlaybackSpeed(OHOS::Media::PlaybackRateMode::SPEED_FORWARD_1_00_X));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestCreateSetSourceStopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_NE(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlay3StopRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Stop());
        ASSERT_NE(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepareResetRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayResetRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPauseResetRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayStopResetRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekResetRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000}; // 5000 MS
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepare3ResetRelease(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Reset());
        ASSERT_NE(0, player->Reset());
        ASSERT_EQ(0, player->Release());
    }

    // fast3
    void TestCreateReset(std::string url, int32_t fileSize)
    {
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->Release());
    }

    void TestCreateSeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepareSeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekRelease300(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000}; // 5000 MS
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayPauseSeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000}; // 5000 MS
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayStopSeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayResetSeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000}; // 5000 MS
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Reset());
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySetvolumeSeekRelease(std::string url, int32_t fileSize)
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

    void TestSetSourceSeekRelease(std::string url, int32_t fileSize)
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

    void TestPreparePlay3SeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        seekPos = 5000; // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        seekPos = 5000; // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekOutValueRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {-1};
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekOutValue2Release(std::string url, int32_t fileSize)
    {
        int64_t seekPos {0};
        int64_t currentMS {0};
        int64_t durationMs {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->GetDuration(durationMs));
        seekPos = durationMs + 1000; // 1000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }
    //fast4
    void TestPrepareSetvolumeRelease(std::string url, int32_t fileSize)
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

    void TestPreparePlayPauseSetvolumeRelease(std::string url, int32_t fileSize)
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

    void TestPreparePlayStopSetvolumeRelease(std::string url, int32_t fileSize)
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

    void TestPreparePlayResetSetvolumeRelease(std::string url, int32_t fileSize)
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

    void TestPreparePlaySeekSetvolumeRelease(std::string url, int32_t fileSize)
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    void TestSetSourceSetvolumeRelease(std::string url, int32_t fileSize)
    {
        float leftVolume {1};
        float rightVolume {1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySetvolumeErrorValueRelease(std::string url, int32_t fileSize)
    {
        float leftVolume {-1};
        float rightVolume {-1};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_NE(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySetvolumeErrorValue2Release(std::string url, int32_t fileSize)
    {
        float leftVolume {2};
        float rightVolume {2};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_NE(0, player->SetVolume(leftVolume, rightVolume));
        ASSERT_EQ(0, player->Release());
    }

    //prepare, setsingleloop true, play, seek, durationtime 3 times, setsingleloop flase, release
    void TestSetSingleLoop(std::string url, int32_t fileSize)
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
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 5000 MS
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(8000)); // 8000 MS
        ASSERT_EQ(0, player->SetSingleLoop(false));
        ASSERT_EQ(0, player->Release());
    }

    //prepare, setsingleloop true, play, seek, set fd, seek 2 times, setsingleloop false, release
    void TestSetSingleLoop2(std::string url, int32_t fileSize)
    {
        int64_t durationMs {0};
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Pause());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Reset());
        std::string uri = FilePathToFd(url, fileSize);
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->SetSingleLoop(true));
        ASSERT_EQ(0, player->Play());
        ASSERT_TRUE(player->IsPlaying());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3000 MS
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs/2)); // 2, half the time
        ASSERT_EQ(0, player->Seek(0));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(8000)); // 8000 MS
        ASSERT_EQ(0, player->SetSingleLoop(false));
        ASSERT_EQ(0, player->Release());
    }
} // namespace Test
} // namespace Media
} // namespace OHOS
