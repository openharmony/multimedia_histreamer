/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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
#include "plugin/plugins/ffmpeg_adapter/audio_encoder/audio_ffmpeg_encoder_plugin.h"
#include "plugin/plugins/ffmpeg_adapter/audio_encoder//ffmpeg_au_enc_config.h"
#include "plugin/common/plugin_caps_builder.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin;
using namespace Ffmpeg;
using namespace testing::ext;
std::shared_ptr<CodecPlugin> AuFfmpegEncoderCreator(const std::string& name)
{
    return std::make_shared<AudioFfmpegEncoderPlugin>(name);
}

HWTEST(AudioFfmpegEncoderPluginTest, test_Init, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ASSERT_EQ(Status::ERROR_UNSUPPORTED_FORMAT, aEncoderPlugin->Init());
}

HWTEST(AudioFfmpegEncoderPluginTest, test_Prepare, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aEncoderPlugin->Prepare());
}

HWTEST(AudioFfmpegEncoderPluginTest, test_Reset, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ASSERT_EQ(Status::OK, aEncoderPlugin->Reset());
}

HWTEST(AudioFfmpegEncoderPluginTest, test_Start, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aEncoderPlugin->Start());
}

HWTEST(AudioFfmpegEncoderPluginTest, test_Stop, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ASSERT_EQ(Status::OK, aEncoderPlugin->Stop());
}

HWTEST(AudioFfmpegEncoderPluginTest, test_SetParameter, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ValueType value = 128;
    ASSERT_EQ(Status::OK, aEncoderPlugin->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, &value));
}

HWTEST(AudioFfmpegEncoderPluginTest, test_GetParameter, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ValueType value;
    ASSERT_EQ(Status::OK, aEncoderPlugin->GetParameter(Tag::REQUIRED_OUT_BUFFER_CNT, value));
}

HWTEST(AudioFfmpegEncoderPluginTest, test_Flush, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    ASSERT_EQ(Status::OK, aEncoderPlugin->Flush());
}

HWTEST(AudioFfmpegEncoderPluginTest, test_QueInputBuffer, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegEncoderCreator("AudioFfmpegEncoderPluginTest");
    std::shared_ptr<Buffer> inputBuffer = std::make_shared<Buffer>(BufferMetaType::AUDIO);
    int32_t timeoutMs = 100;
    ASSERT_EQ(Status::ERROR_INVALID_DATA, aEncoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
    uint32_t size = 16;
    inputBuffer->AllocMemory(nullptr, size);
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aEncoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
    inputBuffer->flag = 1;
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aEncoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
}

} //namespace Test
} //namespace Media
} //namespace OHOS