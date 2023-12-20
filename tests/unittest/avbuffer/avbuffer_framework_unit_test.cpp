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

#include "avbuffer_mock.h"
#include "avbuffer_unit_test.h"
#include "avbuffer_utils.h"
#include "common/log.h"
#include "common/status.h"
#include "unittest_log.h"
#ifdef AVBUFFER_CAPI_UNIT_TEST
#include "common/native_mfmagic.h"
#include "native_avbuffer.h"
#include "native_avformat.h"
#endif

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace AVBufferUT {
/**
 * @tc.name: AVBuffer_Create_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Create_001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;

    buffer_ = AVBufferMockFactory::CreateAVBuffer(-1);
    EXPECT_EQ(nullptr, buffer_);
}

/**
 * @tc.name: AVBuffer_GetAddr_001
 * @tc.desc: buffer get memory addr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_GetAddr_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, buffer_->GetAddr());

    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_GetCapacity_001
 * @tc.desc: buffer get capacity
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_GetCapacity_001, TestSize.Level1)
{
    EXPECT_EQ(MEMSIZE, buffer_->GetCapacity());

    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_SetBufferAttr_001
 * @tc.desc: buffer get and set buffer attribute
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_SetBufferAttr_001, TestSize.Level1)
{
    OH_AVCodecBufferAttr attr;
    attr.pts = DEFAULT_PTS;
    attr.size = MEMSIZE;
    attr.offset = DEFAULT_OFFSET;
    attr.flags = DEFAULT_FLAG;
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->SetBufferAttr(attr));

    OH_AVCodecBufferAttr attrTemp;
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->GetBufferAttr(attrTemp));
    EXPECT_EQ(attr.pts, attrTemp.pts);
    EXPECT_EQ(attr.size, attrTemp.size);
    EXPECT_EQ(attr.offset, attrTemp.offset);
    EXPECT_EQ(attr.flags, attrTemp.flags);

    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_SetParameter_001
 * @tc.desc: buffer get and set parameter
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_SetParameter_001, TestSize.Level1)
{
    std::shared_ptr<FormatMock> format;

    EXPECT_TRUE(format->PutIntValue(INT_CAPI_TESTKEY, INTVALUE));
    EXPECT_TRUE(format->PutLongValue(LONG_CAPI_TESTKEY, LONGVALUE));
    EXPECT_TRUE(format->PutFloatValue(FlOAT_CAPI_TESTKEY, FLOATVALUE));
    EXPECT_TRUE(format->PutDoubleValue(DOUBLE_CAPI_TESTKEY, DOUBLEVALUE));
    EXPECT_TRUE(format->PutStringValue(STRING_CAPI_TESTKEY, STRINGVALUE));
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->SetParameter(format));

    std::shared_ptr<FormatMock> formatTemp = buffer_->GetParameter();
    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    float getFloatValue = 0.0;
    double getDoubleValue = 0.0;
    std::string getStringValue = "";

    EXPECT_TRUE(formatTemp->GetIntValue(INT_CAPI_TESTKEY, getIntValue));
    EXPECT_TRUE(formatTemp->GetLongValue(LONG_CAPI_TESTKEY, getLongValue));
    EXPECT_TRUE(formatTemp->GetFloatValue(FlOAT_CAPI_TESTKEY, getFloatValue));
    EXPECT_TRUE(formatTemp->GetDoubleValue(DOUBLE_CAPI_TESTKEY, getDoubleValue));
    EXPECT_TRUE(formatTemp->GetStringValue(STRING_CAPI_TESTKEY, getStringValue));

    EXPECT_EQ(getIntValue, INTVALUE);
    EXPECT_EQ(getLongValue, LONGVALUE);
    EXPECT_EQ(getFloatValue, FLOATVALUE);
    EXPECT_EQ(getDoubleValue, DOUBLEVALUE);
    EXPECT_EQ(getStringValue, STRINGVALUE);

    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_GetNativeBuffer_001
 * @tc.desc: get native buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_GetNativeBuffer_001, TestSize.Level1)
{
    EXPECT_EQ(nullptr, buffer_->GetNativeBuffer());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_Destroy_001
 * @tc.desc: destroy buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Destroy_001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;
}

#ifdef AVBUFFER_CAPI_UNIT_TEST

/**
 * @tc.name: AVBuffer_Capi_Create_001
 * @tc.desc: create buffer with capacity > 0
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_Create_001, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_Create_002
 * @tc.desc: create buffer with capacity = 0
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_Create_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(0);
    ASSERT_EQ(buffer, nullptr);
}

/**
 * @tc.name: AVBuffer_Capi_Create_003
 * @tc.desc: create buffer with capacity < 0
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_Create_003, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(-1);
    ASSERT_EQ(buffer, nullptr);
}

/**
 * @tc.name: AVBuffer_Capi_Destroy_001
 * @tc.desc: Destroy buffer with nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_Destroy_001, TestSize.Level1)
{
    EXPECT_EQ(OH_AVBuffer_Destroy(nullptr), AV_ERR_INVALID_VAL);
}

/**
 * @tc.name: AVBuffer_Capi_Destroy_002
 * @tc.desc: Destroy buffer with not user created
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_Destroy_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    buffer->isUserCreated = false;
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OPERATE_NOT_PERMIT);
    delete buffer;
}

/**
 * @tc.name: AVBuffer_Capi_Destroy_003
 * @tc.desc: Destroy buffer with error magic object
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_Destroy_003, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    buffer->magic_ = MFMagic::MFMAGIC_FORMAT;
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_INVALID_VAL);
    delete buffer;
}

/**
 * @tc.name: AVBuffer_Capi_SetAndGetBufferAttr_001
 * @tc.desc: Set buffer attr with memory is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetAndGetBufferAttr_001, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    OH_AVCodecBufferAttr attr;
    attr.size = MEMSIZE;
    attr.offset = DEFAULT_OFFSET;
    attr.pts = DEFAULT_PTS;
    attr.flags = DEFAULT_FLAG;
    EXPECT_EQ(OH_AVBuffer_SetBufferAttr(buffer, &attr), AV_ERR_OK);

    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_OK, OH_AVBuffer_GetBufferAttr(buffer, &getAttr));
    EXPECT_EQ(getAttr.size, MEMSIZE);
    EXPECT_EQ(getAttr.offset, DEFAULT_OFFSET);
    EXPECT_EQ(getAttr.pts, DEFAULT_PTS);
    EXPECT_EQ(getAttr.flags, DEFAULT_FLAG);
}

/**
 * @tc.name: AVBuffer_Capi_SetAndGetBufferAttr_002
 * @tc.desc: Set buffer attr with memory is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetAndGetBufferAttr_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    OH_AVCodecBufferAttr attr;
    attr.size = MEMSIZE;
    attr.offset = DEFAULT_OFFSET;
    attr.pts = DEFAULT_PTS;
    attr.flags = DEFAULT_FLAG;
    buffer->buffer_->memory_ = nullptr;
    EXPECT_EQ(OH_AVBuffer_SetBufferAttr(buffer, &attr), AV_ERR_OK);

    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_OK, OH_AVBuffer_GetBufferAttr(buffer, &getAttr));
    EXPECT_EQ(getAttr.size, 0);
    EXPECT_EQ(getAttr.offset, 0);
    EXPECT_EQ(getAttr.pts, DEFAULT_PTS);
    EXPECT_EQ(getAttr.flags, DEFAULT_FLAG);
}

/**
 * @tc.name: AVBuffer_Capi_GetBufferAttr_Invalid_001
 * @tc.desc: Get buffer attr with attr is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetBufferAttr_Invalid_001, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_GetBufferAttr(buffer, nullptr));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetBufferAttr_Invalid_002
 * @tc.desc: Get buffer attr with buffer->buffer_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetBufferAttr_Invalid_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    buffer->buffer_ = nullptr;
    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_GetBufferAttr(buffer, &getAttr));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetBufferAttr_Invalid_003
 * @tc.desc: Get buffer attr with buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetBufferAttr_Invalid_003, TestSize.Level1)
{
    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_GetBufferAttr(nullptr, &getAttr));
}

/**
 * @tc.name: AVBuffer_Capi_GetBufferAttr_Invalid_004
 * @tc.desc: Get buffer attr with buffer magic is error
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetBufferAttr_Invalid_004, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->magic_ = MFMagic::MFMAGIC_FORMAT;
    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_GetBufferAttr(buffer, &getAttr));
    buffer->magic_ = MFMagic::MFMAGIC_AVBUFFER;
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_SetBufferAttr_Invalid_001
 * @tc.desc: Set buffer attr with attr is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetBufferAttr_Invalid_001, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_SetBufferAttr(buffer, nullptr));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_SetBufferAttr_Invalid_002
 * @tc.desc: Set buffer attr with buffer->buffer_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetBufferAttr_Invalid_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    buffer->buffer_ = nullptr;
    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_SetBufferAttr(buffer, &getAttr));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_SetBufferAttr_Invalid_003
 * @tc.desc: Set buffer attr with buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetBufferAttr_Invalid_003, TestSize.Level1)
{
    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_SetBufferAttr(nullptr, &getAttr));
}

/**
 * @tc.name: AVBuffer_Capi_SetBufferAttr_Invalid_004
 * @tc.desc: Set buffer attr with buffer magic is error
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetBufferAttr_Invalid_004, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->magic_ = MFMagic::MFMAGIC_FORMAT;
    OH_AVCodecBufferAttr getAttr;
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_SetBufferAttr(buffer, &getAttr));
    buffer->magic_ = MFMagic::MFMAGIC_AVBUFFER;
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_SetAndGetParameter_001
 * @tc.desc: Set buffer parameter
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetAndGetParameter_001, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    auto format = OH_AVFormat_Create();
    ASSERT_NE(format, nullptr);
    EXPECT_TRUE(OH_AVFormat_SetIntValue(format, INT_CAPI_TESTKEY.data(), INTVALUE));
    EXPECT_TRUE(OH_AVFormat_SetLongValue(format, LONG_CAPI_TESTKEY.data(), LONGVALUE));
    EXPECT_TRUE(OH_AVFormat_SetFloatValue(format, FlOAT_CAPI_TESTKEY.data(), FLOATVALUE));
    EXPECT_TRUE(OH_AVFormat_SetDoubleValue(format, DOUBLE_CAPI_TESTKEY.data(), DOUBLEVALUE));
    EXPECT_TRUE(OH_AVFormat_SetStringValue(format, STRING_CAPI_TESTKEY.data(), STRINGVALUE.c_str()));

    EXPECT_EQ(OH_AVBuffer_SetParameter(buffer, format), AV_ERR_OK);

    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    float getFloatValue = 0.0;
    double getDoubleValue = 0.0;
    const char *getStringValue = nullptr;

    auto getFormat = OH_AVBuffer_GetParameter(buffer);
    ASSERT_NE(getFormat, nullptr);
    EXPECT_TRUE(OH_AVFormat_GetIntValue(getFormat, INT_CAPI_TESTKEY.data(), &getIntValue));
    EXPECT_TRUE(OH_AVFormat_GetLongValue(getFormat, LONG_CAPI_TESTKEY.data(), &getLongValue));
    EXPECT_TRUE(OH_AVFormat_GetFloatValue(getFormat, FlOAT_CAPI_TESTKEY.data(), &getFloatValue));
    EXPECT_TRUE(OH_AVFormat_GetDoubleValue(getFormat, DOUBLE_CAPI_TESTKEY.data(), &getDoubleValue));
    EXPECT_TRUE(OH_AVFormat_GetStringValue(getFormat, STRING_CAPI_TESTKEY.data(), &getStringValue));

    EXPECT_EQ(INTVALUE, getIntValue);
    EXPECT_EQ(LONGVALUE, getLongValue);
    EXPECT_EQ(FLOATVALUE, getFloatValue);
    EXPECT_EQ(DOUBLEVALUE, getDoubleValue);
    EXPECT_EQ(STRINGVALUE, std::string(getStringValue));
    OH_AVFormat_Destroy(format);
    OH_AVFormat_Destroy(getFormat);
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_SetAndGetParameter_002
 * @tc.desc: Get buffer parameter 3 times
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetAndGetParameter_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    auto format = OH_AVFormat_Create();
    ASSERT_NE(format, nullptr);
    EXPECT_TRUE(OH_AVFormat_SetIntValue(format, INT_CAPI_TESTKEY.data(), INTVALUE));
    EXPECT_EQ(OH_AVBuffer_SetParameter(buffer, format), AV_ERR_OK);

    int32_t getIntValue = 0;
    auto getFormat = OH_AVBuffer_GetParameter(buffer);
    OH_AVFormat_Destroy(getFormat);
    getFormat = OH_AVBuffer_GetParameter(buffer);
    OH_AVFormat_Destroy(getFormat);
    getFormat = OH_AVBuffer_GetParameter(buffer);

    ASSERT_NE(getFormat, nullptr);
    EXPECT_TRUE(OH_AVFormat_GetIntValue(getFormat, INT_CAPI_TESTKEY.data(), &getIntValue));
    EXPECT_EQ(INTVALUE, getIntValue);
    OH_AVFormat_Destroy(format);
    OH_AVFormat_Destroy(getFormat);
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_SetParameter_Invalid_001
 * @tc.desc: Set buffer parameter with buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetParameter_Invalid_001, TestSize.Level1)
{
    auto format = OH_AVFormat_Create();
    ASSERT_NE(format, nullptr);
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_SetParameter(nullptr, format));
    OH_AVFormat_Destroy(format);
}

/**
 * @tc.name: AVBuffer_Capi_SetParameter_Invalid_002
 * @tc.desc: Set buffer parameter with format is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_SetParameter_Invalid_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(AV_ERR_INVALID_VAL, OH_AVBuffer_SetParameter(buffer, nullptr));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetParameter_Invalid_001
 * @tc.desc: Get buffer parameter with buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetParameter_Invalid_001, TestSize.Level1)
{
    EXPECT_EQ(nullptr, OH_AVBuffer_GetParameter(nullptr));
}

/**
 * @tc.name: AVBuffer_Capi_GetAddr_Invalid_001
 * @tc.desc: Get buffer address with buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetAddr_Invalid_001, TestSize.Level1)
{
    EXPECT_EQ(nullptr, OH_AVBuffer_GetAddr(nullptr));
}

/**
 * @tc.name: AVBuffer_Capi_GetAddr_Invalid_002
 * @tc.desc: Get buffer address with buffer magic is error
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetAddr_Invalid_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->magic_ = MFMagic::MFMAGIC_FORMAT;
    EXPECT_EQ(nullptr, OH_AVBuffer_GetAddr(buffer));
    buffer->magic_ = MFMagic::MFMAGIC_AVBUFFER;
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetAddr_Invalid_003
 * @tc.desc: Get buffer address with buffer->buffer_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetAddr_Invalid_003, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->buffer_ = nullptr;
    EXPECT_EQ(nullptr, OH_AVBuffer_GetAddr(buffer));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetAddr_Invalid_004
 * @tc.desc: Get buffer address with buffer->buffer_->memory_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetAddr_Invalid_004, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->buffer_->memory_ = nullptr;
    EXPECT_EQ(nullptr, OH_AVBuffer_GetAddr(buffer));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetCapacity_Invalid_001
 * @tc.desc: Get buffer address with buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetCapacity_Invalid_001, TestSize.Level1)
{
    EXPECT_EQ(-1, OH_AVBuffer_GetCapacity(nullptr));
}

/**
 * @tc.name: AVBuffer_Capi_GetCapacity_Invalid_002
 * @tc.desc: Get buffer address with buffer magic is error
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetCapacity_Invalid_002, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->magic_ = MFMagic::MFMAGIC_FORMAT;
    EXPECT_EQ(-1, OH_AVBuffer_GetCapacity(buffer));
    buffer->magic_ = MFMagic::MFMAGIC_AVBUFFER;
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetCapacity_Invalid_003
 * @tc.desc: Get buffer address with buffer->buffer_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetCapacity_Invalid_003, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->buffer_ = nullptr;
    EXPECT_EQ(-1, OH_AVBuffer_GetCapacity(buffer));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetCapacity_Invalid_004
 * @tc.desc: Get buffer address with buffer->buffer_->memory_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetCapacity_Invalid_004, TestSize.Level1)
{
    auto buffer = OH_AVBuffer_Create(MEMSIZE);
    buffer->buffer_->memory_ = nullptr;
    EXPECT_EQ(-1, OH_AVBuffer_GetCapacity(buffer));
    EXPECT_EQ(OH_AVBuffer_Destroy(buffer), AV_ERR_OK);
}

/**
 * @tc.name: AVBuffer_Capi_GetNativeBuffer_001
 * @tc.desc: get native buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Capi_GetNativeBuffer_001, TestSize.Level1)
{
    auto allocator = AVAllocatorFactory::CreateSurfaceAllocator(DEFAULT_CONFIG);
    ASSERT_NE(nullptr, allocator);
    auto surfaceAVBuffer = AVBuffer::CreateAVBuffer(allocator, 0, 0);
    ASSERT_NE(nullptr, surfaceAVBuffer);
    ASSERT_NE(nullptr, surfaceAVBuffer->memory_->GetAddr());

    struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(surfaceAVBuffer);
    auto nativeBuffer = OH_AVBuffer_GetNativeBuffer(buf);
    EXPECT_NE(nullptr, nativeBuffer);
    EXPECT_EQ(0, OH_NativeBuffer_Unreference(nativeBuffer));
    delete buf;
}
#endif
} // namespace AVBufferUT
} // namespace Media
} // namespace OHOS