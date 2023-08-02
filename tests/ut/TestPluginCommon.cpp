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
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/surface_allocator.h"
#include "plugin/common/surface_memory.h"
#include "plugin/common/share_memory.h"
#include "surface.h"
namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin;
using namespace testing::ext;
HWTEST(PluginBufferTest, test_PluginBuffer, TestSize.Level1) {
    std::shared_ptr<Buffer> audioBuffer = Buffer::CreateDefaultBuffer(BufferMetaType::AUDIO, 16,
                                                                      std::shared_ptr<Allocator>(), 1);
    audioBuffer->AllocMemory(nullptr, 16);
    ASSERT_TRUE(nullptr == audioBuffer->GetMemory(10));
    std::shared_ptr<Memory> memory = audioBuffer->GetMemory(0);
    uint8_t in;
    memory->Write(&in, 10, -1);
    memory->Write(&in, 10, 0);
    memory->Read(&in, 10, 0);
    ASSERT_TRUE(nullptr == memory->GetReadOnlyData(17));
    ASSERT_TRUE(nullptr == memory->GetWritableAddr(8, 9));
    memory->UpdateDataSize(8, 9);
    audioBuffer->Reset();
    audioBuffer->ChangeBufferMetaType(BufferMetaType::AUDIO);
    audioBuffer->ChangeBufferMetaType(BufferMetaType::VIDEO);
    std::shared_ptr<Buffer> videoBuffer = Buffer::CreateDefaultBuffer(BufferMetaType::VIDEO, 16,
                                                                      std::shared_ptr<Allocator>(), 1);
    videoBuffer->Reset();
}
} //namespace Test
} //namespace Media
} //namespace OHOS