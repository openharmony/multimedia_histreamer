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

#ifndef AVBUFFER_UNITTEST_H
#define AVBUFFER_UNITTEST_H
#include <dirent.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "av_hardware_allocator.h"
#include "buffer/avbuffer.h"
#include "meta.h"
#include "meta_key.h"
#include "surface_buffer.h"
#include "surface_type.h"

#define INT_TESTKEY Tag::APP_PID
#define LONG_TESTKEY Tag::MEDIA_DURATION
#define DOUBLE_TESTKEY Tag::VIDEO_CAPTURE_RATE
#define STRING_TESTKEY Tag::MEDIA_FILE_URI
namespace OHOS {
namespace Media {
class AVBufferMock;
namespace AVBufferUT {
const std::string_view INT_CAPI_TESTKEY = "IntKey";
const std::string_view LONG_CAPI_TESTKEY = "LongKey";
const std::string_view FlOAT_CAPI_TESTKEY = "FloatKey";
const std::string_view DOUBLE_CAPI_TESTKEY = "DoubleKey";
const std::string_view STRING_CAPI_TESTKEY = "StringKey";

constexpr int32_t MEMSIZE = 1024 * 1024;
constexpr int32_t POSITION_ONE = 1024 * 64;
constexpr int32_t TEST_BUFFER_SIZE = 1048 * 1048 * 8;
constexpr int32_t TEST_LOOP_DEPTH = 10;

const int32_t INTVALUE = 141;
const int64_t LONGVALUE = 115441;
const float FLOATVALUE = 1.25;
const double DOUBLEVALUE = 1.625;
const std::string STRINGVALUE = "STRING_TESTVALUE";

const int32_t DEFAULT_OFFSET = 1000;
const int64_t DEFAULT_PTS = 33000;
const int64_t DEFAULT_DTS = 100;
const int64_t DEFAULT_DURATION = 1000;
const uint32_t DEFAULT_FLAG = 1 << 0;

const int32_t DEFAULT_PIXELSIZE = 4;
const BufferRequestConfig DEFAULT_CONFIG = {
    .width = 800,
    .height = 600,
    .strideAlignment = 0x8,
    .format = GraphicPixelFormat::GRAPHIC_PIXEL_FMT_RGBA_8888,
    .usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA,
    .timeout = 0,
};

class AVBufferInnerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

private:
    void CreateLocalHardwareMem();
    void CreateLocalSharedMem();
    void CreateLocalSurfaceMem();
    void CreateLocalVirtualMem();

    void CreateLocalSurfaceMemByParcel();

    void CreateLocalHardwareMemByConfig();
    void CreateLocalSharedMemByConfig();
    void CreateLocalSurfaceMemByConfig();
    void CreateLocalVirtualMemByConfig();

    void CreateRemoteHardwareMem();
    void CreateRemoteSharedMem();
    void CreateRemoteSurfaceMem();
    void CreateRemoteSurfaceMemByParcel();
    void CreateLocalNullMem();
    void GetRemoteBuffer();

    void CheckMetaSetAndGet();
    void CheckMetaTransParcel();
    void CheckAttrTrans();
    void CheckMemTrans();
    void CheckMemTransPos(int32_t pos);
    void CheckMemTransOutOfRange(int32_t pos);
    void CheckDataSize();
    std::shared_ptr<AVAllocator> allocator_ = nullptr;
    std::shared_ptr<AVBuffer> buffer_ = nullptr;
    std::shared_ptr<AVBuffer> remoteBuffer_ = nullptr;
    std::shared_ptr<Meta> meta_ = nullptr;
    std::shared_ptr<MessageParcel> parcel_ = nullptr;
    MemoryFlag memFlag_;
    int32_t capacity_ = MEMSIZE;
    int32_t align_ = 0;
    int32_t dmaFd_ = -1;
    AVBufferConfig config_;
    std::vector<DmabufHeapBuffer> dmaBufferLst_;

    std::vector<uint8_t> inputBuffer_;
    std::vector<uint8_t> outputBuffer_;
};

class HardwareHeapFactory {
public:
    static HardwareHeapFactory &GetInstance();
    int32_t GetHardwareHeapFd();

private:
    HardwareHeapFactory();
    ~HardwareHeapFactory();
    int32_t dmaHeapFd_ = -1;
};

class AVBufferFrameworkUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

private:
    std::shared_ptr<AVBufferMock> buffer_;
};
} // namespace AVBufferUT
} // namespace Media
} // namespace OHOS
#endif // AVBUFFER_UNITTEST_H