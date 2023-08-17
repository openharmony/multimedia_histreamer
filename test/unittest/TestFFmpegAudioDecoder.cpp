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
#include "plugin/plugins/ffmpeg_adapter/audio_decoder/audio_ffmpeg_decoder_plugin.h"
#include "plugin/common/plugin_caps_builder.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin;
using namespace Ffmpeg;
using namespace testing::ext;
std::shared_ptr<CodecPlugin> AuFfmpegDecoderCreator(const std::string& name)
{
    return std::make_shared<AudioFfmpegDecoderPlugin>(name);
}

HWTEST(AudioFfmpegDecoderPluginTest, test_Init, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ASSERT_EQ(Status::ERROR_UNSUPPORTED_FORMAT, aDecoderPlugin->Init());
}

HWTEST(AudioFfmpegDecoderPluginTest, test_Prepare, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aDecoderPlugin->Prepare());
}

HWTEST(AudioFfmpegDecoderPluginTest, test_Reset, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aEncoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ASSERT_EQ(Status::OK, aEncoderPlugin->Reset());
}

HWTEST(AudioFfmpegDecoderPluginTest, test_Start, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aDecoderPlugin->Start());
}

HWTEST(AudioFfmpegDecoderPluginTest, test_Stop, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ASSERT_EQ(Status::OK, aDecoderPlugin->Stop());
}

HWTEST(AudioFfmpegDecoderPluginTest, test_SetParameter, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ValueType value = 128;
    ASSERT_EQ(Status::OK, aDecoderPlugin->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, &value));
}

HWTEST(AudioFfmpegDecoderPluginTest, test_GetParameter, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ValueType value;
    ASSERT_EQ(Status::ERROR_INVALID_PARAMETER, aDecoderPlugin->GetParameter(Tag::REQUIRED_OUT_BUFFER_CNT, value));
}

HWTEST(AudioFfmpegDecoderPluginTest, test_Flush, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    ASSERT_EQ(Status::OK, aDecoderPlugin->Flush());
}

HWTEST(AudioFfmpegDecoderPluginTest, test_QueInputBuffer, TestSize.Level1)
{
    std::shared_ptr<CodecPlugin> aDecoderPlugin = AuFfmpegDecoderCreator("AudioFfmpegDecoderPluginTest");
    std::shared_ptr<Buffer> inputBuffer = std::make_shared<Buffer>(BufferMetaType::AUDIO);
    int32_t timeoutMs = 100;
    ASSERT_EQ(Status::ERROR_INVALID_DATA, aDecoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
    uint32_t size = 16;
    inputBuffer->AllocMemory(nullptr, size);
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aDecoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
    inputBuffer->flag = 1;
    ASSERT_EQ(Status::ERROR_WRONG_STATE, aDecoderPlugin->QueueInputBuffer(inputBuffer, timeoutMs));
}

} //namespace Test
} //namespace Media
} //namespace OHOS