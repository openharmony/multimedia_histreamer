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
#else
#include <direct.h>
#endif
#include <format.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <thread>
#include "helper/test_recorder.hpp"
#include "helper/test_player.hpp"
#include "testngpp/testngpp.hpp"
#include "foundation/log.h"
#include "foundation/osal/filesystem/file_system.h"
#include "test_single_audio_recorder_fast.h"

using namespace OHOS::Media::Test;

// @fixture(tags=fast)
FIXTURE(dataDrivenSingleAudioRecorderTestFastStartPause)
{
    int32_t fd;

    // file name: 44100_2_02.pcm,  44100 - sample rate, 2 - channel count, 02 - file index
    DATA_PROVIDER(pcmSources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 320000)));

    // The recorder  start error and release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0100)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_NE(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
    }
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0200)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, pause, start error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0300)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, pause, resume, start, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0400)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Resume());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, stop, start, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0500)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_NE(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, reset, start, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0600)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_NE(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder prepare  start stop prepare  start reset start prepare start release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0800)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);

        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_NE(0, recorder->Start());
        ASSERT_EQ(0, close(fd));

        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_NE(0, recorder->Start());
        ASSERT_EQ(0, close(fd));

        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare start start start release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0900)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder pause error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0100)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_NE(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
    }

    // The recorder can  prepare, pause error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0200)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_NE(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, pause, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0300)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, pause, resume, pause, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0400)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Resume());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare, start, stop, pause error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0500)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_NE(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, reset, pause error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0600)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_NE(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder prepare start pause resume pause stop pause error reset pause release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0800)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Resume());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_NE(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_NE(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  prepare, start, pause, pause, pause, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Pause_API_0900)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Pause());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }
};