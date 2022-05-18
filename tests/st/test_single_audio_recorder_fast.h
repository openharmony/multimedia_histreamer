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

using namespace OHOS::Media::Test;
 // @fixture(tags=debuging)
FIXTURE(DataDrivenSingleAudioRecorderTestFast)
{
    int32_t getFileSize(std::string inputPath,std::string &fullPath)
    {
        char buffer[1024];
        char* ptrPath = getcwd(buffer,1024);
        if (ptrPath == nullptr) {
            return 0;
        }

        long hFile = 0;
        std::string strFindPath;
        struct _finddata_t fileinfo;
        fullPath.assign(buffer).append("\\").append(inputPath);
        strFindPath.assign(fullPath).append("\\*.*");
        if ((hFile = _findfirst(strFindPath.c_str(), &fileinfo)) != -1) {
            do
            {
                if ((fileinfo.attrib &  _A_SUBDIR)) {
                    continue;
                } else {
                    fullPath.append("\\").append(fileinfo.name);
                    break;
                }
            } while (_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
            return fileinfo.size;
        }
        return 0;
    }
    DATA_PROVIDER(pcmSources, 100,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/AudioData.pcm"), 44100, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/8000_1_01.pcm"), 8000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/8000_2_01.pcm"), 8000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/11025_2_01.pcm"), 11025, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/12000_2_01.pcm"), 12000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/16000_1_01.pcm"), 16000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/16000_2_01.pcm"), 16000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/16000_1_02.pcm"), 16000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/22050_2_01.pcm"), 22050, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/24000_2_01.pcm"), 24000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/32000_1_01.pcm"), 32000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_01.pcm"), 44100, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/48000_1_01.pcm"), 48000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/48000_1_02.pcm"), 48000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/48000_1_03.pcm"), 48000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/48000_2_01.pcm"), 48000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/48000_2_02.pcm"), 48000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/48000_2_03.pcm"), 48000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/48000_2_04.pcm"), 48000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/64000_1_01.pcm"), 64000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/96000_1_01.pcm"), 96000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/96000_1_02.pcm"), 96000, 1, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/96000_2_01.pcm"), 96000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/96000_2_02.pcm"), 96000, 2, 320000)),
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/96000_2_03.pcm"), 96000, 2, 320000))
    );

    // @test(data="pcmSources", tags=fast)
    PTEST((AudioRecordSource recordSource), Test single audio recorder)
    {
        int32_t ret = 0;
        std::string strPath;
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, recorder->Pause());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      //  ASSERT_EQ(0, recorder->Resume());
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, getFileSize(recordSource.getPcmPath(),strPath) < 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(strPath)));
        ASSERT_EQ(0, player->SetSingleLoop(true));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::seconds(1000));
        ASSERT_EQ(0, player->Stop());
    }
};
