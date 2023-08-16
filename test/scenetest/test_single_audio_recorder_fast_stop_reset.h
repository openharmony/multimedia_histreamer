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
FIXTURE(dataDrivenSingleAudioRecorderTestFastStopReset)
{
    int32_t fd;
    bool CheckDurationMs(int64_t expectValue, int64_t actualValue)
    {
        MEDIA_LOG_I("expectValue : %d, actualValue : %d", expectValue, actualValue);
        return true;
    }

    bool CheckTrackInfo(std::vector<OHOS::Media::Format> &audioTrack, AudioRecordSource recordSource)
    {
        int32_t audioSampleRate;
        int32_t audioBitRate;
        std::string audioMime;
        int32_t audioChannels;
        audioTrack[0].GetIntValue("bitrate", audioBitRate);
        audioTrack[0].GetIntValue("sample_rate", audioSampleRate);
        audioTrack[0].GetIntValue("channel_count", audioChannels);
        audioTrack[0].GetStringValue("codec_mime", audioMime);
        std::string configMime = "audio/mpeg";
        int32_t configSampleRate;
        int64_t configBitRate;
        int32_t configChannel;
        recordSource.GetBitRate(configBitRate);
        recordSource.GetChannel(configChannel);
        recordSource.GetSampleRate(configSampleRate);
        ASSERT_TRUE(audioSampleRate == configSampleRate);
        ASSERT_TRUE(audioChannels == configChannel);
        ASSERT_TRUE(audioMime == configMime);
        return true;
    }

    void CheckAudio(std::string filePath, AudioRecordSource recordSource)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        int64_t durationTime;
        int64_t recordTime = 1000;
        std::vector<OHOS::Media::Format> audioTrack;
        ASSERT_EQ(0, player->GetDuration(durationTime));
        ASSERT_EQ(0, player->GetAudioTrackInfo(audioTrack));
        ASSERT_TRUE(CheckDurationMs(recordTime, durationTime));
        ASSERT_TRUE(CheckTrackInfo(audioTrack, recordSource));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
    }

    // file name: 44100_2_02.pcm,  44100 - sample rate, 2 - channel count, 02 - file index
    DATA_PROVIDER(pcmSources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 320000)));

    // The recorder stop error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0100)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_NE(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Release());
    }

    // The recorder can prepare, stop error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0200)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_NE(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare, start, stop, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0300)
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
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare, start, pause, stop, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0400)
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
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare, start, pause, resume, stop, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0500)
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
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can preapre, start, reset, stop error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0600)
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
        ASSERT_NE(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can not stop after prepare, then can prepare after reset
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0800)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_NE(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, close(fd));

        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, close(fd));

        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());

        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_NE(0, recorder->Stop());
        (void)recorder->Configure(recordSource); // errorState configure fail.
        (void)recorder->Prepare(); // prepare fail.
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare, start, stop, stop error, stop error, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0900)
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
        ASSERT_NE(0, recorder->Stop());
        ASSERT_NE(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can  reset release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0100)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Release());
    }

    // The recorder can prepare reset release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0200)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare start reset release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0300)
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
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare start reset release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0400)
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
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare start, pause, resume, reset, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0500)
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
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare start, pause, resume, reset, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0600)
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
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder prepare reset prepare start reset prepare start stop reset prepare start release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0800)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, close(fd));

        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, close(fd));

        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Reset());
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

    // The recorder can prepare, start, reset, reset, reset, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0900)
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
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Reset());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }
};