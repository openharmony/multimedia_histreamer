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
#include "filters/demux/data_packer.h"
#include "type_define.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;

class DataPackerTest: public :: testing::Test {
public:
    std::shared_ptr<DataPacker> dataPacker;
    void SetUp() override
    {
        dataPacker = std::make_shared<DataPacker>();
    }

    void TearDown() override
    {
    }
};

AVBufferPtr CreateBuffer(size_t size)
{
    auto buffer = std::make_shared<AVBuffer>();
    buffer->AllocMemory(nullptr, size);
    buffer->GetMemory()->Write((uint8_t*)"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", size);
    return buffer;
}

AVBufferPtr CreateEmptyBuffer(size_t size)
{
    auto buffer = std::make_shared<AVBuffer>();
    buffer->AllocMemory(nullptr, size);
    buffer->GetMemory()->Write((uint8_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", size);
    return buffer;
}

TEST_F(DataPackerTest, get_data_in_the_middle_of_one_buffer)
{
    auto bufferPtr = CreateBuffer(10);
    dataPacker->PushData(bufferPtr, 0);
    ASSERT_STREQ("DataPacker (offset 0, size 10, buffer count 1)", dataPacker->ToString().c_str());
    uint64_t curOffset = 0;
    ASSERT_TRUE(dataPacker->IsDataAvailable(3, 2, curOffset));
    ASSERT_EQ(curOffset, 10);
    auto bufferOut = CreateEmptyBuffer(3);
    dataPacker->GetRange(3, 2, bufferOut);
    ASSERT_STREQ("45", (const char *)(bufferOut->GetMemory()->GetReadOnlyData()));
}

} // namespace Test
} // namespace Media
} // namespace OHOS