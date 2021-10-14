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

#include "common/any.h"

#include <gtest/gtest.h>

#define private public
#define protected public
#define UNIT_TEST 1

#include "core/compatible_check.h"
#include "utils/constants.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/core/plugin_meta.h"

using namespace std;
using namespace OHOS::Media::Plugin;

namespace OHOS::Media::Test {
TEST(TestMime, Mime_compatible) {
    Capability wildcard {"*"};
    Capability audioWildcard {"audio/*"};
    Capability testWildcard {"test/*"};
    Capability wrongWildcard {"/audio*"};
    Capability wrongCapability {"wrong"};
    Capability rawMimeCapability {"audio/raw"};
    Capability mpegMimeCapability {"audio/mpeg"};
    Meta meta;
    ASSERT_FALSE(Pipeline::CompatibleWith(wildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(audioWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(testWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(rawMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(mpegMimeCapability, meta));
    meta.SetString(MetaID::MIME, "/TEST");
    ASSERT_FALSE(Pipeline::CompatibleWith(wildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(audioWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(testWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(rawMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(mpegMimeCapability, meta));

    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_RAW);
    ASSERT_TRUE(Pipeline::CompatibleWith(wildcard, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(audioWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(testWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongCapability, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(rawMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(mpegMimeCapability, meta));

    meta.SetString(MetaID::MIME, "AUDIO/RAW");
    ASSERT_TRUE(Pipeline::CompatibleWith(wildcard, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(audioWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(testWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongWildcard, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(wrongCapability, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(rawMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(mpegMimeCapability, meta));
}

TEST(TestAudioSampleRate, AudioSampleRate_compatible) {
    Capability rawFixedMimeCapability (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000);
    Capability rawListMimeCapability {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {8000, 32000, 48000, 44100});
    Capability rawIntervalMimeCapability {MEDIA_MIME_AUDIO_RAW};
    rawIntervalMimeCapability.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000, 48000);


    Meta meta;
    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_RAW);
    ASSERT_FALSE(Pipeline::CompatibleWith(rawFixedMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(rawListMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(rawIntervalMimeCapability, meta));
    meta.SetUint32(MetaID::AUDIO_SAMPLE_RATE, 8000);
    ASSERT_TRUE(Pipeline::CompatibleWith(rawFixedMimeCapability, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(rawListMimeCapability, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(rawIntervalMimeCapability, meta));
    meta.SetUint32(MetaID::AUDIO_SAMPLE_RATE, 80000);
    ASSERT_FALSE(Pipeline::CompatibleWith(rawFixedMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(rawListMimeCapability, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(rawIntervalMimeCapability, meta));
}

TEST(TestAudioChannelMask, AudioChannelLayout_compatible) {
        Capability rawFixedMimeCapability {MEDIA_MIME_AUDIO_RAW};
        rawFixedMimeCapability.AppendFixedKey<AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT,
                                                                  AudioChannelLayout::STEREO);

        Capability rawListMimeCapability {MEDIA_MIME_AUDIO_RAW};
        rawListMimeCapability.AppendDiscreteKeys<AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT,
                {AudioChannelLayout::STEREO, AudioChannelLayout::SURROUND,
                 AudioChannelLayout::CH_5POINT1, AudioChannelLayout::CH_7POINT1});

        Capability illFormat {MEDIA_MIME_AUDIO_RAW};
        // channel layout does not support format [a, b]
        illFormat.AppendIntervalKey<AudioChannelLayout>(
                CapabilityID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO,
                AudioChannelLayout::SURROUND);


        Meta meta;
        meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_RAW);
        ASSERT_FALSE(Pipeline::CompatibleWith(rawFixedMimeCapability, meta));
        ASSERT_FALSE(Pipeline::CompatibleWith(rawListMimeCapability, meta));
        ASSERT_FALSE(Pipeline::CompatibleWith(illFormat, meta));
        meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO);
        ASSERT_TRUE(Pipeline::CompatibleWith(rawFixedMimeCapability, meta));
        ASSERT_TRUE(Pipeline::CompatibleWith(rawListMimeCapability, meta));
        ASSERT_FALSE(Pipeline::CompatibleWith(illFormat, meta));
        meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::CH_2_1);
        ASSERT_FALSE(Pipeline::CompatibleWith(rawFixedMimeCapability, meta));
        ASSERT_FALSE(Pipeline::CompatibleWith(rawListMimeCapability, meta));
        ASSERT_FALSE(Pipeline::CompatibleWith(illFormat, meta));
}

TEST(TestCapabilityListCompatible, CapabilityList_compatible) {
    CapabilitySet cs0 {Capability(MEDIA_MIME_AUDIO_RAW), Capability(MEDIA_MIME_AUDIO_AAC),
                       Capability(MEDIA_MIME_AUDIO_APE)};
    Capability ca0;
    ca0.SetMime(MEDIA_MIME_AUDIO_RAW).AppendFixedKey<uint32_t>(CapabilityID::AUDIO_MPEG_VERSION, 1);
    Capability ca1;
    ca1.SetMime(MEDIA_MIME_AUDIO_RAW)
        .AppendDiscreteKeys<AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT,
            {AudioChannelLayout::STEREO, AudioChannelLayout::SURROUND,
             AudioChannelLayout::CH_5POINT1, AudioChannelLayout::CH_7POINT1})
        .AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 2, 10)
        .AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 44100)
        .AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_MPEG_VERSION, 100, 1000);
    Capability ca2;
    ca2.SetMime(MEDIA_MIME_AUDIO_RAW)
        .AppendDiscreteKeys<AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT,
                {AudioChannelLayout::STEREO, AudioChannelLayout::SURROUND})
        .AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 2, 5)
        .AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 48000)
        .AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_MPEG_VERSION, 3000, 10000);

    Capability ca3;
    ca3.SetMime(MEDIA_MIME_AUDIO_FLAC)
        .AppendDiscreteKeys<AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT,
                {AudioChannelLayout::STEREO, AudioChannelLayout::SURROUND})
        .AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 2, 5)
        .AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 48000)
        .AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_MPEG_VERSION, 3000, 10000);

    CapabilitySet cs1{ca0, ca1, ca2, ca3};

    Meta meta;
    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_RAW);
    meta.SetUint32(MetaID::AUDIO_MPEG_VERSION, 1);
    ASSERT_TRUE(Pipeline::CompatibleWith(cs0, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(ca0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca1, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca2, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca3, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(cs1, meta));

    meta.SetUint32(MetaID::AUDIO_MPEG_VERSION, 300);
    meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO);
    meta.SetUint32(MetaID::AUDIO_CHANNELS, 2);
    meta.SetUint32(MetaID::AUDIO_SAMPLE_RATE, 48000);
    ASSERT_TRUE(Pipeline::CompatibleWith(cs0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca1, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca2, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca3, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(cs1, meta));

    meta.SetUint32(MetaID::AUDIO_MPEG_VERSION, 3000);
    ASSERT_TRUE(Pipeline::CompatibleWith(cs0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca1, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(ca2, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca3, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(cs1, meta));

    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_FLAC);
    ASSERT_FALSE(Pipeline::CompatibleWith(cs0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca1, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca2, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(ca3, meta));
    ASSERT_TRUE(Pipeline::CompatibleWith(cs1, meta));

    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_APE);
    meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::CH_2_1);
    ASSERT_TRUE(Pipeline::CompatibleWith(cs0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca0, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca1, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca2, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(ca3, meta));
    ASSERT_FALSE(Pipeline::CompatibleWith(cs1, meta));
}
}