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

#include <memory>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "av_hardware_allocator.h"
#include "av_hardware_memory.h"
#include "av_shared_allocator.h"
#include "av_shared_memory_ext.h"
#include "av_surface_allocator.h"
#include "av_surface_memory.h"
#include "avbuffer.h"
#include "avbuffer_ipc.h"
#include "avbuffer_mock.h"
#include "avcodec_errors.h"
#include "avcodec_parcel.h"
#include "unittest_log.h"
#include "utils.h"
#ifdef AVBUFFER_CAPI_UNIT_TEST
#include "native_avmagic.h"
#endif

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MediaAVCodec;
namespace {
const int32_t MEMSIZE = 1024 * 190;
const int32_t g_offset = 1000;
const int64_t g_pts = 33000;
const AVCodecBufferFlag g_flag = AVCODEC_BUFFER_FLAG_EOS;

const std::string_view g_intKey = "IntKey";
const std::string_view g_longKey = "LongKey";
const std::string_view g_floatKey = "FloatKey";
const std::string_view g_doubleKey = "DoubleKey";
const std::string_view g_stringKey = "StringKey";

const int32_t g_intValue = 1;
const int64_t g_longValue = 1;
const float g_floatValue = 1.0;
const double g_doubleValue = 1.0;
const std::string g_stringValue = "StringValue";
} // namespace

namespace OHOS {
namespace MediaAVCodec {
namespace AVBufferFrameworkUT {
class AVBufferFrameworkUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

private:
    std::shared_ptr<AVBufferMock> buffer_;
};

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

/**
 * @tc.name: AVBuffer_Create_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Create_001, TestSize.Level1)
{
    EXPECT_EQ(AV_ERR_OK, buffer_->Destroy());
    buffer_ = nullptr;

    buffer_ = AVBufferMockFactory::CreateAVBuffer(-1);
    EXPECT_EQ(nullptr, buffer_);
}

/**
 * @tc.name: AVBuffer_GetAddr_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_GetAddr_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, buffer_->GetAddr());

    EXPECT_EQ(AV_ERR_OK, buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_GetCapacity_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_GetCapacity_001, TestSize.Level1)
{
    EXPECT_EQ(MEMSIZE, buffer_->GetCapacity());

    EXPECT_EQ(AV_ERR_OK, buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_SetBufferAttr_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_SetBufferAttr_001, TestSize.Level1)
{
    OH_AVCodecBufferAttr attr;
    attr.pts = g_pts;
    attr.size = MEMSIZE;
    attr.offset = g_offset;
    attr.flags = static_cast<uint32_t>(g_flag);
    EXPECT_EQ(AV_ERR_OK, buffer_->SetBufferAttr(attr));

    OH_AVCodecBufferAttr attrTemp = buffer_->GetBufferAttr();
    EXPECT_EQ(attr.pts, attrTemp.pts);
    EXPECT_EQ(attr.size, attrTemp.size);
    EXPECT_EQ(attr.offset, attrTemp.offset);
    EXPECT_EQ(attr.flags, attrTemp.flags);

    EXPECT_EQ(AV_ERR_OK, buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_SetParameter_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_SetParameter_001, TestSize.Level1)
{
    std::shared_ptr<FormatMock> format;

    EXPECT_TRUE(format->PutIntValue(g_intKey, g_intValue));
    EXPECT_TRUE(format->PutLongValue(g_longKey, g_longValue));
    EXPECT_TRUE(format->PutFloatValue(g_floatKey, g_floatValue));
    EXPECT_TRUE(format->PutDoubleValue(g_doubleKey, g_doubleValue));
    EXPECT_TRUE(format->PutStringValue(g_stringKey, g_stringValue));
    EXPECT_EQ(AV_ERR_OK, buffer_->SetParameter(format));

    std::shared_ptr<FormatMock> formatTemp = buffer_->GetParameter();
    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    float getFloatValue = 0.0;
    double getDoubleValue = 0.0;
    std::string getStringValue = "";

    EXPECT_TRUE(formatTemp->GetIntValue(g_intKey, getIntValue));
    EXPECT_TRUE(formatTemp->GetLongValue(g_longKey, getLongValue));
    EXPECT_TRUE(formatTemp->GetFloatValue(g_floatKey, getFloatValue));
    EXPECT_TRUE(formatTemp->GetDoubleValue(g_doubleKey, getDoubleValue));
    EXPECT_TRUE(formatTemp->GetStringValue(g_stringKey, getStringValue));

    EXPECT_EQ(getIntValue, g_intValue);
    EXPECT_EQ(getLongValue, g_longValue);
    EXPECT_EQ(getFloatValue, g_floatValue);
    EXPECT_EQ(getDoubleValue, g_doubleValue);
    EXPECT_EQ(getStringValue, g_stringValue);

    EXPECT_EQ(AV_ERR_OK, buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_Destroy_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Destroy_001, TestSize.Level1)
{
    EXPECT_EQ(AV_ERR_OK, buffer_->Destroy());
    buffer_ = nullptr;
}

#ifdef AVBUFFER_CAPI_UNIT_TEST
#endif
} // namespace AVBufferFrameworkUT
} // namespace MediaAVCodec
} // namespace OHOS