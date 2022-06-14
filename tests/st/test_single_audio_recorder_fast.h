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
#include <fcntl.h>
#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <math.h>
#include <thread>
#include <fstream>
#include <iostream>
#include <sstream>
#include "helper/test_recorder.hpp"
#include "helper/test_player.hpp"
#include "testngpp/testngpp.hpp"
#include "foundation/log.h"
#include "foundation/osal/filesystem/file_system.h"

using namespace OHOS::Media::Test;

// @fixture(tags=audio_record_fast)
FIXTURE(DataDrivenSingleAudioRecorderTestFast)
{
    // file name: 44100_2_02.pcm,  44100 - sample rate, 2 - channel count, 02 - file index
    DATA_PROVIDER(pcmSources, 1,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 320000)));

    SETUP()
    {
        OHOS::Media::OSAL::FileSystem::MakeMultipleDir(TestRecorder::GetOutputDir());
        OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(TestRecorder::GetOutputDir());
    }

    TEARDOWN()
    {
    }

    // @test(data="pcmSources", tags=fast)
    PTEST((AudioRecordSource recordSource), Test single audio fd recorder)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

         // Don't add O_APPEND, or else seek fail, can not write the file length.
        int32_t fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, recorder->Pause());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, recorder->Resume());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, close(fd));
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="pcmSources", tags=fast)
    PTEST((AudioRecordSource recordSource), Test single audio recorder)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, recorder->Pause());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, recorder->Resume());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, recorder->Stop());

        std::string filePath;
        ASSERT_TRUE(recorder->GetRecordedFile(filePath) > 0);

        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="pcmSources", tags=fast)
    PTEST((AudioRecordSource recordSource), The recorder can be stopped and set source again)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());

        std::string filePath;
        ASSERT_TRUE(recorder->GetRecordedFile(filePath) > 0);

        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());

        // set source and record again
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());

        ASSERT_TRUE(recorder->GetRecordedFile(filePath) > 0);

        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="pcmSources", tags=debuging)
    PTEST((AudioRecordSource recordSource), The can not stop after prepare, then can prepare after reset)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_NE(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());

        std::string filePath;
        ASSERT_TRUE(recorder->GetRecordedFile(filePath) > 0);

        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());

        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_NE(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Release());
    }
};
