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
#include "avbuffer_mock.h"
#include "avbuffer_utils.h"
#include "avcodec_errors.h"
#include "unittest_log.h"
#ifdef AVBUFFER_CAPI_UNIT_TEST
#include "native_avmagic.h"
#endif

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MediaAVCodec;
namespace {
const int32_t MEMSIZE = 1024 * 190;
const int32_t DEFAULT_OFFSET = 1000;
const int64_t DEFAULT_PTS = 33000;
const AVCodecBufferFlag DEFAULT_FLAG = MF_BUFFER_FLAG_EOS;

const std::string_view INT_CAPI_TESTKEY = "IntKey";
const std::string_view LONG_CAPI_TESTKEY = "LongKey";
const std::string_view FlOAT_CAPI_TESTKEY = "FloatKey";
const std::string_view DOUBLE_CAPI_TESTKEY = "DoubleKey";
const std::string_view STRING_CAPI_TESTKEY = "StringKey";

const int32_t INTVALUE = 1;
const int64_t LONGVALUE = 1;
const float FLOATVALUE = 1.0;
const double DOUBLEVALUE = 1.0;
const std::string STRINGVALUE = "StringValue";
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
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
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

    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
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

    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;
}

/**
 * @tc.name: AVBuffer_SetBufferAttr_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_SetBufferAttr_001, TestSize.Level1)
{
    OH_AVBufferAttr attr;
    attr.pts = DEFAULT_PTS;
    attr.size = MEMSIZE;
    attr.offset = DEFAULT_OFFSET;
    attr.flags = static_cast<uint32_t>(DEFAULT_FLAG);
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->SetBufferAttr(attr));

    OH_AVBufferAttr attrTemp = buffer_->GetBufferAttr();
    EXPECT_EQ(attr.pts, attrTemp.pts);
    EXPECT_EQ(attr.size, attrTemp.size);
    EXPECT_EQ(attr.offset, attrTemp.offset);
    EXPECT_EQ(attr.flags, attrTemp.flags);

    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
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
 * @tc.name: AVBuffer_Destroy_001
 * @tc.desc: create buffer
 * @tc.type: FUNC
 */
HWTEST_F(AVBufferFrameworkUnitTest, AVBuffer_Destroy_001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(Status::OK), buffer_->Destroy());
    buffer_ = nullptr;
}

#ifdef AVBUFFER_CAPI_UNIT_TEST
#endif
} // namespace AVBufferFrameworkUT
} // namespace MediaAVCodec
} // namespace OHOS