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

#include <memory>
#include <string>
#include "gtest/gtest.h"
#include "plugin/plugins/source/file_source/file_source_plugin.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;
using namespace testing::ext;

class TestFileSourcePlugin : public ::testing::Test {
public:
    std::shared_ptr<FileSource::FileSourcePlugin> fileSourcePlugin;
    void SetUp() override
    {
        fileSourcePlugin = std::make_shared<FileSource::FileSourcePlugin>("test");
    }

    void TearDown() override
    {
    }
};

HWTEST_F(TestFileSourcePlugin, test_init, TestSize.Level1)
{
    auto status = fileSourcePlugin->Init();
    EXPECT_EQ(Status::OK, status);
}

HWTEST_F(TestFileSourcePlugin, test_prepare, TestSize.Level1)
{
    auto status = fileSourcePlugin->Prepare();
    EXPECT_EQ(Status::OK, status);
}

HWTEST_F(TestFileSourcePlugin, test_reset, TestSize.Level1)
{
    auto status = fileSourcePlugin->Reset();
    EXPECT_EQ(Status::OK, status);
}

HWTEST_F(TestFileSourcePlugin, test_set_callback, TestSize.Level1)
{
    Callback* cb  = nullptr;
    auto status = fileSourcePlugin->SetCallback(cb);
    EXPECT_EQ(Status::ERROR_UNIMPLEMENTED, status);
    delete cb;
}

HWTEST_F(TestFileSourcePlugin, test_get_size, TestSize.Level1)
{
    uint64_t size = 10;
    auto status = fileSourcePlugin->GetSize(size);
    EXPECT_EQ(Status::ERROR_WRONG_STATE, status);
}

HWTEST_F(TestFileSourcePlugin, test_seek_to, TestSize.Level1)
{
    auto status = fileSourcePlugin->SeekToPos(10);
    EXPECT_EQ(Status::ERROR_WRONG_STATE, status);
}

HWTEST_F(TestFileSourcePlugin, test_set_get_parameter, TestSize.Level1)
{
    Tag tag = Tag::SECTION_VIDEO_UNIVERSAL_START;
    ValueType value = 2;
    auto status = fileSourcePlugin->GetParameter(tag, value);
    EXPECT_EQ(Status::ERROR_UNIMPLEMENTED, status);
    status = fileSourcePlugin->SetParameter(tag, value);
    EXPECT_EQ(Status::ERROR_UNIMPLEMENTED, status);
}

HWTEST_F(TestFileSourcePlugin, test_deinit_status, TestSize.Level1)
{
    auto status = fileSourcePlugin->Deinit();
    EXPECT_EQ(Status::OK, status);
}
} // namespace Test
} // namespace Media
} // namespace OHOS
