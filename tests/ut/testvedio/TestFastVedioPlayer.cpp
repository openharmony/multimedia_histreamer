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
#include "../../st/helper/test_player.hpp"

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
    class UtTestFastPlayer : public ::testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }
        std::vector<std::string> vecSource{std::string(RESOURCE_DIR "/MP4/H264_AAC.mp4")};
    };

    static const int64_t NEXT_FRAME_TIME {8300}; // 8300 next I frame position
    static const int64_t PREV_FRAME_TIME {4166}; // 4166 prev I frame position
    constexpr uint64_t FILE_SIZE = 1894335; // file size
    std::string FilePathToFd(std::string url, int32_t fileSize)
    {
        std::string uri = "fd://?offset=0&size=";
        uri += std::to_string(fileSize);
        int32_t fd = open(url.c_str(), O_RDONLY | O_BINARY);
        std::string fdStr = std::to_string(fd);
        uri.insert(5, fdStr); // 5 ---fd:://
        return uri;
    }

    bool CheckVedioTimeEquality(int32_t expectValue, int32_t currentValue)
    {
        MEDIA_LOG_I("expectValue : %d, currentValue : %d", expectValue, currentValue);
        return fabs(expectValue - currentValue) < 1000; // if open debug log, should use value >= 1000
    }

    void TestPlayerFinishedAutomatically(std::string url)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
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
        int64_t seekPos {5000}; // 5000 MS
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
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
        int64_t seekPos {5000}; // 5000 MS
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_NE(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPrepareSeekRelease(std::string url, int32_t fileSize)
    {
        int64_t seekPos {5000}; // 5000 MS
        int64_t currentMS {0};
        std::string uri = FilePathToFd(url, fileSize);
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(uri)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Seek(seekPos));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlaySeekRelease2(std::string url, int32_t fileSize) // XXX_VIDEO_PLAYER_SEEK_CALLBACK_0300
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
        ASSERT_EQ(0, player->Release());
    }

    void TestPreparePlayStopSeekRelease(std::string url, int32_t fileSize)
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
        int64_t seekPos {5000}; // 5000 MS
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
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
        seekPos = 5000; // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckVedioTimeEquality(PREV_FRAME_TIME, currentMS));
        seekPos = 5000; // 5000 MS
        ASSERT_EQ(0, player->Seek(seekPos, OHOS::Media::PlayerSeekMode::SEEK_NEXT_SYNC));
        ASSERT_EQ(0, player->GetCurrentTime(currentMS));
        EXPECT_TRUE(CheckVedioTimeEquality(NEXT_FRAME_TIME, currentMS));
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
        int64_t realSeekTime {8300}; // 8300 MS
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
        EXPECT_TRUE(CheckVedioTimeEquality(realSeekTime, currentMS));
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
        int64_t seekPos {5000}; // 8300 MS
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
        ASSERT_EQ(0, player->Seek(durationMs/2)); // half duration
        ASSERT_EQ(0, player->Seek(0));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000 MS
        ASSERT_EQ(0, player->GetDuration(durationMs));
        ASSERT_EQ(0, player->Seek(durationMs, OHOS::Media::PlayerSeekMode::SEEK_PREVIOUS_SYNC));
        std::this_thread::sleep_for(std::chrono::milliseconds(8000)); // 8000 MS
        ASSERT_EQ(0, player->SetSingleLoop(false));
        ASSERT_EQ(0, player->Release());
    }

    HST_TEST(UtTestFastPlayer, TestPlayerFinishedAutomatically, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPlayerFinishedAutomatically(url);
        }
    }

    HST_TEST(UtTestFastPlayer, TestSinglePlayerFdSourceFinishedAutomatically, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestSinglePlayerFdSourceFinishedAutomatically(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestSinglePlayerWrongFd, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestSinglePlayerWrongFd(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPauseRelease(url);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPauseThenRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPauseThenRelease(url);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepareWrongFdThenRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepareWrongFdThenRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepareThenRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepareThenRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPausePrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPausePrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayStopPrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayStopPrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayResetSetSourcePrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayResetSetSourcePrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekPrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekPrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySetvolumePrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySetvolumePrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, Test3PrepareRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            Test3PrepareRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPausePlayRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPausePlayRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayStopPlayRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayStopPlayRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayResetPlayRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayResetPlayRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySetvolumeRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySetvolumeRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPlayRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPlayRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepare3PlayRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepare3PlayRelease(url, FILE_SIZE);
        }
    }

    // fast2
    HST_TEST(UtTestFastPlayer, TestCreatePauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestCreatePauseRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePauseRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayStopPauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayStopPauseRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayResetPauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayResetPauseRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekPauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekPauseRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySetvolumePauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySetvolumePauseRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestCreateSetSourcePauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestCreateSetSourcePauseRelease(url, FILE_SIZE);
        }
    }


    HST_TEST(UtTestFastPlayer, TestPreparePlay3PauseRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlay3PauseRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestCreateStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestCreateStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepareStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepareStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPauseStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPauseStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayResetStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayResetStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySetvolumeStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySetvolumeStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySpeedStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySpeedStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestCreateSetSourceStopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestCreateSetSourceStopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlay3StopRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlay3StopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepareResetRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlay3StopRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayResetRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayResetRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPauseResetRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPauseResetRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayStopResetRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayStopResetRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekResetRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekResetRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepare3ResetRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepare3ResetRelease(url, FILE_SIZE);
        }
    }

    // fast3
    HST_TEST(UtTestFastPlayer, TestCreateReset, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestCreateReset(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestCreateSeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestCreateSeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPrepareSeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepareSeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekRelease_300, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekRelease2(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPauseSeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPauseSeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayStopSeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayStopSeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayResetSeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayResetSeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySetvolumeSeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySetvolumeSeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestSetSourceSeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestSetSourceSeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlay3SeekRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlay3SeekRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekOutValueRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekOutValueRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekOutValue2Release, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekOutValue2Release(url, FILE_SIZE);
        }
    }
    //fast4
    HST_TEST(UtTestFastPlayer, TestPrepareSetvolumeRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPrepareSetvolumeRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayPauseSetvolumeRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayPauseSetvolumeRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayStopSetvolumeRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayStopSetvolumeRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlayResetSetvolumeRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlayResetSetvolumeRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySeekSetvolumeRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySeekSetvolumeRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestSetSourceSetvolumeRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestSetSourceSetvolumeRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySetvolumeErrorValueRelease, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySetvolumeErrorValueRelease(url, FILE_SIZE);
        }
    }

    HST_TEST(UtTestFastPlayer, TestPreparePlaySetvolumeErrorValue2Release, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestPreparePlaySetvolumeErrorValue2Release(url, FILE_SIZE);
        }
    }

    //prepare, setsingleloop true, play, seek, durationtime 3 times, setsingleloop flase, release
    HST_TEST(UtTestFastPlayer, TestSetSingleLoop, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestSetSingleLoop(url, FILE_SIZE);
        }
    }

    //prepare, setsingleloop true, play, seek, set fd, seek 2 times, setsingleloop false, release
    HST_TEST(UtTestFastPlayer, TestSetSingleLoop2, TestSize.Level1)
    {
        for (auto url : vecSource)
        {
            TestSetSingleLoop2(url, FILE_SIZE);
        }
    }

} // namespace Test
} // namespace Media
} // namespace OHOS
