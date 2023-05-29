/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <memory>
#include <string>
#include "gtest/gtest.h"
#define private public
#define protected public
#include "plugin/common/plugin_meta.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;

const int64_t TEST_TRACK_ID = 10000;

const int64_t TEST_MEDIA_DURATION = 10000;

const int64_t TEST_FILE_SIZE = 500;

class TestMeta : public ::testing::Test {
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(TestMeta, Can_insert_tag_value_int64_to_Meta)
{
    Meta map;
    ASSERT_TRUE(map.Set<Tag::MEDIA_DURATION>(TEST_MEDIA_DURATION));
    ASSERT_TRUE(map.Set<Tag::MEDIA_FILE_SIZE>(TEST_FILE_SIZE));
    int64_t value;
    ASSERT_TRUE(map.Get<Tag::MEDIA_DURATION>(value));
    ASSERT_EQ(TEST_MEDIA_DURATION, value);
    uint64_t size;
    ASSERT_TRUE(map.Get<Tag::MEDIA_FILE_SIZE>(size));
    ASSERT_EQ(TEST_FILE_SIZE, value);
}

TEST_F(TestMeta, Can_insert_tag_value_uint32_to_TagMap)
{
    Meta map;
    ASSERT_TRUE(map.Set<Tag::TRACK_ID>(TEST_TRACK_ID));
    uint32_t value;
    ASSERT_TRUE(map.Get<Tag::TRACK_ID>(value));
    ASSERT_EQ(TEST_TRACK_ID, value);
}
} // namespace Test
} // namespace Media
} // namespace OHOS