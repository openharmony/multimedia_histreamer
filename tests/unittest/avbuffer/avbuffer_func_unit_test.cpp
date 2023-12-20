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

#include "av_hardware_memory.h"
#include "av_shared_allocator.h"
#include "av_shared_memory_ext.h"
#include "av_surface_allocator.h"
#include "av_surface_memory.h"
#include "avbuffer_unit_test.h"
#include "avbuffer_utils.h"
#include "unittest_log.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;

namespace OHOS {
namespace Media {
namespace AVBufferUT {
/**
 * @tc.name: AVBuffer_Config_001
 * @tc.desc: test AVBufferConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Config_001, TestSize.Level1)
{
    AVBufferConfig configFirst;
    AVBufferConfig configSecond;
    configFirst.memoryType = MemoryType::HARDWARE_MEMORY;
    configSecond.memoryType = MemoryType::SURFACE_MEMORY;
    EXPECT_FALSE(configFirst.memoryType <= configSecond.memoryType);
}

/**
 * @tc.name: AVBuffer_Config_002
 * @tc.desc: test AVBufferConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Config_002, TestSize.Level1)
{
    AVBufferConfig configFirst;
    AVBufferConfig configSecond;
    configFirst.memoryType = configSecond.memoryType = MemoryType::HARDWARE_MEMORY;
    configFirst.size = 0;
    configSecond.capacity = 1;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 0;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 1;
    configSecond.capacity = 0;
    EXPECT_FALSE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 1;
    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 1;
    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 1;
    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    EXPECT_FALSE(configFirst <= configSecond);
}

/**
 * @tc.name: AVBuffer_Config_003
 * @tc.desc: test AVBufferConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Config_003, TestSize.Level1)
{
    AVBufferConfig configFirst;
    AVBufferConfig configSecond;
    configFirst.memoryType = configSecond.memoryType = MemoryType::SHARED_MEMORY;
    configFirst.size = 0;
    configSecond.capacity = 1;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 0;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 2; // 2: first size
    configSecond.capacity = 1;
    configSecond.align = 2; // 2: align size
    EXPECT_TRUE(configFirst <= configSecond);
    configSecond.align = 0;

    configFirst.size = 1;
    configSecond.capacity = 0;
    EXPECT_FALSE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 1;
    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 1;
    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 1;
    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    EXPECT_FALSE(configFirst <= configSecond);
}

/**
 * @tc.name: AVBuffer_Config_004
 * @tc.desc: test AVBufferConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Config_004, TestSize.Level1)
{
    AVBufferConfig configFirst;
    AVBufferConfig configSecond;
    configFirst.memoryType = configSecond.memoryType = MemoryType::SURFACE_MEMORY;
    configFirst.size = 1;
    configSecond.capacity = 0;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    EXPECT_TRUE(configFirst <= configSecond);

    *(configFirst.surfaceBufferConfig) = DEFAULT_CONFIG;
    EXPECT_FALSE(configFirst <= configSecond);

    *(configFirst.surfaceBufferConfig) = DEFAULT_CONFIG;
    *(configSecond.surfaceBufferConfig) = DEFAULT_CONFIG;
    EXPECT_TRUE(configFirst <= configSecond);
}

/**
 * @tc.name: AVBuffer_Config_005
 * @tc.desc: test AVBufferConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Config_005, TestSize.Level1)
{
    AVBufferConfig configFirst;
    AVBufferConfig configSecond;
    configFirst.memoryType = configSecond.memoryType = MemoryType::VIRTUAL_MEMORY;
    configFirst.size = 0;
    configSecond.capacity = 1;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 0;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 1;
    configSecond.capacity = 0;
    EXPECT_FALSE(configFirst <= configSecond);

    configFirst.size = 0;
    configSecond.capacity = 1;
    configFirst.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    configSecond.memoryFlag = MemoryFlag::MEMORY_READ_ONLY;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 0;
    configFirst.capacity = 1;
    configSecond.size = 1;
    configSecond.capacity = 1;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 1;
    configFirst.capacity = 0;
    configSecond.size = 0;
    configSecond.capacity = 1;
    EXPECT_TRUE(configFirst <= configSecond);

    configFirst.size = 1;
    configFirst.capacity = 0;
    configSecond.size = 1;
    configSecond.capacity = 0;
    EXPECT_FALSE(configFirst <= configSecond);
}

/**
 * @tc.name: AVBuffer_Config_006
 * @tc.desc: test AVBufferConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Config_006, TestSize.Level1)
{
    AVBufferConfig configRemote;
    configRemote.size = MEMSIZE;
    configRemote.align = POSITION_ONE;
    configRemote.memoryType = MemoryType::HARDWARE_MEMORY;
    configRemote.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    configRemote.capacity = TEST_BUFFER_SIZE;
    configRemote.dmaFd = 1;

    MessageParcel parcel;
    *(configRemote.surfaceBufferConfig) = DEFAULT_CONFIG;
    EXPECT_TRUE(MarshallingConfig(parcel, configRemote));

    AVBufferConfig configLocal;
    EXPECT_TRUE(UnmarshallingConfig(parcel, configLocal));

    EXPECT_EQ(configRemote.size, configLocal.size);
    EXPECT_EQ(configRemote.align, configLocal.align);
    EXPECT_EQ(configRemote.memoryType, configLocal.memoryType);
    EXPECT_EQ(configRemote.memoryFlag, configLocal.memoryFlag);
    EXPECT_EQ(configRemote.capacity, configLocal.capacity);
    EXPECT_EQ(configRemote.dmaFd, configLocal.dmaFd);

    EXPECT_EQ(configRemote.surfaceBufferConfig->width, configLocal.surfaceBufferConfig->width);
    EXPECT_EQ(configRemote.surfaceBufferConfig->height, configLocal.surfaceBufferConfig->height);
    EXPECT_EQ(configRemote.surfaceBufferConfig->strideAlignment, configLocal.surfaceBufferConfig->strideAlignment);
    EXPECT_EQ(configRemote.surfaceBufferConfig->format, configLocal.surfaceBufferConfig->format);
    EXPECT_EQ(configRemote.surfaceBufferConfig->usage, configLocal.surfaceBufferConfig->usage);
    EXPECT_EQ(configRemote.surfaceBufferConfig->timeout, configLocal.surfaceBufferConfig->timeout);
    EXPECT_EQ(configRemote.surfaceBufferConfig->colorGamut, configLocal.surfaceBufferConfig->colorGamut);
    EXPECT_EQ(configRemote.surfaceBufferConfig->transform, configLocal.surfaceBufferConfig->transform);
}

/**
 * @tc.name: AVBuffer_CreateWithInvalid_001
 * @tc.desc: create memory with invalid parcel
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_CreateWithInvalid_001, TestSize.Level1)
{
    parcel_ = std::make_shared<MessageParcel>();
    buffer_ = AVBuffer::CreateAVBuffer();
    ASSERT_FALSE(buffer_->ReadFromMessageParcel(*parcel_));
    ASSERT_EQ(buffer_->memory_, nullptr);
}

/**
 * @tc.name: AVBuffer_CreateWithInvalid_002
 * @tc.desc: create buffer with null allocator
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_CreateWithInvalid_002, TestSize.Level1)
{
    buffer_ = AVBuffer::CreateAVBuffer(nullptr);
    EXPECT_EQ(nullptr, buffer_);
}

/**
 * @tc.name: AVBuffer_CreateWithInvalid_003
 * @tc.desc: create virtual memory with invalid allocator
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_CreateWithInvalid_003, TestSize.Level1)
{
    allocator_ = AVAllocatorFactory::CreateVirtualAllocator();
    buffer_ = AVBuffer::CreateAVBuffer(allocator_, -1, MemoryFlag::MEMORY_READ_ONLY);
    EXPECT_EQ(nullptr, buffer_);

    buffer_ = AVBuffer::CreateAVBuffer(allocator_, 0, MemoryFlag::MEMORY_READ_ONLY);
    EXPECT_NE(nullptr, buffer_);
}

/**
 * @tc.name: AVBuffer_CreateWithInvalid_004
 * @tc.desc: create hardware memory with invalid allocator
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_CreateWithInvalid_004, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    DmabufHeapBuffer dmaBuffer = {.size = capacity_, .heapFlags = 0};
    int32_t dmaHeapFd = HardwareHeapFactory::GetInstance().GetHardwareHeapFd();
    DmabufHeapBufferAlloc(dmaHeapFd, &dmaBuffer);

    allocator_ = AVAllocatorFactory::CreateHardwareAllocator(0, capacity_, memFlag_);
    buffer_ = AVBuffer::CreateAVBuffer(allocator_);
    EXPECT_EQ(nullptr, buffer_);

    allocator_ = AVAllocatorFactory::CreateHardwareAllocator(dmaBuffer.fd, -1, memFlag_);
    buffer_ = AVBuffer::CreateAVBuffer(allocator_);
    EXPECT_EQ(nullptr, buffer_);

    allocator_ = AVAllocatorFactory::CreateHardwareAllocator(dmaBuffer.fd, 0, memFlag_);
    buffer_ = AVBuffer::CreateAVBuffer(allocator_);
    EXPECT_EQ(nullptr, buffer_);
}

/**
 * @tc.name: AVBuffer_Create_Local_SharedMemory_001
 * @tc.desc: create local shared memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_SharedMemory_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 2; // test align
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(std::static_pointer_cast<AVSharedMemoryExt>(buffer_->memory_)->allocator_->GetMemoryType(),
              MemoryType::SHARED_MEMORY);
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SHARED_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
    EXPECT_EQ(std::static_pointer_cast<AVSharedAllocator>(allocator_)->memFlag_, memFlag_);
}

/**
 * @tc.name: AVBuffer_Create_Local_SharedMemory_002
 * @tc.desc: create local shared memory and GetCapacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_SharedMemory_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    allocator_ = AVAllocatorFactory::CreateSharedAllocator(memFlag_);
    ASSERT_NE(nullptr, allocator_);
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(allocator_, capacity_, align_);
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetCapacity(), capacity_);
}

/**
 * @tc.name: AVBuffer_Create_Local_SharedMemory_003
 * @tc.desc: create local shared memory and GetCapacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_SharedMemory_003, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMemByConfig();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(std::static_pointer_cast<AVSharedMemoryExt>(buffer_->memory_)->allocator_->GetMemoryType(),
              MemoryType::SHARED_MEMORY);
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SHARED_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_SharedMemory_GetConfig_001
 * @tc.desc: create local shared memory and GetConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_GetConfig_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 1;
    CreateLocalSharedMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    AVBufferConfig config = buffer_->GetConfig();
    EXPECT_EQ(config.memoryType, MemoryType::SHARED_MEMORY);
    EXPECT_EQ(config.memoryFlag, memFlag_);
    EXPECT_EQ(config.capacity, capacity_);
    EXPECT_EQ(config.align, align_);
}

/**
 * @tc.name: AVBuffer_Create_Remote_SharedMemory_001
 * @tc.desc: create remote shared memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Remote_SharedMemory_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SHARED_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
    EXPECT_EQ(std::static_pointer_cast<AVSharedAllocator>(allocator_)->memFlag_, memFlag_);
    EXPECT_GT(std::static_pointer_cast<AVSharedMemoryExt>(buffer_->memory_)->fd_, 0);
}

/**
 * @tc.name: AVBuffer_SharedMemory_SetParams_001
 * @tc.desc: shared memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_SetParams_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_SharedMemory_SetParams_002
 * @tc.desc: shared memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_SetParams_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_SharedMemory_WriteAndRead_001
 * @tc.desc: shared memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_WriteAndRead_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_SharedMemory_WriteAndRead_002
 * @tc.desc: shared memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_WriteAndRead_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_SharedMemory_WriteAndRead_003
 * @tc.desc: shared memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_WriteAndRead_003, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransPos(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_SharedMemory_WriteAndRead_004
 * @tc.desc: shared memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_WriteAndRead_004, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransOutOfRange(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_SharedMemory_GetCapacity_001
 * @tc.desc: shared memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_GetCapacity_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetCapacity(), capacity_);
}

/**
 * @tc.name: AVBuffer_SharedMemory_GetCapacity_002
 * @tc.desc: shared memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_GetCapacity_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_EQ(buffer_->memory_->GetCapacity(), capacity_);
}

/**
 * @tc.name: AVBuffer_SharedMemory_CheckDataSize_001
 * @tc.desc: shared memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_CheckDataSize_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_SharedMemory_CheckDataSize_002
 * @tc.desc: shared memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_CheckDataSize_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_SharedMemory_GetMemoryType_001
 * @tc.desc: shared memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_GetMemoryType_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SHARED_MEMORY);
}

/**
 * @tc.name: AVBuffer_SharedMemory_GetMemoryType_002
 * @tc.desc: shared memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_GetMemoryType_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SHARED_MEMORY);
}

/**
 * @tc.name: AVBuffer_SharedMemory_GetFileDescriptor_001
 * @tc.desc: shared memory get memory file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_GetFileDescriptor_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_SharedMemory_GetFileDescriptor_002
 * @tc.desc: shared memory get memory file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_GetFileDescriptor_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_SharedMemory_Reset_001
 * @tc.desc: shared memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_Reset_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    buffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(buffer_->memory_->size_, MEMSIZE);
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_SharedMemory_Reset_002
 * @tc.desc: shared memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_Reset_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    remoteBuffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(remoteBuffer_->memory_->size_, MEMSIZE);
    remoteBuffer_->memory_->Reset();
    EXPECT_EQ(remoteBuffer_->memory_->size_, 0);
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_SharedMemory_ReadFromMessageParcel_001
 * @tc.desc: shared memory read from message parcel
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SharedMemory_ReadFromMessageParcel_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSharedMem();
    CheckAttrTrans();
}

/**
 * @tc.name: AVBuffer_Create_Local_SurfaceMemory_001
 * @tc.desc: create local surface memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_SurfaceMemory_001, TestSize.Level1)
{
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(std::static_pointer_cast<AVSurfaceMemory>(buffer_->memory_)->allocator_->GetMemoryType(),
              MemoryType::SURFACE_MEMORY);
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_Create_Local_SurfaceMemory_002
 * @tc.desc: create local surface memory by parcel
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_SurfaceMemory_002, TestSize.Level1)
{
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_Create_Local_SurfaceMemory_003
 * @tc.desc: create local surface memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_SurfaceMemory_003, TestSize.Level1)
{
    align_ = 0;
    CreateLocalSurfaceMemByConfig();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetConfig_001
 * @tc.desc: create local surface memory and GetConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetConfig_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    AVBufferConfig config = buffer_->GetConfig();
    EXPECT_EQ(config.memoryType, MemoryType::SURFACE_MEMORY);
    EXPECT_EQ(config.surfaceBufferConfig->width, DEFAULT_CONFIG.width);
    EXPECT_EQ(config.surfaceBufferConfig->height, DEFAULT_CONFIG.height);
    EXPECT_EQ(config.surfaceBufferConfig->format, DEFAULT_CONFIG.format);
    EXPECT_EQ(config.surfaceBufferConfig->usage, DEFAULT_CONFIG.usage);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetConfig_002
 * @tc.desc: create local surface memory and GetConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetConfig_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    CreateLocalSurfaceMemByConfig();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    AVBufferConfig config = buffer_->GetConfig();
    EXPECT_TRUE(config <= config_);
}

/**
 * @tc.name: AVBuffer_Create_Remote_SurfaceMemory_001
 * @tc.desc: create remote surface memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Remote_SurfaceMemory_001, TestSize.Level1)
{
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_Create_Remote_SurfaceMemory_002
 * @tc.desc: create remote surface memory by parcel
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Remote_SurfaceMemory_002, TestSize.Level1)
{
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_SetParams_001
 * @tc.desc: surface memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_SetParams_001, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_SetParams_002
 * @tc.desc: surface memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_SetParams_002, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_SetParams_003
 * @tc.desc: surface memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_SetParams_003, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_SetParams_004
 * @tc.desc: surface memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_SetParams_004, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_001
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_001, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_002
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_002, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_003
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_003, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransPos(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_004
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_004, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransOutOfRange(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_005
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_005, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_006
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_006, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_007
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_007, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransPos(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_WriteAndRead_008
 * @tc.desc: surface memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_WriteAndRead_008, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransOutOfRange(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetCapacity_001
 * @tc.desc: surface memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetCapacity_001, TestSize.Level1)
{
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetCapacity_002
 * @tc.desc: surface memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetCapacity_002, TestSize.Level1)
{
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetCapacity_003
 * @tc.desc: surface memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetCapacity_003, TestSize.Level1)
{
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetCapacity_004
 * @tc.desc: surface memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetCapacity_004, TestSize.Level1)
{
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_CheckDataSize_001
 * @tc.desc: surface memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_CheckDataSize_001, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_CheckDataSize_002
 * @tc.desc: surface memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_CheckDataSize_002, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_CheckDataSize_003
 * @tc.desc: surface memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_CheckDataSize_003, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_CheckDataSize_004
 * @tc.desc: surface memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_CheckDataSize_004, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetMemoryType_001
 * @tc.desc: surface memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetMemoryType_001, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetMemoryType_002
 * @tc.desc: surface memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetMemoryType_002, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetMemoryType_003
 * @tc.desc: surface memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetMemoryType_003, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetMemoryType_004
 * @tc.desc: surface memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetMemoryType_004, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::SURFACE_MEMORY);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetFileDescriptor_001
 * @tc.desc: surface memory get memory file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetFileDescriptor_001, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetFileDescriptor_002
 * @tc.desc: surface memory get memory file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetFileDescriptor_002, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetFileDescriptor_003
 * @tc.desc: surface memory get memory file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetFileDescriptor_003, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_GetFileDescriptor_004
 * @tc.desc: surface memory get memory file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_GetFileDescriptor_004, TestSize.Level1)
{
    capacity_ = DEFAULT_CONFIG.width * DEFAULT_CONFIG.height * DEFAULT_PIXELSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_Reset_001
 * @tc.desc: surface memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_Reset_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    buffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(buffer_->memory_->size_, MEMSIZE);
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_Reset_002
 * @tc.desc: surface memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_Reset_002, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    remoteBuffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(remoteBuffer_->memory_->size_, MEMSIZE);
    remoteBuffer_->memory_->Reset();
    EXPECT_EQ(remoteBuffer_->memory_->size_, 0);
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_Reset_003
 * @tc.desc: surface memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_Reset_003, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalSurfaceMemByParcel();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    buffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(buffer_->memory_->size_, MEMSIZE);
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_Reset_004
 * @tc.desc: surface memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_Reset_004, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSurfaceMemByParcel();
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    remoteBuffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(remoteBuffer_->memory_->size_, MEMSIZE);
    remoteBuffer_->memory_->Reset();
    EXPECT_EQ(remoteBuffer_->memory_->size_, 0);
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_SurfaceMemory_ReadFromMessageParcel_001
 * @tc.desc: surface memory read from message parcel
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_SurfaceMemory_ReadFromMessageParcel_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteSurfaceMem();
    CheckAttrTrans();
}

/**
 * @tc.name: AVBuffer_Create_Local_HardwareMemory_001
 * @tc.desc: create local hardware memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_HardwareMemory_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(std::static_pointer_cast<AVHardwareMemory>(buffer_->memory_)->allocator_->GetMemoryType(),
              MemoryType::HARDWARE_MEMORY);
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::HARDWARE_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
    EXPECT_EQ(std::static_pointer_cast<AVHardwareAllocator>(allocator_)->memFlag_, memFlag_);
}

/**
 * @tc.name: AVBuffer_Create_Local_HardwareMemory_002
 * @tc.desc: create local hardware memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_HardwareMemory_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMemByConfig();
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::HARDWARE_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_GetConfig_001
 * @tc.desc: create local hardware memory and GetConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_GetConfig_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    CreateLocalHardwareMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    AVBufferConfig config = buffer_->GetConfig();
    EXPECT_EQ(config.memoryType, MemoryType::HARDWARE_MEMORY);
    EXPECT_EQ(config.memoryFlag, memFlag_);
    EXPECT_EQ(config.capacity, capacity_);
}

/**
 * @tc.name: AVBuffer_Create_Remote_HardwareMemory_001
 * @tc.desc: create remote hardware memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Remote_HardwareMemory_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::HARDWARE_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
    EXPECT_EQ(std::static_pointer_cast<AVHardwareAllocator>(allocator_)->memFlag_, memFlag_);
    EXPECT_GT(std::static_pointer_cast<AVHardwareMemory>(buffer_->memory_)->fd_, 0);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_SetParams_001
 * @tc.desc: hardware memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_SetParams_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_HardwareMemory_SetParams_002
 * @tc.desc: hardware memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_SetParams_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_HardwareMemory_WriteAndRead_001
 * @tc.desc: hardware memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_WriteAndRead_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_HardwareMemory_WriteAndRead_002
 * @tc.desc: hardware memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_WriteAndRead_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_HardwareMemory_WriteAndRead_003
 * @tc.desc: hardware memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_WriteAndRead_003, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransPos(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_WriteAndRead_004
 * @tc.desc: hardware memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_WriteAndRead_004, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckMemTransOutOfRange(POSITION_ONE);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_GetCapacity_001
 * @tc.desc: hardware memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_GetCapacity_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetCapacity(), capacity_);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_GetCapacity_002
 * @tc.desc: hardware memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_GetCapacity_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_EQ(buffer_->memory_->GetCapacity(), capacity_);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_CheckDataSize_001
 * @tc.desc: hardware memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_CheckDataSize_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_HardwareMemory_CheckDataSize_002
 * @tc.desc: hardware memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_CheckDataSize_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_HardwareMemory_GetMemoryType_001
 * @tc.desc: hardware memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_GetMemoryType_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::HARDWARE_MEMORY);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_GetMemoryType_002
 * @tc.desc: hardware memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_GetMemoryType_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::HARDWARE_MEMORY);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_GetFileDescriptor_001
 * @tc.desc: hardware memory get file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_GetFileDescriptor_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_GetFileDescriptor_002
 * @tc.desc: hardware memory get file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_GetFileDescriptor_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    EXPECT_GT(buffer_->memory_->GetFileDescriptor(), 0);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_Reset_001
 * @tc.desc: hardware memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_Reset_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    buffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(buffer_->memory_->size_, MEMSIZE);
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_Reset_002
 * @tc.desc: hardware memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_Reset_002, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    remoteBuffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(remoteBuffer_->memory_->size_, MEMSIZE);
    remoteBuffer_->memory_->Reset();
    EXPECT_EQ(remoteBuffer_->memory_->size_, 0);
    GetRemoteBuffer();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_HardwareMemory_ReadFromMessageParcel_001
 * @tc.desc: hardware memory read from message parcel
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_HardwareMemory_ReadFromMessageParcel_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateRemoteHardwareMem();
    CheckAttrTrans();
}

/**
 * @tc.name: AVBuffer_Create_Local_VirtualMemory_001
 * @tc.desc: create local virtual memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_VirtualMemory_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::VIRTUAL_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_Create_Local_VirtualMemory_002
 * @tc.desc: create local virtual memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_VirtualMemory_002, TestSize.Level1)
{
    CreateLocalNullMem();
    ASSERT_NE(nullptr, remoteBuffer_);
    ASSERT_NE(nullptr, remoteBuffer_->meta_);
    ASSERT_EQ(nullptr, remoteBuffer_->memory_);
    remoteBuffer_->pts_ = DEFAULT_PTS;
    remoteBuffer_->dts_ = DEFAULT_DTS;
    remoteBuffer_->duration_ = DEFAULT_DURATION;
    remoteBuffer_->flag_ = DEFAULT_FLAG;

    GetRemoteBuffer();
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->meta_);
    ASSERT_EQ(nullptr, buffer_->memory_);
    EXPECT_EQ(remoteBuffer_->pts_, buffer_->pts_);
    EXPECT_EQ(remoteBuffer_->dts_, buffer_->dts_);
    EXPECT_EQ(remoteBuffer_->duration_, buffer_->duration_);
    EXPECT_EQ(remoteBuffer_->flag_, buffer_->flag_);
}

/**
 * @tc.name: AVBuffer_Create_Local_VirtualMemory_003
 * @tc.desc: create local virtual memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_VirtualMemory_003, TestSize.Level1)
{
    // create loacal
    capacity_ = MEMSIZE;
    uint8_t *array = new uint8_t[capacity_];
    auto error = memcpy_s(array, capacity_, inputBuffer_.data(), capacity_);
    ASSERT_EQ(error, EOK);
    buffer_ = AVBuffer::CreateAVBuffer(array, capacity_, capacity_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
    uint8_t *addr = buffer_->memory_->GetAddr();
    ASSERT_NE(addr, nullptr);
    EXPECT_EQ(memcmp(static_cast<void *>(addr), inputBuffer_.data(), capacity_), 0);
    // write and read
    outputBuffer_.resize(MEMSIZE, 0);
    EXPECT_EQ(buffer_->memory_->Read(outputBuffer_.data(), capacity_, 0), capacity_);
    EXPECT_EQ(buffer_->memory_->GetSize(), capacity_);
    EXPECT_EQ(memcmp(inputBuffer_.data(), outputBuffer_.data(), capacity_), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr), inputBuffer_.data(), capacity_), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr), outputBuffer_.data(), capacity_), 0);
    delete[] array;
}

/**
 * @tc.name: AVBuffer_Create_Local_VirtualMemory_004
 * @tc.desc: create local virtual memory
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_Create_Local_VirtualMemory_004, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMemByConfig();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    int32_t offset = static_cast<size_t>(
        AlignUp(reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()), static_cast<uintptr_t>(align_)) -
        reinterpret_cast<uintptr_t>(buffer_->memory_->GetAddr()));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::VIRTUAL_MEMORY);
    EXPECT_EQ(buffer_->memory_->capacity_, capacity_);
    EXPECT_EQ(buffer_->memory_->offset_, offset);
}

/**
 * @tc.name: AVBuffer_VirtualMemory_GetConfig_001
 * @tc.desc: create local virtual memory and GetConfig
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_GetConfig_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 1;
    CreateLocalVirtualMem();
    AVBufferConfig config = buffer_->GetConfig();
    EXPECT_EQ(config.memoryType, MemoryType::VIRTUAL_MEMORY);
    EXPECT_EQ(config.capacity, capacity_);
    EXPECT_EQ(config.align, align_);
}

/**
 * @tc.name: AVBuffer_VirtualMemory_SetParams_001
 * @tc.desc: virtual memory set params
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_SetParams_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMetaSetAndGet();
}

/**
 * @tc.name: AVBuffer_VirtualMemory_WriteAndRead_001
 * @tc.desc: virtual memory write and read
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_WriteAndRead_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckMemTrans();
}

/**
 * @tc.name: AVBuffer_VirtualMemory_GetCapacity_001
 * @tc.desc: virtual memory get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_GetCapacity_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetCapacity(), capacity_);
}

/**
 * @tc.name: AVBuffer_VirtualMemory_CheckDataSize_001
 * @tc.desc: virtual memory check dataSize
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_CheckDataSize_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    CheckDataSize();
}

/**
 * @tc.name: AVBuffer_VirtualMemory_GetMemoryType_001
 * @tc.desc: virtual memory get memory type
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_GetMemoryType_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetMemoryType(), MemoryType::VIRTUAL_MEMORY);
}

/**
 * @tc.name: AVBuffer_VirtualMemory_GetFileDescriptor_001
 * @tc.desc: virtual memory get memory file descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_GetFileDescriptor_001, TestSize.Level1)
{
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    EXPECT_EQ(buffer_->memory_->GetFileDescriptor(), -1);
}

/**
 * @tc.name: AVBuffer_VirtualMemory_Reset_001
 * @tc.desc: virtual memory reset
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_Reset_001, TestSize.Level1)
{
    memFlag_ = MemoryFlag::MEMORY_READ_WRITE;
    capacity_ = MEMSIZE;
    align_ = 0;
    CreateLocalVirtualMem();
    ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    buffer_->memory_->SetSize(MEMSIZE);
    EXPECT_EQ(buffer_->memory_->size_, MEMSIZE);
    buffer_->memory_->Reset();
    EXPECT_EQ(buffer_->memory_->size_, 0);
}

/**
 * @tc.name: AVBuffer_VirtualMemory_ReadFromMessageParcel_001
 * @tc.desc: null memory read from message parcel
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferInnerUnitTest, AVBuffer_VirtualMemory_ReadFromMessageParcel_001, TestSize.Level1)
{
    CreateLocalNullMem();
    CreateRemoteHardwareMem();
    CheckAttrTrans();
}
} // namespace AVBufferUT
} // namespace Media
} // namespace OHOS