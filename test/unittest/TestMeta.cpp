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

#include <gtest/gtest.h>
#include "plugin/common/plugin_caps.h"
#include "pipeline/core/compatible_check.h"
#include "pipeline/core/type_define.h"
#include "plugin_utils.h"
#include "plugin/common/plugin_meta.h"

#define private public
#define protected public
#define UNIT_TEST 1


using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Pipeline;

HWTEST(TestMeta, can_get_uint32_after_set, TestSize.Level1)
{
    Meta meta;
    uint32_t channels = 64;
    meta.Set<Tag::AUDIO_CHANNELS>(channels);
    uint32_t outChannels = 0;
    ASSERT_TRUE(meta.Get<Tag::AUDIO_CHANNELS>(outChannels));
    ASSERT_EQ(channels, outChannels);
}

HWTEST(TestMeta, can_not_get_uint32_if_not_set, TestSize.Level1)
{
    Meta meta;
    uint32_t canNotGet = 0;
    ASSERT_FALSE(meta.Get<Tag::AUDIO_CHANNELS>(canNotGet));
}

HWTEST(TestMeta, set_then_get_cstring, TestSize.Level1)
{
    Meta meta;
    std::string artist("abcd");
    meta.Set<Tag::MEDIA_TITLE>(artist);
    std::string outArtist;
    ASSERT_TRUE(meta.Get<Tag::MEDIA_TITLE>(outArtist));
    ASSERT_STREQ(artist.c_str(), outArtist.c_str());
}

HWTEST(TestMeta, set_then_get_float, TestSize.Level1)
{
    Meta meta;
    std::string in = "9.9999f";
    meta.Set<Tag::MEDIA_ARTIST>(in); // this is only for test, normally MEDIA_ARTIST should be string
    std::string out = "";
    ASSERT_TRUE(meta.Get<Tag::MEDIA_ARTIST>(out));
    ASSERT_TRUE(in == out);
}

HWTEST(TestMeta, fail_to_get_unexisted_data, TestSize.Level1)
{
    Meta meta;
    int32_t channels = 64;
    meta.Set<Tag::AUDIO_CHANNELS>(channels);
    int64_t bitRate = 1876411;
    ASSERT_FALSE(meta.Get<Tag::MEDIA_BITRATE>(bitRate));
}

HWTEST(TestMeta, remove_data, TestSize.Level1)
{
    Meta meta;
    uint32_t channels = 64;
    meta.Set<Tag::AUDIO_CHANNELS>(channels);
    uint32_t out;
    ASSERT_TRUE(meta.Get<Tag::AUDIO_CHANNELS>(out));
    ASSERT_EQ(channels, out);
    meta.Remove(Tag::AUDIO_CHANNELS);
    ASSERT_FALSE(meta.Get<Tag::AUDIO_CHANNELS>(out));
}

HWTEST(TestMeta, clear_data, TestSize.Level1)
{
    Meta meta;
    std::string title("title");
    std::string album("album");
    uint32_t channels = 64;
    meta.Set<Tag::MEDIA_TITLE>(title);
    meta.Set<Tag::MEDIA_ALBUM>(album);
    meta.Set<Tag::AUDIO_CHANNELS>(channels);
    meta.Clear();
    std::string out;
    ASSERT_FALSE(meta.Get<Tag::MEDIA_TITLE>(out));
    ASSERT_TRUE(out.empty());
    ASSERT_FALSE(meta.Get<Tag::MEDIA_ALBUM>(out));
    ASSERT_TRUE(out.empty());
    uint32_t oChannels = 0;
    ASSERT_FALSE(meta.Get<Tag::AUDIO_CHANNELS>(oChannels));
    ASSERT_EQ(0u, oChannels);
}

HWTEST(TestMeta, update_meta, TestSize.Level1)
{
    Meta meta;
    std::string title("title");
    std::string album("album");
    int32_t channels = 64;
    meta.Set<Tag::MEDIA_TITLE>(title);
    meta.Set<Tag::MEDIA_ALBUM>(album);
    meta.Set<Tag::AUDIO_CHANNELS>(channels);

    Meta meta2;
    uint32_t channels2 = 32;
    meta2.Set<Tag::AUDIO_CHANNELS>(channels2);

    meta = meta2;
    std::string out;
    ASSERT_FALSE(meta.Get<Tag::MEDIA_TITLE>(out));
    ASSERT_STRNE("title", out.c_str());
    ASSERT_FALSE(meta.Get<Tag::MEDIA_ALBUM>(out));
    ASSERT_STRNE("album", out.c_str());
    uint32_t oChannel = 0;
    ASSERT_TRUE(meta.Get<Tag::AUDIO_CHANNELS>(oChannel));
    ASSERT_EQ(channels2, oChannel);
}

HWTEST(TestMeta, Can_insert_tag_value_int64_to_Meta, TestSize.Level1)
{
    Meta meta;
    ASSERT_TRUE(meta.Set<Tag::MEDIA_DURATION>(10000));
    ASSERT_TRUE(meta.Set<Tag::MEDIA_FILE_SIZE>(500));
    int64_t value;
    ASSERT_TRUE(meta.Get<Tag::MEDIA_DURATION>(value));
    ASSERT_EQ(10000, value);
    uint64_t size;
    ASSERT_TRUE(meta.Get<Tag::MEDIA_FILE_SIZE>(size));
    ASSERT_EQ(500, size);
}

HWTEST(TestMeta, Can_insert_tag_value_uint32_to_Meta, TestSize.Level1)
{
    Meta meta;
    ASSERT_TRUE(meta.Set<Tag::TRACK_ID>(10000));
    uint32_t value;
    ASSERT_TRUE(meta.Get<Tag::TRACK_ID>(value));
    ASSERT_EQ(10000, value);
}

HWTEST(TestMeta, return_value_after_meta_to_capability, TestSize.Level1)
{
    Meta meta;
    meta.Set<Plugin::Tag::AUDIO_MPEG_VERSION>(1);
    meta.Set<Plugin::Tag::AUDIO_CHANNELS>(2);
    std::shared_ptr<Capability> cap = MetaToCapability(meta);
    auto mpegVersion = Plugin::AnyCast<uint32_t>(cap->keys[CapabilityID::AUDIO_MPEG_VERSION]);
    ASSERT_TRUE(mpegVersion == 1);
    auto channels = Plugin::AnyCast<uint32_t>(cap->keys[CapabilityID::AUDIO_CHANNELS]);
    ASSERT_TRUE(channels == 2);
}

HWTEST(TestMeta, meta_to_string, TestSize.Level1)
{
    Meta meta;
    meta.Set<Plugin::Tag::AUDIO_MPEG_VERSION>(1);
    meta.Set<Plugin::Tag::AUDIO_CHANNELS>(2);
    std::string string = Meta2String(meta);
    ASSERT_EQ(string, "Meta{channels:(uint32_t)2, ad_mpeg_ver:(uint32_t)1}");
}
} // namespace Test
} // namespace Media
} // namespace OHOS
