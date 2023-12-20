/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "avbuffer_inner_mock.h"
#include "buffer/avbuffer.h"
#include "buffer/avbuffer_common.h"
#include "unittest_log.h"

namespace OHOS {
namespace Media {
std::shared_ptr<AVBufferMock> AVBufferMockFactory::CreateAVBuffer(const int32_t &capacity)
{
    auto allocator = AVAllocatorFactory::CreateSharedAllocator(MemoryFlag::MEMORY_READ_WRITE);
    std::shared_ptr<AVBuffer> buffer = AVBuffer::CreateAVBuffer(allocator, capacity);
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "CreateAVBuffer is nullptr!");
    return std::make_shared<AVBufferInnerMock>(buffer);
}
} // namespace Media
} // namespace OHOS