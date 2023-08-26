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
#include <format.h>
#include <fstream>
#include <thread>
#include "gtest/gtest.h"
#include "../helper/test_player.hpp"
#include "foundation/log.h"
#include "foundation/osal/filesystem/file_system.h"
#include "i_engine_factory.h"
#include "i_recorder_engine.h"
#include "recorder_param.h"
#include "media_errors.h"
#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#define O_BINARY 0 // which is not defined for Linux
#define RESOURCE_DIR "/data/test/media/"
#define HST_WORKING_DIR "/data/test/media/"
using namespace testing::ext;
#endif

extern "C" {
__attribute__((visibility("default"))) OHOS::Media::IEngineFactory* CreateEngineFactory();
}

using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Test;

namespace OHOS {
namespace Media {
namespace Test {

using namespace OHOS::Media;

class RecorderEngineObs : public IRecorderEngineObs {
public:
    ~RecorderEngineObs() override = default;

    void OnError(ErrorType errorType, int32_t errorCode) override
    {
        std::cout<< "error"<< errorCode <<std::endl;
    }

    void OnInfo(InfoType type, int32_t extra) override
    {
    }
};

std::shared_ptr<IRecorderEngine> CreateRecorder()
{
    auto engineFactory = std::shared_ptr<OHOS::Media::IEngineFactory>(CreateEngineFactory());
    auto recorder = engineFactory->CreateRecorderEngine(0, 0, 0, 0); // 0
    auto obs = std::make_shared<RecorderEngineObs>();
    recorder->SetObs(obs);
    return recorder;
}

void CheckAudio(std::string filePath)
{
    std::unique_ptr<TestPlayer> player = TestPlayer::Create();
    ASSERT_EQ(0, player->SetSource(TestSource(filePath)));
    ASSERT_EQ(0, player->Prepare());
    ASSERT_EQ(0, player->Play());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 500 MS
    ASSERT_EQ(0, player->Stop());
    ASSERT_EQ(0, player->Release());
}

HWTEST(TestFastAudioRecorder, Test_single_audio_fd_recorder_1, TestSize.Level1)
{
    std::string filePath = std::string(std::string(HST_WORKING_DIR) + "/test.m4a");
    OHOS::Media::OSAL::FileSystem::MakeMultipleDir(std::string(HST_WORKING_DIR));
    OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(std::string(HST_WORKING_DIR));
    int fd;
    fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission;
    ASSERT_TRUE(fd >= 0);
    auto recorder = CreateRecorder();
    int32_t audioSourceId = 0;
    recorder->SetAudioSource(AudioSourceType::AUDIO_MIC, audioSourceId);
    recorder->SetOutputFormat(OutputFormatType::FORMAT_M4A);
    auto audSampleRate = AudSampleRate{44100};
    auto audChannel = AudChannel{2};
    auto audBitRate = AudBitRate{320000};
    auto auEncoder = AudEnc{AudioCodecFormat::AAC_LC};
    recorder->Configure(audioSourceId, audSampleRate);
    recorder->Configure(audioSourceId, audChannel);
    recorder->Configure(audioSourceId, audBitRate);
    recorder->Configure(audioSourceId, auEncoder);
    auto outFileFD = OutFd {fd};
    recorder->Configure(DUMMY_SOURCE_ID, outFileFD);
    ASSERT_EQ(0, recorder->Prepare());
    ASSERT_EQ(0, recorder->Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Stop());
    ASSERT_EQ(0, close(fd));
}

HWTEST(TestFastAudioRecorder, Test_single_audio_fd_recorder_2, TestSize.Level1)
{
    std::string filePath = std::string(std::string(HST_WORKING_DIR) + "/test.m4a");
    OHOS::Media::OSAL::FileSystem::MakeMultipleDir(std::string(HST_WORKING_DIR));
    OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(std::string(HST_WORKING_DIR));
    int fd;
    fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission;
    ASSERT_TRUE(fd >= 0);
    auto recorder = CreateRecorder();
    int32_t audioSourceId = 0;
    recorder->SetAudioSource(AudioSourceType::AUDIO_MIC, audioSourceId);
    recorder->SetOutputFormat(OutputFormatType::FORMAT_M4A);
    auto audSampleRate = AudSampleRate{44100};
    auto audChannel = AudChannel{2};
    auto audBitRate = AudBitRate{320000};
    auto auEncoder = AudEnc{AudioCodecFormat::AAC_LC};
    recorder->Configure(audioSourceId, audSampleRate);
    recorder->Configure(audioSourceId, audChannel);
    recorder->Configure(audioSourceId, audBitRate);
    recorder->Configure(audioSourceId, auEncoder);
    auto outFileFD = OutFd {fd};
    recorder->Configure(DUMMY_SOURCE_ID, outFileFD);
    ASSERT_EQ(0, recorder->Prepare());
    ASSERT_EQ(0, recorder->Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Stop());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Reset());
    ASSERT_EQ(0, close(fd));
}

HWTEST(TestFastAudioRecorder, Test_single_audio_fd_recorder_3, TestSize.Level1)
{
    std::string filePath = std::string(std::string(HST_WORKING_DIR) + "/test.m4a");
    OHOS::Media::OSAL::FileSystem::MakeMultipleDir(std::string(HST_WORKING_DIR));
    OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(std::string(HST_WORKING_DIR));
    int fd;
    fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission;
    ASSERT_TRUE(fd >= 0);
    auto recorder = CreateRecorder();
    int32_t audioSourceId = 0;
    recorder->SetAudioSource(AudioSourceType::AUDIO_MIC, audioSourceId);
    recorder->SetOutputFormat(OutputFormatType::FORMAT_M4A);
    auto audSampleRate = AudSampleRate{44100};
    auto audChannel = AudChannel{2};
    auto audBitRate = AudBitRate{320000};
    auto auEncoder = AudEnc{AudioCodecFormat::AAC_LC};
    recorder->Configure(audioSourceId, audSampleRate);
    recorder->Configure(audioSourceId, audChannel);
    recorder->Configure(audioSourceId, audBitRate);
    recorder->Configure(audioSourceId, auEncoder);
    auto outFileFD = OutFd {fd};
    recorder->Configure(DUMMY_SOURCE_ID, outFileFD);
    ASSERT_EQ(0, recorder->Prepare());
    ASSERT_EQ(0, recorder->Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Pause());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Stop());
    ASSERT_EQ(0, close(fd));
}

HWTEST(TestFastAudioRecorder, Test_single_audio_fd_recorder_4, TestSize.Level1)
{
    std::string filePath = std::string(std::string(HST_WORKING_DIR) + "/test.m4a");
    OHOS::Media::OSAL::FileSystem::MakeMultipleDir(std::string(HST_WORKING_DIR));
    OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(std::string(HST_WORKING_DIR));
    int fd;
    fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission;
    ASSERT_TRUE(fd >= 0);
    auto recorder = CreateRecorder();
    int32_t audioSourceId = 0;
    recorder->SetAudioSource(AudioSourceType::AUDIO_MIC, audioSourceId);
    recorder->SetOutputFormat(OutputFormatType::FORMAT_M4A);
    auto audSampleRate = AudSampleRate{44100};
    auto audChannel = AudChannel{2};
    auto audBitRate = AudBitRate{320000};
    auto auEncoder = AudEnc{AudioCodecFormat::AAC_LC};
    recorder->Configure(audioSourceId, audSampleRate);
    recorder->Configure(audioSourceId, audChannel);
    recorder->Configure(audioSourceId, audBitRate);
    recorder->Configure(audioSourceId, auEncoder);
    auto outFileFD = OutFd {fd};
    recorder->Configure(DUMMY_SOURCE_ID, outFileFD);
    ASSERT_EQ(0, recorder->Prepare());
    ASSERT_EQ(0, recorder->Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Pause());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Stop());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(0, recorder->Reset());
    ASSERT_EQ(0, close(fd));
}

HWTEST(TestFastAudioRecorder, Test_single_audio_fd_recorder_5, TestSize.Level1)
{
    std::string filePath = std::string(std::string(HST_WORKING_DIR) + "/test.m4a");
    OHOS::Media::OSAL::FileSystem::MakeMultipleDir(std::string(HST_WORKING_DIR));
    OHOS::Media::OSAL::FileSystem::RemoveFilesInDir(std::string(HST_WORKING_DIR));
    int fd;
    fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_BINARY, 0644); // 0644, permission;
    ASSERT_TRUE(fd >= 0);
    auto recorder = CreateRecorder();
    int32_t audioSourceId = 0;
    recorder->SetAudioSource(AudioSourceType::AUDIO_MIC, audioSourceId);
    recorder->SetOutputFormat(OutputFormatType::FORMAT_M4A);
    auto audSampleRate = AudSampleRate{44100};
    auto audChannel = AudChannel{2};
    auto audBitRate = AudBitRate{320000};
    auto auEncoder = AudEnc{AudioCodecFormat::AAC_LC};
    recorder->Configure(audioSourceId, audSampleRate);
    recorder->Configure(audioSourceId, audChannel);
    recorder->Configure(audioSourceId, audBitRate);
    recorder->Configure(audioSourceId, auEncoder);
    auto outFileFD = OutFd {fd};
    recorder->Configure(DUMMY_SOURCE_ID, outFileFD);
    ASSERT_EQ(0, recorder->Prepare());
    ASSERT_EQ(0, recorder->Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(0, recorder->Pause());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_EQ(0, recorder->Resume());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(0, recorder->Stop());
    CheckAudio(filePath);
    ASSERT_EQ(0, close(fd));
}

} // namespace Test
} // namespace Media
} // namespace OHOS
