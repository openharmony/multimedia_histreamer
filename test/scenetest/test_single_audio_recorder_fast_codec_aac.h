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
FIXTURE(dataDrivenSingleAudioRecorderTestFastCodecAac)
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

    DATA_PROVIDER(pcm_1_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/16000_1_01.pcm"), 16000, 1, 320000)));

    DATA_PROVIDER(pcm_2_8000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/8000_2_01.pcm"), 8000, 2, 320000)));

    DATA_PROVIDER(pcm_1_32000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/32000_1_01.pcm"), 32000, 1, 320000)));

    DATA_PROVIDER(pcm_2_96000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/32000_1_01.pcm"), 96000, 2, 320000)));

    DATA_PROVIDER(pcm_2_44100_8000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 8000)));

    DATA_PROVIDER(pcm_2_44100_16000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 16000)));

    DATA_PROVIDER(pcm_2_44100_32000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 32000)));

    DATA_PROVIDER(pcm_2_44100_64000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 64000)));

    DATA_PROVIDER(pcm_2_44100_112000_Sources, 4,
    DATA_GROUP(AudioRecordSource(std::string(RESOURCE_DIR "/PCM/44100_2_02.pcm"), 44100, 2, 112000)));

    // The recorder can prepare fd source, start, stop, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0100)
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

    // The recorder prepare fd one channel source, start, stop, release
    // @test(data="pcm_1_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0210)
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

    // The recorder prepare fd two channel source, start, stop, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0220)
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

    // The recorder prepare fd 2_8000 source, start, stop, release
    // @test(data="pcm_2_8000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0310)
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

    // The recorder prepare fd 1_32000 source, start, stop, release
    // @test(data="pcm_1_32000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0320)
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

    // The recorder prepare fd 2_44100 source, start, stop, release
    // @test(data="pcmSources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0330)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

    // The recorder prepare fd 2_96000 source, start, stop, release
    // @test(data="pcm_2_96000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0340)
    {
        std::unique_ptr<TestRecorder> recorder = TestRecorder::CreateAudioRecorder();
        std::string filePath = std::string(recorder->GetOutputDir() + "/test.m4a");
        fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission
        ASSERT_TRUE(fd >= 0);
        recordSource.UseOutFd(fd);
        ASSERT_EQ(0, recorder->Configure(recordSource));
        ASSERT_EQ(0, recorder->Prepare());
        ASSERT_EQ(0, recorder->Start());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

    // The recorder prepare fd 2_44100_8000 source, start, stop, release
    // @test(data="pcm_2_44100_8000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0410)
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

    // The recorder prepare fd 2_44100_16000 source, start, stop, release
    // @test(data="pcm_2_44100_16000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0420)
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

    // The recorder prepare fd 2_44100_32000 source, start, stop, release
    // @test(data="pcm_2_44100_32000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0430)
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

    // The recorder prepare fd 2_44100_64000 source, start, stop, release
    // @test(data="pcm_2_44100_64000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0440)
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

    // The recorder prepare fd 2_44100_112000 source, start, stop, release
    // @test(data="pcm_2_44100_112000_Sources", tags=audio_record_fast)
    PTEST((AudioRecordSource recordSource), SUB_MEDIA_RECORDER_CODEC_AAC_0450)
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