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

using namespace OHOS::Media::Test;

// @fixture(tags=fast)
FIXTURE(dataDrivenSingleAudioRecorderTestFast)
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

    // file name: 44100_2_02.pcm,  44100 - sample rate, 2 - channel count, 02 - file index
    DATA_PROVIDER(pcmSources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 320000)));

    DATA_PROVIDER(pcm_2_22050_22050_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/22050_2_01.pcm"), 22050, 2, 22050)));

    SETUP()
    {
        OHOS::Media::OSAL::FileSystem::MakeMultipleDir(TestRecorder::GetOutputDir());
        OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(TestRecorder::GetOutputDir());
    }

    TEARDOWN()
    {
        close(fd);
    }

    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), Test single audio fd recorder)
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Pause());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, recorder->Resume());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, close(fd));
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        ASSERT_EQ(0, player->Stop());
    }

    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), Test single audio recorder)
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Pause());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(0, recorder->Resume());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, close(fd));
    }

    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), The recorder can be stopped and set source again)
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, close(fd));

        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        // set source and record again
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, close(fd));

        ASSERT_EQ(0, player->Reset());
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        ASSERT_EQ(0, player->Stop());
    }

    // The recorder can create audioRecorder
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_createAudioRecorder_API_0100)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_TRUE(recorder != nullptr);
    }

    // The recorder can release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0100)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        ASSERT_EQ(0, recorder->Release());
    }

    // The recorder can prepare, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0200)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");

        // Don't add O_APPEND, or else seek fail, can not write the file length.
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare, start, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0300)
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

    // The recorder can prepare, start, pause, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0400)
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

    // The recorder can prepare, start, pause, resume, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0500)
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
        ASSERT_EQ(0, recorder->Release());
        ASSERT_EQ(0, close(fd));
    }

    // The recorder can prepare start stop release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0600)
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

    // The recorder can prepare start pause reset release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0700)
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

    // The recorder prepare fd 2_22050_22050 source, start, stop, release
    // @test(data="pcm_2_22050_22050_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_FORMAT_MP4_0200)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ASSERT_EQ(0, recorder->Stop());
        ASSERT_EQ(0, recorder->Release());
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        int64_t recorderTime = 1000;
        int64_t DurationTime;
        std::vector<OHOS::Media::Format> audioTrack;
        ASSERT_EQ(0, player->GetDuration(DurationTime));
        ASSERT_EQ(0, player->GetAudioTrackInfo(audioTrack));
        ASSERT_TRUE(CheckDurationMs(recorderTime, DurationTime));
        ASSERT_TRUE(CheckTrackInfo(audioTrack, recordSource));
        ASSERT_EQ(0, player->Stop());
        ASSERT_EQ(0, player->Release());
        ASSERT_EQ(0, close(fd));
    }
};