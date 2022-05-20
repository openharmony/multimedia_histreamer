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

// @fixture(tags=debuging)
FIXTURE(DataDrivenSingleAudioRecorderTestFast)
{
    // file name: 44100_2_02.pcm,  44100 - sample rate, 2 - channel count, 02 - file index
    DATA_PROVIDER(pcmSources, 10,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 320000))
    );

    SETUP()
    {
        OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(TestRecorder::GetOutputDir());
    }

    TEARDOWN()
    {
    }

    // @test(data="pcmSources", tags=fast)
    PTEST((AudioRecordSource recordSource), Test single audio recorder)
    {
        int32_t ret = 0;
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(30000));
        ASSERT_EQ(0, recorder->Stop());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::string filePath;
        ASSERT_TRUE(recorder->GetRecordedFile(filePath) > 0);

        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::seconds(1000));
        ASSERT_EQ(0, player->Stop());
    }
};
