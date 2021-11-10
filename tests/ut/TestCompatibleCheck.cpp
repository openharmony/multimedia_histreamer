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

TEST(TestApplyCapability, mime_Test)
{
    Capability wildcard {"*"};
    Capability audioWildcard {"audio/*"};
    Capability testWildcard {"test/*"};
    Capability wrongWildcard {"/audio*"};
    Capability wrongCapability {"wrong"};
    Capability rawMimeCapability {"audio/raw"};
    Capability mpegMimeCapability {"audio/mpeg"};

    Capability out;
    ASSERT_TRUE(Pipeline::MergeCapability(audioWildcard, wildcard, out));
    ASSERT_TRUE(out.mime == audioWildcard.mime);
    ASSERT_TRUE(out.keys.empty());

    ASSERT_FALSE(Pipeline::MergeCapability(wrongWildcard, wildcard, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    ASSERT_FALSE(Pipeline::MergeCapability(wrongCapability, wildcard, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    ASSERT_FALSE(Pipeline::MergeCapability(wrongCapability, audioWildcard, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());


    ASSERT_TRUE(Pipeline::MergeCapability(rawMimeCapability, wildcard, out));
    ASSERT_TRUE(out.mime == rawMimeCapability.mime);
    ASSERT_TRUE(out.keys.empty());

    ASSERT_TRUE(Pipeline::MergeCapability(rawMimeCapability, audioWildcard, out));
    ASSERT_TRUE(out.mime == rawMimeCapability.mime);
    ASSERT_TRUE(out.keys.empty());

    ASSERT_FALSE(Pipeline::MergeCapability(rawMimeCapability, testWildcard, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    ASSERT_FALSE(Pipeline::MergeCapability(rawMimeCapability, mpegMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    ASSERT_FALSE(Pipeline::MergeCapability(rawMimeCapability, wrongWildcard, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());
}

TEST(TestMergeCapabilityKeys, SingleType_Test)
{
    Capability wildMimeCapability("*");
    Capability out;

    Capability rawFixedMimeCapability (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000);

    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawFixedMimeCapability, wildMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    Capability rawFixedMimeCapability2 (MEDIA_MIME_AUDIO_MPEG);
    rawFixedMimeCapability2.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000);

    // fix apply with fix
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawFixedMimeCapability, rawFixedMimeCapability2, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    // apply failed
    Capability rawFixedMimeCapability3 (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability3.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 4000);
    ASSERT_FALSE(Pipeline::MergeCapabilityKeys(rawFixedMimeCapability, rawFixedMimeCapability3, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    Capability rawListMimeCapability {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {8000, 32000, 48000, 44100});

    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawListMimeCapability, wildMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    auto disCaps = Plugin::AnyCast<Plugin::DiscreteCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(disCaps[0] == 8000);
    ASSERT_TRUE(disCaps[1] == 32000);
    ASSERT_TRUE(disCaps[2] == 48000);
    ASSERT_TRUE(disCaps[3] == 44100);

    // fix apply with discrete
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawFixedMimeCapability, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    // apply failed
    Capability rawFixedMimeCapability4 (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability4.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 4000);
    ASSERT_FALSE(Pipeline::MergeCapabilityKeys(rawFixedMimeCapability4, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    // discrete apply with discrete
    Capability rawListMimeCapability2 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability2.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 48000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawListMimeCapability2, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    auto tmp1 = Plugin::AnyCast<Plugin::DiscreteCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(tmp1.size() == 2);
    ASSERT_TRUE(tmp1[0] == 48000);
    ASSERT_TRUE(tmp1[1] == 44100);

    // discrete apply with discrete
    Capability rawListMimeCapability3 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability3.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawListMimeCapability3, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    auto tmp2 = Plugin::AnyCast<Plugin::FixedCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(tmp2 == 44100);

    // discrete apply with discrete failed
    Capability rawListMimeCapability4 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability4.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 4100});
    ASSERT_FALSE(Pipeline::MergeCapabilityKeys(rawListMimeCapability4, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    Capability rawIntervalMimeCapability {MEDIA_MIME_AUDIO_RAW};
    rawIntervalMimeCapability.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000, 48000);

    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability, wildMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    auto intCaps = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps.first == 8000);
    ASSERT_TRUE(intCaps.second == 48000);

    // inter apply with fix
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawFixedMimeCapability, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability, rawFixedMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    Capability rawFixedMimeCapability5 (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability5.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 4000);
    ASSERT_FALSE(Pipeline::MergeCapabilityKeys(rawFixedMimeCapability5, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    // inter apply with inter
    Capability rawIntervalMimeCapability2 {MEDIA_MIME_AUDIO_RAW};
    rawIntervalMimeCapability2.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 3000, 9000);
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability2, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    auto intCaps2 = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps2.first == 8000);
    ASSERT_TRUE(intCaps2.second == 9000);

    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability, rawIntervalMimeCapability2, out));
    ASSERT_TRUE(out.mime.empty());
    auto intCaps3 = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps3.first == 8000);
    ASSERT_TRUE(intCaps3.second == 9000);

    Capability rawIntervalMimeCapability3 {MEDIA_MIME_AUDIO_RAW};
    rawIntervalMimeCapability3.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 3000, 4000);
    ASSERT_FALSE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability3, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    // inter apply with discrete
    Capability rawListMimeCapability5 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability5.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 4100});
    ASSERT_FALSE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability, rawListMimeCapability5, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    Capability rawListMimeCapability6 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability6.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability, rawListMimeCapability6, out));
    ASSERT_TRUE(out.mime.empty());
    auto intCaps4 = Plugin::AnyCast<Plugin::FixedCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps4 == 44100);

    Capability rawListMimeCapability7 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability7.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 40000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapabilityKeys(rawIntervalMimeCapability, rawListMimeCapability7, out));
    ASSERT_TRUE(out.mime.empty());
    auto intCaps5 = Plugin::AnyCast<Plugin::DiscreteCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps5.size() == 2);
    ASSERT_TRUE(intCaps5[0] == 40000);
    ASSERT_TRUE(intCaps5[1] == 44100);
}

TEST(TestMergeCapability, SingleType_Test)
{
    Capability wildMimeCapability("*");
    Capability out;

    Capability rawFixedMimeCapability (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000);

    ASSERT_TRUE(Pipeline::MergeCapability(rawFixedMimeCapability, wildMimeCapability, out));
    ASSERT_TRUE(out.mime == rawFixedMimeCapability.mime);
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    Capability rawFixedMimeCapability2 (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability2.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000);

    // fix apply with fix
    ASSERT_TRUE(Pipeline::MergeCapability(rawFixedMimeCapability, rawFixedMimeCapability2, out));
    ASSERT_TRUE(out.mime == rawFixedMimeCapability.mime);
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    // apply failed
    Capability rawFixedMimeCapability3 (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability3.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 4000);
    ASSERT_FALSE(Pipeline::MergeCapability(rawFixedMimeCapability, rawFixedMimeCapability3, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    Capability rawListMimeCapability {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {8000, 32000, 48000, 44100});

    ASSERT_TRUE(Pipeline::MergeCapability(rawListMimeCapability, wildMimeCapability, out));
    ASSERT_TRUE(out.mime == rawListMimeCapability.mime);
    auto disCaps = Plugin::AnyCast<Plugin::DiscreteCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(disCaps[0] == 8000);
    ASSERT_TRUE(disCaps[1] == 32000);
    ASSERT_TRUE(disCaps[2] == 48000);
    ASSERT_TRUE(disCaps[3] == 44100);

    // fix apply with discrete
    ASSERT_TRUE(Pipeline::MergeCapability(rawFixedMimeCapability, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime == rawFixedMimeCapability.mime);
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    // apply failed
    Capability rawFixedMimeCapability4 (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability4.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 4000);
    ASSERT_FALSE(Pipeline::MergeCapability(rawFixedMimeCapability4, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    // discrete apply with discrete
    Capability rawListMimeCapability2 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability2.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 48000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapability(rawListMimeCapability2, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime == rawListMimeCapability2.mime);
    auto tmp1 = Plugin::AnyCast<Plugin::DiscreteCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(tmp1.size() == 2);
    ASSERT_TRUE(tmp1[0] == 48000);
    ASSERT_TRUE(tmp1[1] == 44100);

    // discrete apply with discrete
    Capability rawListMimeCapability3 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability3.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapability(rawListMimeCapability3, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime == rawListMimeCapability3.mime);
    auto tmp2 = Plugin::AnyCast<Plugin::FixedCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(tmp2 == 44100);

    // discrete apply with discrete failed
    Capability rawListMimeCapability4 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability4.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 4100});
    ASSERT_FALSE(Pipeline::MergeCapability(rawListMimeCapability4, rawListMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    Capability rawIntervalMimeCapability {MEDIA_MIME_AUDIO_RAW};
    rawIntervalMimeCapability.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000, 48000);

    ASSERT_TRUE(Pipeline::MergeCapability(rawIntervalMimeCapability, wildMimeCapability, out));
    ASSERT_TRUE(out.mime == rawIntervalMimeCapability.mime);
    auto intCaps = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps.first == 8000);
    ASSERT_TRUE(intCaps.second == 48000);

    // inter apply with fix
    ASSERT_TRUE(Pipeline::MergeCapability(rawFixedMimeCapability, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime == rawFixedMimeCapability.mime);
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    ASSERT_TRUE(Pipeline::MergeCapability(rawIntervalMimeCapability, rawFixedMimeCapability, out));
    ASSERT_TRUE(out.mime == rawIntervalMimeCapability.mime);
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);

    Capability rawFixedMimeCapability5 (MEDIA_MIME_AUDIO_RAW);
    rawFixedMimeCapability5.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 4000);
    ASSERT_FALSE(Pipeline::MergeCapability(rawFixedMimeCapability5, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    // inter apply with inter
    Capability rawIntervalMimeCapability2 {MEDIA_MIME_AUDIO_RAW};
    rawIntervalMimeCapability2.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 3000, 9000);
    ASSERT_TRUE(Pipeline::MergeCapability(rawIntervalMimeCapability2, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime == rawIntervalMimeCapability2.mime);
    auto intCaps2 = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps2.first == 8000);
    ASSERT_TRUE(intCaps2.second == 9000);

    ASSERT_TRUE(Pipeline::MergeCapability(rawIntervalMimeCapability, rawIntervalMimeCapability2, out));
    ASSERT_TRUE(out.mime == rawIntervalMimeCapability.mime);
    auto intCaps3 = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps3.first == 8000);
    ASSERT_TRUE(intCaps3.second == 9000);

    Capability rawIntervalMimeCapability3 {MEDIA_MIME_AUDIO_RAW};
    rawIntervalMimeCapability3.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 3000, 4000);
    ASSERT_FALSE(Pipeline::MergeCapability(rawIntervalMimeCapability3, rawIntervalMimeCapability, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    // inter apply with discrete
    Capability rawListMimeCapability5 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability5.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 4100});
    ASSERT_FALSE(Pipeline::MergeCapability(rawIntervalMimeCapability, rawListMimeCapability5, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    Capability rawListMimeCapability6 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability6.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 4000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapability(rawIntervalMimeCapability, rawListMimeCapability6, out));
    ASSERT_TRUE(out.mime == rawIntervalMimeCapability.mime);
    auto intCaps4 = Plugin::AnyCast<Plugin::FixedCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps4 == 44100);

    Capability rawListMimeCapability7 {MEDIA_MIME_AUDIO_RAW};
    rawListMimeCapability7.AppendDiscreteKeys<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, {1000, 2000, 40000, 44100});
    ASSERT_TRUE(Pipeline::MergeCapability(rawIntervalMimeCapability, rawListMimeCapability7, out));
    ASSERT_TRUE(out.mime == rawIntervalMimeCapability.mime);
    auto intCaps5 = Plugin::AnyCast<Plugin::DiscreteCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(intCaps5.size() == 2);
    ASSERT_TRUE(intCaps5[0] == 40000);
    ASSERT_TRUE(intCaps5[1] == 44100);
}

TEST(TestMergeCapability, ComplexType_Test)
{
    Capability wildMimeCapability("*");
    Capability out;

    Capability cap1 (MEDIA_MIME_AUDIO_RAW);
    cap1.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000);
    cap1.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 2, 8);
    cap1.AppendDiscreteKeys<Plugin::AudioSampleFormat>(CapabilityID::AUDIO_SAMPLE_FORMAT, {
        Plugin::AudioSampleFormat::S64, Plugin::AudioSampleFormat::S64P, Plugin::AudioSampleFormat::U64,
        Plugin::AudioSampleFormat::U64P, Plugin::AudioSampleFormat::F64,
    });

    Capability cap2(MEDIA_MIME_AUDIO_APE);

    ASSERT_FALSE(Pipeline::MergeCapability(cap1, cap2, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());

    Capability cap3(MEDIA_MIME_AUDIO_RAW);
    ASSERT_TRUE(Pipeline::MergeCapability(cap1, cap3, out));
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);
    auto intCaps = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_CHANNELS]);
    ASSERT_TRUE(intCaps.first == 2);
    ASSERT_TRUE(intCaps.second == 8);
    auto disCaps = Plugin::AnyCast<Plugin::DiscreteCapability<Plugin::AudioSampleFormat>>(
            out.keys[CapabilityID::AUDIO_SAMPLE_FORMAT]);
    ASSERT_TRUE(disCaps.size() == 5);
    ASSERT_TRUE(disCaps[0] == Plugin::AudioSampleFormat::S64);
    ASSERT_TRUE(disCaps[1] == Plugin::AudioSampleFormat::S64P);
    ASSERT_TRUE(disCaps[2] == Plugin::AudioSampleFormat::U64);
    ASSERT_TRUE(disCaps[3] == Plugin::AudioSampleFormat::U64P);
    ASSERT_TRUE(disCaps[4] == Plugin::AudioSampleFormat::F64);

    Capability cap4(MEDIA_MIME_AUDIO_RAW);
    cap4.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000, 96000);
    cap4.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 4);
    cap4.AppendDiscreteKeys<Plugin::AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT, {
            Plugin::AudioChannelLayout::STEREO, Plugin::AudioChannelLayout::SURROUND,
            Plugin::AudioChannelLayout::CH_5POINT1, Plugin::AudioChannelLayout::CH_7POINT1,
    });
    ASSERT_TRUE(Pipeline::MergeCapability(cap1, cap4, out));
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_CHANNELS]) == 4);
    auto disCaps1 = Plugin::AnyCast<Plugin::DiscreteCapability<Plugin::AudioSampleFormat>>(
            out.keys[CapabilityID::AUDIO_SAMPLE_FORMAT]);
    ASSERT_TRUE(disCaps.size() == 5);
    ASSERT_TRUE(disCaps[0] == Plugin::AudioSampleFormat::S64);
    ASSERT_TRUE(disCaps[1] == Plugin::AudioSampleFormat::S64P);
    ASSERT_TRUE(disCaps[2] == Plugin::AudioSampleFormat::U64);
    ASSERT_TRUE(disCaps[3] == Plugin::AudioSampleFormat::U64P);
    ASSERT_TRUE(disCaps[4] == Plugin::AudioSampleFormat::F64);
    auto intCaps1 = Plugin::AnyCast<Plugin::DiscreteCapability<Plugin::AudioChannelLayout>>(
            out.keys[CapabilityID::AUDIO_CHANNEL_LAYOUT]);
    ASSERT_TRUE(intCaps1.size() == 4);
    ASSERT_TRUE(intCaps1[0] == Plugin::AudioChannelLayout::STEREO);
    ASSERT_TRUE(intCaps1[1] == Plugin::AudioChannelLayout::SURROUND);
    ASSERT_TRUE(intCaps1[2] == Plugin::AudioChannelLayout::CH_5POINT1);
    ASSERT_TRUE(intCaps1[3] == Plugin::AudioChannelLayout::CH_7POINT1);

    Capability cap5(MEDIA_MIME_AUDIO_RAW);
    cap5.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000, 96000);
    cap5.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 10);
    cap5.AppendDiscreteKeys<Plugin::AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT, {
            Plugin::AudioChannelLayout::STEREO, Plugin::AudioChannelLayout::SURROUND,
            Plugin::AudioChannelLayout::CH_5POINT1, Plugin::AudioChannelLayout::CH_7POINT1,
    });
    ASSERT_FALSE(Pipeline::MergeCapability(cap1, cap5, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());
}

TEST(TestApplyCapabilitySet, ComplexType_Test)
{
    Capability out;

    Capability cap1 (MEDIA_MIME_AUDIO_RAW);
    cap1.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000);
    cap1.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 2, 8);
    cap1.AppendDiscreteKeys<Plugin::AudioSampleFormat>(CapabilityID::AUDIO_SAMPLE_FORMAT, {
            Plugin::AudioSampleFormat::S64, Plugin::AudioSampleFormat::S64P, Plugin::AudioSampleFormat::U64,
            Plugin::AudioSampleFormat::U64P, Plugin::AudioSampleFormat::F64,
    });

    Capability cap2(MEDIA_MIME_AUDIO_APE);
    Capability cap3(MEDIA_MIME_AUDIO_RAW);

    Capability cap4(MEDIA_MIME_AUDIO_RAW);
    cap4.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000, 96000);
    cap4.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 4);
    cap4.AppendDiscreteKeys<Plugin::AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT, {
            Plugin::AudioChannelLayout::STEREO, Plugin::AudioChannelLayout::SURROUND,
            Plugin::AudioChannelLayout::CH_5POINT1, Plugin::AudioChannelLayout::CH_7POINT1,
    });

    Capability cap5(MEDIA_MIME_AUDIO_RAW);
    cap5.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 8000, 96000);
    cap5.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 10);
    cap5.AppendDiscreteKeys<Plugin::AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT, {
            Plugin::AudioChannelLayout::STEREO, Plugin::AudioChannelLayout::SURROUND,
            Plugin::AudioChannelLayout::CH_5POINT1, Plugin::AudioChannelLayout::CH_7POINT1,
    });

    CapabilitySet capSet1 = {cap2, cap3};
    ASSERT_TRUE(Pipeline::ApplyCapabilitySet(cap1, capSet1, out));
    ASSERT_TRUE(Plugin::AnyCast<uint32_t>(out.keys[CapabilityID::AUDIO_SAMPLE_RATE]) == 8000);
    auto intCaps = Plugin::AnyCast<Plugin::IntervalCapability<uint32_t>>(out.keys[CapabilityID::AUDIO_CHANNELS]);
    ASSERT_TRUE(intCaps.first == 2);
    ASSERT_TRUE(intCaps.second == 8);
    auto disCaps = Plugin::AnyCast<Plugin::DiscreteCapability<Plugin::AudioSampleFormat>>(
            out.keys[CapabilityID::AUDIO_SAMPLE_FORMAT]);
    ASSERT_TRUE(disCaps.size() == 5);
    ASSERT_TRUE(disCaps[0] == Plugin::AudioSampleFormat::S64);
    ASSERT_TRUE(disCaps[1] == Plugin::AudioSampleFormat::S64P);
    ASSERT_TRUE(disCaps[2] == Plugin::AudioSampleFormat::U64);
    ASSERT_TRUE(disCaps[3] == Plugin::AudioSampleFormat::U64P);
    ASSERT_TRUE(disCaps[4] == Plugin::AudioSampleFormat::F64);

    CapabilitySet capSet2 = {cap2, cap5};
    ASSERT_FALSE(Pipeline::ApplyCapabilitySet(cap1, capSet2, out));
    ASSERT_TRUE(out.mime.empty());
    ASSERT_TRUE(out.keys.empty());
}

TEST(TestMetaToCap, MetaToCap_Test)
{
    Meta meta;
    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_RAW);
    meta.SetUint32(MetaID::AUDIO_MPEG_VERSION, 1);
    meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO);
    meta.SetUint32(MetaID::AUDIO_CHANNELS, 2);
    meta.SetUint32(MetaID::AUDIO_SAMPLE_RATE, 48000);
    auto cap = Pipeline::MetaToCapability(meta);
    ASSERT_STREQ(MEDIA_MIME_AUDIO_RAW, cap->mime.c_str());
    auto mpegVersion = Plugin::AnyCast<uint32_t>(cap->keys[CapabilityID::AUDIO_MPEG_VERSION]);
    ASSERT_TRUE(mpegVersion == 1);

    auto channelLayout = Plugin::AnyCast<AudioChannelLayout>(cap->keys[CapabilityID::AUDIO_CHANNEL_LAYOUT]);
    ASSERT_TRUE(channelLayout == AudioChannelLayout::STEREO);

    auto channels = Plugin::AnyCast<uint32_t>(cap->keys[CapabilityID::AUDIO_CHANNELS]);
    ASSERT_TRUE(channels == 2);

    auto sampleRate = Plugin::AnyCast<uint32_t>(cap->keys[CapabilityID::AUDIO_SAMPLE_RATE]);
    ASSERT_TRUE(sampleRate == 48000);
}

TEST(TestMergeMetaWithCapability, MergeMetaWithEmptyKeyCapability_Test)
{
    Meta meta;
    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_MPEG);
    meta.SetUint32(MetaID::AUDIO_MPEG_VERSION, 1);
    meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO);
    meta.SetUint32(MetaID::AUDIO_CHANNELS, 2);
    meta.SetUint32(MetaID::AUDIO_SAMPLE_RATE, 48000);
    meta.SetData<AudioSampleFormat>(MetaID::AUDIO_SAMPLE_FORMAT, AudioSampleFormat::U16P);

    Capability cap0(MEDIA_MIME_AUDIO_RAW);
    Meta out1;
    std::string outMime1;
    uint32_t outMpegVersion1 = 0;
    AudioChannelLayout outAudioChannelLayout1;
    uint32_t outChannels1 = 0;
    uint32_t outSampleRate1 = 0;
    AudioSampleFormat outSampleFormat1 = AudioSampleFormat::U8;
    ASSERT_TRUE(Pipeline::MergeMetaWithCapability(meta, cap0, out1));
    ASSERT_TRUE(out1.GetString(MetaID::MIME, outMime1));
    ASSERT_STREQ(outMime1.c_str(), MEDIA_MIME_AUDIO_RAW);
    ASSERT_TRUE(out1.GetUint32(MetaID::AUDIO_MPEG_VERSION, outMpegVersion1));
    ASSERT_TRUE(outMpegVersion1 == 1);
    ASSERT_TRUE(out1.GetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, outAudioChannelLayout1));
    ASSERT_TRUE(outAudioChannelLayout1 == AudioChannelLayout::STEREO);
    ASSERT_TRUE(out1.GetUint32(MetaID::AUDIO_CHANNELS, outChannels1));
    ASSERT_TRUE(outChannels1 == 2);
    ASSERT_TRUE(out1.GetUint32(MetaID::AUDIO_SAMPLE_RATE, outSampleRate1));
    ASSERT_TRUE(outSampleRate1 == 48000);
    ASSERT_TRUE(out1.GetData(MetaID::AUDIO_SAMPLE_FORMAT, outSampleFormat1));
    ASSERT_TRUE(outSampleFormat1 == AudioSampleFormat::U16P);
}

TEST(TestMergeMetaWithCapability, Merge_meta_contains_meta_ony_key_capability_Test)
{
    Meta meta;
    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_MPEG);
    meta.SetUint32(MetaID::AUDIO_MPEG_VERSION, 1);
    meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO);
    meta.SetUint32(MetaID::AUDIO_CHANNELS, 2);
    meta.SetUint32(MetaID::AUDIO_SAMPLE_RATE, 48000);
    meta.SetInt64(MetaID::MEDIA_BITRATE, 128000);

    Capability cap0(MEDIA_MIME_AUDIO_RAW);
    cap0.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_MPEG_VERSION, 1);
    cap0.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_SAMPLE_RATE, 48000);
    cap0.AppendDiscreteKeys<AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT,
                                                {AudioChannelLayout::STEREO,AudioChannelLayout::SURROUND});
    cap0.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 1, 8);
    cap0.AppendDiscreteKeys<AudioSampleFormat>(CapabilityID::AUDIO_SAMPLE_FORMAT,
                                               {AudioSampleFormat::U16P, AudioSampleFormat::U8});
    cap0.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_MPEG_LAYER, 3, 7);

    Meta out1;
    std::string outMime1;
    uint32_t outMpegVersion1 = 0;
    AudioChannelLayout outAudioChannelLayout1;
    uint32_t outChannels1 = 0;
    uint32_t outSampleRate1 = 0;
    AudioSampleFormat outSampleFormat1 = AudioSampleFormat::U8;
    uint32_t outMpegLayer = 0;
    int64_t  outBitRate = 0;

    ASSERT_TRUE(Pipeline::MergeMetaWithCapability(meta, cap0, out1));
    ASSERT_TRUE(out1.GetString(MetaID::MIME, outMime1));
    ASSERT_STREQ(outMime1.c_str(), MEDIA_MIME_AUDIO_RAW);
    ASSERT_TRUE(out1.GetUint32(MetaID::AUDIO_MPEG_VERSION, outMpegVersion1));
    ASSERT_TRUE(outMpegVersion1 == 1);
    ASSERT_TRUE(out1.GetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, outAudioChannelLayout1));
    ASSERT_TRUE(outAudioChannelLayout1 == AudioChannelLayout::STEREO);
    ASSERT_TRUE(out1.GetUint32(MetaID::AUDIO_CHANNELS, outChannels1));
    ASSERT_TRUE(outChannels1 == 2);
    ASSERT_TRUE(out1.GetUint32(MetaID::AUDIO_SAMPLE_RATE, outSampleRate1));
    ASSERT_TRUE(outSampleRate1 == 48000);
    ASSERT_TRUE(out1.GetData(MetaID::AUDIO_SAMPLE_FORMAT, outSampleFormat1));
    ASSERT_TRUE(outSampleFormat1 == AudioSampleFormat::U16P);
    ASSERT_TRUE(out1.GetUint32(MetaID::AUDIO_MPEG_LAYER, outMpegLayer));
    ASSERT_TRUE(outMpegLayer == 3);
    ASSERT_TRUE(out1.GetInt64(MetaID::MEDIA_BITRATE, outBitRate));
    ASSERT_TRUE(outBitRate == 128000);
}

TEST(TestMergeMetaWithCapability, Merge_meta_with_capability_failed_Test)
{
    Meta meta;
    meta.SetString(MetaID::MIME, MEDIA_MIME_AUDIO_MPEG);
    meta.SetUint32(MetaID::AUDIO_MPEG_VERSION, 1);
    meta.SetData<AudioChannelLayout>(MetaID::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout::STEREO);
    meta.SetUint32(MetaID::AUDIO_CHANNELS, 2);
    meta.SetUint32(MetaID::AUDIO_SAMPLE_RATE, 48000);
    meta.SetInt64(MetaID::MEDIA_BITRATE, 128000);

    Capability cap0(MEDIA_MIME_AUDIO_RAW);
    cap0.AppendFixedKey<uint32_t>(CapabilityID::AUDIO_MPEG_VERSION, 2);
    Meta out1;
    ASSERT_FALSE(Pipeline::MergeMetaWithCapability(meta, cap0, out1));

    Capability cap1(MEDIA_MIME_AUDIO_RAW);
    cap1.AppendDiscreteKeys<AudioChannelLayout>(CapabilityID::AUDIO_CHANNEL_LAYOUT,
                                                {AudioChannelLayout::CH_5POINT1,AudioChannelLayout::SURROUND});
    Meta out2;
    ASSERT_FALSE(Pipeline::MergeMetaWithCapability(meta, cap1, out2));


    Capability cap2(MEDIA_MIME_AUDIO_RAW);
    cap2.AppendIntervalKey<uint32_t>(CapabilityID::AUDIO_CHANNELS, 3, 8);
    Meta out3;
    ASSERT_FALSE(Pipeline::MergeMetaWithCapability(meta, cap2, out3));
}
}