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

#include "avbuffer_unit_test.h"
#include "av_hardware_allocator.h"
#include "av_hardware_memory.h"
#include "av_shared_allocator.h"
#include "av_shared_memory_ext.h"
#include "av_surface_allocator.h"
#include "av_surface_memory.h"
#include "avbuffer_mock.h"
#include "avbuffer_utils.h"
#include "unittest_log.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace AVBufferUT {
void AVBufferInnerUnitTest::SetUpTestCase(void)
{
    std::cout << "[SetUpTestCase]: SetUp!!!" << std::endl;
}

void AVBufferInnerUnitTest::TearDownTestCase(void)
{
    std::cout << "[TearDownTestCase]: over!!!" << std::endl;
}

void AVBufferInnerUnitTest::SetUp(void)
{
    std::cout << "[SetUp]: SetUp!!!";
    meta_ = std::make_shared<Meta>();
    const ::testing::TestInfo *testInfo_ = ::testing::UnitTest::GetInstance()->current_test_info();
    std::string testName = testInfo_->name();
    std::cout << testName << std::endl;
    parcel_ = nullptr;
    if (inputBuffer_.size() < TEST_BUFFER_SIZE) {
        inputBuffer_.resize(TEST_BUFFER_SIZE);
        for (int32_t i = 0; i < TEST_BUFFER_SIZE; ++i) {
            inputBuffer_[i] = rand() % 256; // 256: rand uint8_t range
        }
    }
    vector<uint8_t> temp(TEST_BUFFER_SIZE, 0);
    swap(temp, outputBuffer_);
}

void AVBufferInnerUnitTest::TearDown(void)
{
    allocator_ = nullptr;
    buffer_ = nullptr;
    meta_ = nullptr;
    parcel_ = nullptr;
    for (auto &buffer : dmaBufferLst_) {
        DmabufHeapBufferFree(&buffer);
    }
    std::vector<DmabufHeapBuffer> tmp;
    swap(tmp, dmaBufferLst_);
    std::cout << "[TearDown]: over!!!" << std::endl;
}

void AVBufferInnerUnitTest::CreateLocalHardwareMem()
{
    // create loacal
    DmabufHeapBuffer dmaBuffer = {.size = capacity_, .heapFlags = 0};
    int32_t dmaHeapFd = HardwareHeapFactory::GetInstance().GetHardwareHeapFd();
    DmabufHeapBufferAlloc(dmaHeapFd, &dmaBuffer);
    dmaBufferLst_.push_back(dmaBuffer);

    allocator_ = AVAllocatorFactory::CreateHardwareAllocator(dmaBuffer.fd, capacity_, memFlag_);
    ASSERT_NE(nullptr, allocator_);
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(allocator_, capacity_, align_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateLocalHardwareMemByConfig()
{
    // create loacal
    DmabufHeapBuffer dmaBuffer = {.size = capacity_, .heapFlags = 0};
    int32_t dmaHeapFd = HardwareHeapFactory::GetInstance().GetHardwareHeapFd();
    DmabufHeapBufferAlloc(dmaHeapFd, &dmaBuffer);
    dmaBufferLst_.push_back(dmaBuffer);

    config_.dmaFd = dmaBuffer.fd;
    config_.size = capacity_;
    config_.memoryFlag = memFlag_;
    config_.memoryType = MemoryType::HARDWARE_MEMORY;
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(config_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateRemoteHardwareMem()
{
    parcel_ = std::make_shared<MessageParcel>();
    // create remote
    CreateLocalHardwareMem();
    buffer_ = nullptr;
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    std::cout << "remote fd: " << remoteBuffer_->memory_->GetFileDescriptor() << "\n";
}

void AVBufferInnerUnitTest::CreateLocalSharedMem()
{
    // create loacal
    allocator_ = AVAllocatorFactory::CreateSharedAllocator(memFlag_);
    ASSERT_NE(nullptr, allocator_);
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(allocator_, capacity_, align_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateLocalSharedMemByConfig()
{
    // create loacal
    config_.align = align_;
    config_.size = capacity_;
    config_.memoryFlag = memFlag_;
    config_.memoryType = MemoryType::SHARED_MEMORY;
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(config_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateRemoteSharedMem()
{
    parcel_ = std::make_shared<MessageParcel>();
    // create remote
    CreateLocalSharedMem();
    buffer_ = nullptr;
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    std::cout << "remote fd: " << remoteBuffer_->memory_->GetFileDescriptor() << "\n";
}

void AVBufferInnerUnitTest::CreateLocalSurfaceMem()
{
    // create loacal
    allocator_ = AVAllocatorFactory::CreateSurfaceAllocator(DEFAULT_CONFIG);
    ASSERT_NE(nullptr, allocator_);
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(allocator_, capacity_, align_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateLocalSurfaceMemByConfig()
{
    // create loacal
    *(config_.surfaceBufferConfig) = DEFAULT_CONFIG;
    config_.memoryType = MemoryType::SURFACE_MEMORY;
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(config_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateRemoteSurfaceMem()
{
    parcel_ = std::make_shared<MessageParcel>();
    // create remote
    CreateLocalSurfaceMem();
    buffer_ = nullptr;
    ASSERT_FALSE((allocator_ == nullptr) || (remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    std::cout << "remote fd: " << remoteBuffer_->memory_->GetFileDescriptor() << "\n";
}

void AVBufferInnerUnitTest::CreateLocalSurfaceMemByParcel()
{
    MessageParcel parcel;
    sptr<SurfaceBuffer> surfaceBuffer = SurfaceBuffer::Create();
    (void)surfaceBuffer->Alloc(DEFAULT_CONFIG);
    (void)surfaceBuffer->WriteToMessageParcel(parcel);
    // create local
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer();
    ASSERT_NE(nullptr, buffer_);
    ASSERT_TRUE(buffer_->ReadFromMessageParcel(parcel, true));
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateRemoteSurfaceMemByParcel()
{
    parcel_ = std::make_shared<MessageParcel>();
    // create remote
    CreateLocalSurfaceMemByParcel();
    buffer_ = nullptr;
    ASSERT_FALSE((remoteBuffer_ == nullptr) || (remoteBuffer_->memory_ == nullptr));
    std::cout << "remote fd: " << remoteBuffer_->memory_->GetFileDescriptor() << "\n";
}

void AVBufferInnerUnitTest::CreateLocalVirtualMem()
{
    // create loacal
    allocator_ = AVAllocatorFactory::CreateVirtualAllocator();
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(allocator_, capacity_, align_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateLocalVirtualMemByConfig()
{
    // create loacal
    config_.align = align_;
    config_.size = capacity_;
    config_.memoryType = MemoryType::VIRTUAL_MEMORY;
    remoteBuffer_ = buffer_ = AVBuffer::CreateAVBuffer(config_);
    ASSERT_NE(nullptr, buffer_);
    ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
}

void AVBufferInnerUnitTest::CreateLocalNullMem()
{
    parcel_ = std::make_shared<MessageParcel>();
    // create remote
    remoteBuffer_ = AVBuffer::CreateAVBuffer();
    ASSERT_NE(nullptr, remoteBuffer_);
}

void AVBufferInnerUnitTest::GetRemoteBuffer()
{
    bool ret = remoteBuffer_->WriteToMessageParcel(*parcel_);
    ASSERT_TRUE(ret);
    // create loacal
    buffer_ = AVBuffer::CreateAVBuffer();
    ASSERT_NE(nullptr, buffer_);

    ret = buffer_->ReadFromMessageParcel(*parcel_);
    ASSERT_TRUE(ret);
    if (buffer_->memory_ != nullptr) {
        ASSERT_NE(nullptr, buffer_->memory_->GetAddr());
        std::cout << "local fd: " << buffer_->memory_->GetFileDescriptor() << "\n";
    }
}

void AVBufferInnerUnitTest::CheckMetaTransParcel()
{
    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    double getDoubleValue = 0.0;
    std::string getStringValue = "";
    MessageParcel parcel;
    for (int32_t toIndex = 0; toIndex < TEST_LOOP_DEPTH; ++toIndex) {
        ASSERT_TRUE(meta_->ToParcel(parcel));
    }
    for (int32_t fromIndex = 0; fromIndex < TEST_LOOP_DEPTH; ++fromIndex) {
        ASSERT_TRUE(meta_->FromParcel(parcel));
        meta_->GetData(INT_TESTKEY, getIntValue);
        meta_->GetData(LONG_TESTKEY, getLongValue);
        meta_->GetData(DOUBLE_TESTKEY, getDoubleValue);
        meta_->GetData(STRING_TESTKEY, getStringValue);

        EXPECT_EQ(getIntValue, INTVALUE);
        EXPECT_EQ(getLongValue, LONGVALUE);
        EXPECT_EQ(getDoubleValue, DOUBLEVALUE);
        EXPECT_EQ(getStringValue, STRINGVALUE);
    }
}

void AVBufferInnerUnitTest::CheckMetaSetAndGet()
{
    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    double getDoubleValue = 0.0;
    std::string getStringValue = "";

    remoteBuffer_->pts_ = DEFAULT_PTS;
    remoteBuffer_->dts_ = DEFAULT_DTS;
    remoteBuffer_->duration_ = DEFAULT_DURATION;
    remoteBuffer_->flag_ = DEFAULT_FLAG;

    meta_->SetData(INT_TESTKEY, INTVALUE);
    meta_->SetData(LONG_TESTKEY, LONGVALUE);
    meta_->SetData(DOUBLE_TESTKEY, DOUBLEVALUE);
    meta_->SetData(STRING_TESTKEY, STRINGVALUE);
    CheckMetaTransParcel();
    remoteBuffer_->meta_ = meta_;

    if (parcel_ != nullptr) {
        GetRemoteBuffer();
        ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
        ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    }
    ASSERT_NE(nullptr, buffer_->meta_);
    EXPECT_EQ(buffer_->pts_, DEFAULT_PTS);
    EXPECT_EQ(buffer_->dts_, DEFAULT_DTS);
    EXPECT_EQ(buffer_->duration_, DEFAULT_DURATION);
    EXPECT_EQ(buffer_->flag_, DEFAULT_FLAG);

    buffer_->meta_->GetData(INT_TESTKEY, getIntValue);
    buffer_->meta_->GetData(LONG_TESTKEY, getLongValue);
    buffer_->meta_->GetData(DOUBLE_TESTKEY, getDoubleValue);
    buffer_->meta_->GetData(STRING_TESTKEY, getStringValue);

    EXPECT_EQ(getIntValue, INTVALUE);
    EXPECT_EQ(getLongValue, LONGVALUE);
    EXPECT_EQ(getDoubleValue, DOUBLEVALUE);
    EXPECT_EQ(getStringValue, STRINGVALUE);
}

void AVBufferInnerUnitTest::CheckAttrTrans()
{
    remoteBuffer_->pts_ = DEFAULT_PTS;
    remoteBuffer_->dts_ = DEFAULT_DTS;
    remoteBuffer_->duration_ = DEFAULT_DURATION;
    remoteBuffer_->flag_ = DEFAULT_FLAG;

    GetRemoteBuffer();
    ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
    ASSERT_FALSE(buffer_ == nullptr);

    EXPECT_EQ(buffer_->pts_, DEFAULT_PTS);
    EXPECT_EQ(buffer_->dts_, DEFAULT_DTS);
    EXPECT_EQ(buffer_->duration_, DEFAULT_DURATION);
    EXPECT_EQ(buffer_->flag_, DEFAULT_FLAG);

    for (int32_t i = 0; i < TEST_LOOP_DEPTH; i++) {
        remoteBuffer_->pts_++;
        remoteBuffer_->dts_++;
        remoteBuffer_->duration_++;
        remoteBuffer_->flag_++;

        ASSERT_TRUE(remoteBuffer_->WriteToMessageParcel(*parcel_));
        ASSERT_TRUE(remoteBuffer_->WriteToMessageParcel(*parcel_));
        ASSERT_TRUE(buffer_->ReadFromMessageParcel(*parcel_));
        ASSERT_TRUE(buffer_->ReadFromMessageParcel(*parcel_));

        EXPECT_EQ(buffer_->pts_, remoteBuffer_->pts_);
        EXPECT_EQ(buffer_->dts_, remoteBuffer_->dts_);
        EXPECT_EQ(buffer_->duration_, remoteBuffer_->duration_);
        EXPECT_EQ(buffer_->flag_, remoteBuffer_->flag_);
    }
}

void AVBufferInnerUnitTest::CheckMemTrans()
{
    outputBuffer_.resize(TEST_BUFFER_SIZE, 0);
    int32_t pos = capacity_ / 2;
    int32_t length = capacity_ - pos;
    auto error = memcpy_s(remoteBuffer_->memory_->GetAddr(), pos, inputBuffer_.data(), pos);
    EXPECT_EQ(error, EOK);
    remoteBuffer_->memory_->SetSize(pos);
    EXPECT_EQ(remoteBuffer_->memory_->Write(inputBuffer_.data() + pos, length, -1), length);
    EXPECT_EQ(remoteBuffer_->memory_->Read(outputBuffer_.data(), capacity_, 0), capacity_);
    uint8_t *addr = remoteBuffer_->memory_->GetAddr();
    ASSERT_NE(addr, nullptr);
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), capacity_);
    EXPECT_EQ(memcmp(inputBuffer_.data(), outputBuffer_.data(), capacity_), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr), inputBuffer_.data(), capacity_), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr), outputBuffer_.data(), capacity_), 0);
    if (parcel_ != nullptr) {
        GetRemoteBuffer();
        ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
        ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    }
    ASSERT_EQ(buffer_->memory_->GetSize(), capacity_);
    addr = buffer_->memory_->GetAddr();
    EXPECT_EQ(buffer_->memory_->Read(outputBuffer_.data(), capacity_, 0), capacity_);
    EXPECT_EQ(memcmp(inputBuffer_.data(), outputBuffer_.data(), capacity_), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr), inputBuffer_.data(), capacity_), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr), outputBuffer_.data(), capacity_), 0);
}

void AVBufferInnerUnitTest::CheckMemTransPos(int32_t pos)
{
    outputBuffer_.resize(TEST_BUFFER_SIZE, 0);
    capacity_ = remoteBuffer_->memory_->GetCapacity();
    int32_t length = capacity_ - pos;
    EXPECT_EQ(remoteBuffer_->memory_->Write(inputBuffer_.data() + pos, length, pos), length);
    EXPECT_EQ(remoteBuffer_->memory_->Read(outputBuffer_.data() + pos, length, pos), length);
    uint8_t *addr = remoteBuffer_->memory_->GetAddr();
    ASSERT_NE(addr, nullptr);
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), capacity_);
    EXPECT_EQ(memcmp(inputBuffer_.data() + pos, outputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), inputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), outputBuffer_.data() + pos, length), 0);
    if (parcel_ != nullptr) {
        GetRemoteBuffer();
        ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
        ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    }
    ASSERT_EQ(buffer_->memory_->GetSize(), capacity_);
    addr = buffer_->memory_->GetAddr();
    EXPECT_EQ(buffer_->memory_->Read(outputBuffer_.data() + pos, length, pos), length);
    EXPECT_EQ(memcmp(inputBuffer_.data() + pos, outputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), inputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), outputBuffer_.data() + pos, length), 0);
}

void AVBufferInnerUnitTest::CheckMemTransOutOfRange(int32_t pos)
{
    outputBuffer_.resize(TEST_BUFFER_SIZE, 0);
    capacity_ = remoteBuffer_->memory_->GetCapacity();
    int32_t length = capacity_ - pos;
    EXPECT_EQ(remoteBuffer_->memory_->Write(inputBuffer_.data() + pos, length + 1, pos), length);
    EXPECT_EQ(remoteBuffer_->memory_->Read(outputBuffer_.data() + pos, length + 1, pos), length);
    uint8_t *addr = remoteBuffer_->memory_->GetAddr();
    ASSERT_NE(addr, nullptr);
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), capacity_);
    EXPECT_EQ(memcmp(inputBuffer_.data() + pos, outputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), inputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), outputBuffer_.data() + pos, length), 0);
    if (parcel_ != nullptr) {
        GetRemoteBuffer();
        ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
        ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    }
    ASSERT_EQ(buffer_->memory_->GetSize(), capacity_);
    addr = buffer_->memory_->GetAddr();
    EXPECT_EQ(buffer_->memory_->Read(outputBuffer_.data() + pos, length + 1, pos), length);
    EXPECT_EQ(memcmp(inputBuffer_.data() + pos, outputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), inputBuffer_.data() + pos, length), 0);
    EXPECT_EQ(memcmp(static_cast<void *>(addr + pos), outputBuffer_.data() + pos, length), 0);
}

void AVBufferInnerUnitTest::CheckDataSize()
{
    EXPECT_EQ(Status::OK, remoteBuffer_->memory_->SetSize(capacity_ - 1));
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), capacity_ - 1);
    EXPECT_EQ(Status::OK, remoteBuffer_->memory_->SetSize(0));
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), 0);
    EXPECT_EQ(Status::OK, remoteBuffer_->memory_->SetSize(1));
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), 1);
    EXPECT_EQ(Status::OK, remoteBuffer_->memory_->SetSize(-1));
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), 0);
    EXPECT_EQ(Status::OK, remoteBuffer_->memory_->SetSize(capacity_));
    EXPECT_EQ(remoteBuffer_->memory_->GetSize(), capacity_);
    if (parcel_ != nullptr) {
        GetRemoteBuffer();
        ASSERT_EQ(remoteBuffer_->GetUniqueId(), buffer_->GetUniqueId());
        ASSERT_FALSE((buffer_ == nullptr) || (buffer_->memory_ == nullptr));
    }
    EXPECT_EQ(buffer_->memory_->GetSize(), capacity_);
}

HardwareHeapFactory::HardwareHeapFactory()
{
    std::string rootDir = "/dev/dma_heap/";
    DIR *dir = opendir(rootDir.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent *ptr;
    std::string heapName = "";
    while ((ptr = readdir(dir)) != nullptr) {
        std::string fileName = ptr->d_name;
        std::string::size_type idx = fileName.find("system");
        if (idx != std::string::npos) {
            heapName = fileName;
            break;
        }
    }
    closedir(dir);
    dmaHeapFd_ = DmabufHeapOpen(heapName.c_str());
}

HardwareHeapFactory::~HardwareHeapFactory()
{
    DmabufHeapClose(dmaHeapFd_);
}

HardwareHeapFactory &HardwareHeapFactory::GetInstance()
{
    static HardwareHeapFactory hwHeapInstance;
    return hwHeapInstance;
}

int32_t HardwareHeapFactory::GetHardwareHeapFd()
{
    return dmaHeapFd_;
}

void AVBufferFrameworkUnitTest::SetUpTestCase(void) {}

void AVBufferFrameworkUnitTest::TearDownTestCase(void) {}

void AVBufferFrameworkUnitTest::SetUp(void)
{
    std::cout << "[SetUp]: SetUp!!!" << std::endl;
    const ::testing::TestInfo *testInfo_ = ::testing::UnitTest::GetInstance()->current_test_info();
    std::string testName = testInfo_->name();
    std::cout << testName << std::endl;

    buffer_ = AVBufferMockFactory::CreateAVBuffer(MEMSIZE);
    EXPECT_NE(nullptr, buffer_);
}

void AVBufferFrameworkUnitTest::TearDown(void)
{
    std::cout << "[TearDown]: over!!!" << std::endl;
}
} // namespace AVBufferUT
} // namespace Media
} // namespace OHOS