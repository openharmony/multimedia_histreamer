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
#include <pipeline/core/event.h>
#include <pipeline/core/type_define.h>
#include "pipeline/filters/common/plugin_utils.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Pipeline;
using namespace testing::ext;

HWTEST(TestMeta, get_byte_per_sample, TestSize.Level1)
{
    uint8_t bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S64);
    ASSERT_EQ(bytes, 8);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S64P);
    ASSERT_EQ(bytes, 8);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U64);
    ASSERT_EQ(bytes, 8);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U64P);
    ASSERT_EQ(bytes, 8);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::F64);
    ASSERT_EQ(bytes, 8);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::F64P);
    ASSERT_EQ(bytes, 8);

    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::F32);
    ASSERT_EQ(bytes, 4);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::F32P);
    ASSERT_EQ(bytes, 4);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S32);
    ASSERT_EQ(bytes, 4);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S32P);
    ASSERT_EQ(bytes, 4);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U32);
    ASSERT_EQ(bytes, 4);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U32P);
    ASSERT_EQ(bytes, 4);

    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S24);
    ASSERT_EQ(bytes, 3);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S24P);
    ASSERT_EQ(bytes, 3);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U24);
    ASSERT_EQ(bytes, 3);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U24P);
    ASSERT_EQ(bytes, 3);

    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S16);
    ASSERT_EQ(bytes, 2);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S16P);
    ASSERT_EQ(bytes, 2);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U16);
    ASSERT_EQ(bytes, 2);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U16P);
    ASSERT_EQ(bytes, 2);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S8);
    ASSERT_EQ(bytes, 1);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::S8P);
    ASSERT_EQ(bytes, 1);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U8);
    ASSERT_EQ(bytes, 1);
    bytes = GetBytesPerSample(Plugin::AudioSampleFormat::U8P);
    ASSERT_EQ(bytes, 1);
}

HWTEST(TestMeta, return_type_if_type_is_existed, TestSize.Level1)
{
    EventType eventType = EventType::EVENT_READY;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_READY");
    eventType = EventType::EVENT_AUDIO_PROGRESS;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_AUDIO_PROGRESS");
    eventType = EventType::EVENT_VIDEO_PROGRESS;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_VIDEO_PROGRESS");
    eventType = EventType::EVENT_COMPLETE;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_COMPLETE");
    eventType = EventType::EVENT_ERROR;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_ERROR");
    eventType = EventType::EVENT_PLUGIN_ERROR;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_PLUGIN_ERROR");
    eventType = EventType::EVENT_PLUGIN_EVENT;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_PLUGIN_EVENT");
    eventType = EventType::EVENT_BUFFERING;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_BUFFERING");
    eventType = EventType::EVENT_BUFFER_PROGRESS;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_BUFFER_PROGRESS");
    eventType = EventType::EVENT_DECODER_ERROR;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_DECODER_ERROR");
    eventType = EventType::EVENT_RESOLUTION_CHANGE;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_RESOLUTION_CHANGE");
    eventType = EventType::EVENT_VIDEO_RENDERING_START;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_VIDEO_RENDERING_START");
    eventType = EventType::EVENT_IS_LIVE_STREAM;
    ASSERT_STREQ(GetEventName(eventType), "EVENT_IS_LIVE_STREAM");
}
} // namespace Test
} // namespace Media
} // namespace OHOS
