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

#include "gtest/gtest.h"
#include "plugin/common/any.h"
#include "plugin/plugins/source/audio_capture/audio_capture_plugin.h"
#include "plugin/plugins/source/audio_capture/audio_type_translate.h"

using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin;
using namespace AuCapturePlugin;
using namespace AudioStandard;

#define MAX_TYPE 9
#define MP4_URL "http://www.w3school.com.cn/i/movie.mp4"

class AudioCaptureCallback : public Callback {
public:
    void OnEvent(const PluginEvent &event)
    {
        ASSERT_TRUE(static_cast<int>(event.type) >= 0);
        ASSERT_TRUE(static_cast<int>(event.type) <= MAX_TYPE);
    }
};

HWTEST(AudioCaptureTest, testAudioCapturePluginErrorBranch, TestSize.Level1)
{
    auto str = std::make_shared<std::string>("audio");
    auto audioCapture = std::make_shared<AudioCapturePlugin>(str->c_str());
    ASSERT_TRUE(audioCapture != nullptr);

    auto status = audioCapture->Init();
    ASSERT_TRUE(status == Status::OK);

    auto prepareStatus = audioCapture->Prepare();
    ASSERT_TRUE(prepareStatus == Status::OK);

    auto deInitStatus = audioCapture->Deinit();
    ASSERT_TRUE(deInitStatus == Status::OK);

    auto resetStatus = audioCapture->Reset();
    ASSERT_FALSE(resetStatus == Status::OK);

    auto prepare = audioCapture->Prepare();
    ASSERT_FALSE(prepare == Status::OK);

    auto startStatus = audioCapture->Start();
    ASSERT_FALSE(startStatus == Status::OK);

    auto stopStatus = audioCapture->Stop();
    ASSERT_TRUE(stopStatus == Status::OK);
}

HWTEST(AudioCaptureTest, testAudioCapturePluginStart, TestSize.Level1)
{
    auto str = std::make_shared<std::string>("audio");
    auto audioCapture = std::make_shared<AudioCapturePlugin>(str->c_str());
    ASSERT_TRUE(audioCapture != nullptr);

    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_RATE, AudioSamplingRate::SAMPLE_RATE_44100);
    audioCapture->SetParameter(Tag::AUDIO_CHANNELS, STEREO);
    audioCapture->SetParameter(Tag::MEDIA_BITRATE, 3000);
    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_FORMAT, SAMPLE_S32LE);
    audioCapture->SetParameter(Tag::APP_TOKEN_ID, 2000);
    audioCapture->SetParameter(Tag::APP_UID, 300);
    audioCapture->SetParameter(Tag::APP_PID, 1000);
    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, 60);

    auto initStatus = audioCapture->Init();
    ASSERT_TRUE(initStatus == Status::OK);

    auto prepareSt = audioCapture->Prepare();
    ASSERT_TRUE(prepareSt == Status::OK);

    auto resetStatus2 = audioCapture->Reset();
    ASSERT_TRUE(resetStatus2 == Status::OK);

    auto stopStatus2 = audioCapture->Stop();
    ASSERT_TRUE(stopStatus2 == Status::OK);

    auto startStatus2 = audioCapture->Start();
    ASSERT_TRUE(startStatus2 == Status::ERROR_UNKNOWN);

    auto source = std::make_shared<MediaSource>(MP4_URL);
    auto sourceSt = audioCapture->SetSource(source);
    ASSERT_TRUE(sourceSt == Status::ERROR_UNIMPLEMENTED);

    AudioCaptureCallback pluginCallback {};
    auto audioSt = audioCapture->SetCallback(&pluginCallback);
    ASSERT_TRUE(audioSt == Status::ERROR_UNIMPLEMENTED);

    auto prepareSt2 = audioCapture->Prepare();
    ASSERT_TRUE(prepareSt2 == Status::OK);

    auto startStatus3 = audioCapture->Start();
    ASSERT_TRUE(startStatus3 == Status::ERROR_UNKNOWN);
}

HWTEST(AudioCaptureTest, testAudioCapturePlugin, TestSize.Level1)
{
    auto str = std::make_shared<std::string>("audio");
    auto audioCapture = std::make_shared<AudioCapturePlugin>(str->c_str());
    ASSERT_TRUE(audioCapture != nullptr);

    auto initStatus = audioCapture->Init();
    ASSERT_TRUE(initStatus == Status::OK);

    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_RATE, AudioSamplingRate::SAMPLE_RATE_8000);
    audioCapture->SetParameter(Tag::AUDIO_CHANNELS, MONO);
    audioCapture->SetParameter(Tag::MEDIA_BITRATE, 16666);
    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_FORMAT, SAMPLE_S16LE);
    audioCapture->SetParameter(Tag::APP_TOKEN_ID, 100000);
    audioCapture->SetParameter(Tag::APP_UID, 200);
    audioCapture->SetParameter(Tag::APP_PID, 100);
    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, 30);

    auto source = std::make_shared<MediaSource>(MP4_URL);
    auto sourceSt = audioCapture->SetSource(source);
    ASSERT_TRUE(sourceSt == Status::ERROR_UNIMPLEMENTED);

    auto prepareSt2 = audioCapture->Prepare();
    ASSERT_TRUE(prepareSt2 == Status::OK);

    auto startStatus3 = audioCapture->Start();
    ASSERT_TRUE(startStatus3 == Status::ERROR_UNKNOWN);

    uint64_t size;
    auto sizeStatus = audioCapture->GetSize(size);
    ASSERT_TRUE(size == 1024);
    ASSERT_TRUE(sizeStatus == Status::OK);

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(BufferMetaType::AUDIO);
    size_t expectedLe = 10;
    auto readStatus = audioCapture->Read(buffer, expectedLe);
    ASSERT_TRUE(readStatus == Status::ERROR_AGAIN);

    Seekable seekable = audioCapture->GetSeekable();
    ASSERT_EQ(Seekable::UNSEEKABLE, seekable);
    Status seekSt = audioCapture->SeekToPos(300);
    ASSERT_EQ(seekSt, Status::ERROR_UNIMPLEMENTED);

    auto deInitStatus = audioCapture->Deinit();
    ASSERT_TRUE(deInitStatus == Status::OK);
}

HWTEST(AudioCaptureTest, testGetParams, TestSize.Level1)
{
    auto str = std::make_shared<std::string>("audio");
    auto audioCapture = std::make_shared<AudioCapturePlugin>(str->c_str());
    ASSERT_TRUE(audioCapture != nullptr);

    ValueType value;
    audioCapture->GetParameter(Tag::AUDIO_SAMPLE_RATE, value);

    auto initStatus = audioCapture->Init();
    ASSERT_TRUE(initStatus == Status::OK);

    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_RATE, AudioSamplingRate::SAMPLE_RATE_8000);
    audioCapture->SetParameter(Tag::AUDIO_CHANNELS, MONO);
    audioCapture->SetParameter(Tag::MEDIA_BITRATE, 16666);
    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_FORMAT, SAMPLE_S16LE);
    audioCapture->SetParameter(Tag::APP_TOKEN_ID, 100000);
    audioCapture->SetParameter(Tag::APP_UID, 200);
    audioCapture->SetParameter(Tag::APP_PID, 100);
    audioCapture->SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, 30);

    audioCapture->GetParameter(Tag::AUDIO_SAMPLE_RATE, value);
    ASSERT_TRUE(value.HasValue());

    audioCapture->GetParameter(Tag::AUDIO_CHANNELS, value);
    ASSERT_TRUE(value.HasValue());

    audioCapture->GetParameter(Tag::MEDIA_BITRATE, value);
    ASSERT_TRUE(value.HasValue());

    audioCapture->GetParameter(Tag::AUDIO_SAMPLE_FORMAT, value);
    ASSERT_TRUE(value.HasValue());

    audioCapture->GetParameter(Tag::AUDIO_CHANNEL_LAYOUT, value);
    ASSERT_TRUE(value.HasValue());

    audioCapture->GetParameter(Tag::AUDIO_OUTPUT_CHANNELS, value);
    ASSERT_TRUE(value.HasValue());

    audioCapture->GetParameter(Tag::AUDIO_SAMPLE_RATE, value);
    ASSERT_TRUE(value.HasValue());
}

HWTEST(AudioTypeTranslateTest, testSampleRateNum2Enum, TestSize.Level1)
{
    AudioSamplingRate val;
    auto res = SampleRateNum2Enum(8000, val);
    ASSERT_TRUE(val == AudioStandard::SAMPLE_RATE_8000);
    ASSERT_TRUE(res);

    auto res2 = SampleRateNum2Enum(9000, val);
    ASSERT_FALSE(res2);

    OHOS::AudioStandard::AudioSampleFormat aFmt;
    auto sampleRes = PluginFmt2SampleFmt(AudioSampleFormat::S16, aFmt);
    ASSERT_EQ(aFmt, AudioStandard::SAMPLE_S16LE);
    ASSERT_TRUE(sampleRes);

    auto sampleRes2 = PluginFmt2SampleFmt(AudioSampleFormat::S24P, aFmt);
    ASSERT_FALSE(sampleRes2);

    OHOS::AudioStandard::AudioChannel enumVal;
    auto channelRes = ChannelNumNum2Enum(1, enumVal);
    ASSERT_EQ(enumVal, AudioStandard::MONO);
    ASSERT_TRUE(channelRes);

    auto channelRes2 = ChannelNumNum2Enum(3, enumVal);
    ASSERT_FALSE(channelRes2);

    auto status = Error2Status(OHOS::ERR_OK);
    ASSERT_EQ(status, Status::OK);

    auto status2 = Error2Status(OHOS::ERR_OVERFLOW);
    ASSERT_EQ(status2, Status::ERROR_UNKNOWN);

    auto status3 = Error2Status(30);
    ASSERT_EQ(status3, Status::ERROR_UNKNOWN);
}

} // namespace Test
} // namespace Media
} // namespace OHOS
