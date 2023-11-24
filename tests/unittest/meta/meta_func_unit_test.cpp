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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "meta.h"
#include "meta_key.h"
#include "unittest_log.h"
#include "video_types.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;

#define INT_TESTKEY Tag::APP_PID
#define INT_ENUM_TESTKEY Tag::VIDEO_ROTATION
#define LONG_TESTKEY Tag::MEDIA_DURATION
#define LONG_ENUM_TESTKEY Tag::AUDIO_CHANNEL_LAYOUT
#define DOUBLE_TESTKEY Tag::VIDEO_CAPTURE_RATE
#define STRING_TESTKEY Tag::MEDIA_FILE_URI
namespace {
const int32_t INTVALUE = 141;
const int64_t LONGVALUE = 17592186044673;
const double DOUBLEVALUE = 1.59261111;
const std::string STRINGVALUE = "STRING_TESTVALUE";
} // namespace

namespace OHOS {
namespace Media {
namespace MetaFuncUT {
class MetaInnerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp(void);

    void TearDown(void);

private:
    void CheckMetaSetAndGet();

    void CheckMetaTransParcel();

    std::shared_ptr<Meta> meta_ = nullptr;
    std::shared_ptr<MessageParcel> parcel_ = nullptr;
};

void MetaInnerUnitTest::SetUpTestCase(void) {}

void MetaInnerUnitTest::TearDownTestCase(void) {}

void MetaInnerUnitTest::SetUp(void)
{
    std::cout << "[SetUp]: SetUp!!!, test: ";
    meta_ = std::make_shared<Meta>();
    const ::testing::TestInfo *testInfo_ = ::testing::UnitTest::GetInstance()->current_test_info();
    std::string testName = testInfo_->name();
    std::cout << testName << std::endl;
    parcel_ = nullptr;
}

void MetaInnerUnitTest::TearDown(void)
{
    meta_ = nullptr;
    parcel_ = nullptr;
    std::cout << "[TearDown]: over!!!" << std::endl;
}

/**
 * @tc.name: SetGet_Int32
 * @tc.desc: SetGet_Int32
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_Int32, TestSize.Level1)
{
    int32_t valueOut = 0;
    int32_t valueIn = INTVALUE;
    meta_ = std::make_shared<Meta>();
    meta_->Set<INT_TESTKEY>(valueIn);
    meta_->Get<INT_TESTKEY>(valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_Double
 * @tc.desc: SetGet_MetaData_Double
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_Double, TestSize.Level1)
{
    double valueOut = 0;
    double valueIn = DOUBLEVALUE;
    meta_ = std::make_shared<Meta>();
    meta_->Set<DOUBLE_TESTKEY>(valueIn);
    meta_->Get<DOUBLE_TESTKEY>(valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_Long
 * @tc.desc: SetGet_MetaData_Long
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_Long, TestSize.Level1)
{
    int64_t valueOut = 0;
    int64_t valueIn = LONGVALUE;
    meta_ = std::make_shared<Meta>();
    meta_->Set<LONG_TESTKEY>(valueIn);
    meta_->Get<LONG_TESTKEY>(valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_String
 * @tc.desc: SetGet_MetaData_String
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_String, TestSize.Level1)
{
    std::string valueOut = 0;
    std::string valueIn = STRINGVALUE;
    meta_ = std::make_shared<Meta>();
    meta_->Set<STRING_TESTKEY>(valueIn);
    meta_->Get<STRING_TESTKEY>(valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_Data_Int32_Using_Parcel
 * @tc.desc: SetGet_Data_Int32_Using_Parcel
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_Data_Int32_Using_Parcel, TestSize.Level1)
{
    MessageParcel parcel;
    int32_t valueOut = 0;
    int32_t valueIn = INTVALUE;
    meta_ = std::make_shared<Meta>();
    meta_->SetData(INT_TESTKEY, valueIn);
    ASSERT_TRUE(meta_->ToParcel(parcel));
    ASSERT_TRUE(meta_->FromParcel(parcel));
    meta_->GetData(INT_TESTKEY, valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_Int32
 * @tc.desc: SetGet_MetaData_Int32
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_Int32, TestSize.Level1)
{
    int32_t valueOut = 0;
    int32_t valueIn = INTVALUE;
    meta_ = std::make_shared<Meta>();
    SetMetaData(*meta_, INT_TESTKEY, valueIn);
    GetMetaData(*meta_, INT_TESTKEY, valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_Int32_Using_Parcel
 * @tc.desc: SetGet_MetaData_Int32_Using_Parcel
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_Int32_Using_Parcel, TestSize.Level1)
{
    MessageParcel parcel;
    int32_t valueOut = 0;
    int32_t valueIn = INTVALUE;
    meta_ = std::make_shared<Meta>();
    SetMetaData(*meta_, INT_TESTKEY, valueIn);
    ASSERT_TRUE(meta_->ToParcel(parcel));
    ASSERT_TRUE(meta_->FromParcel(parcel));
    GetMetaData(*meta_, INT_TESTKEY, valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_Enum_As_Int32_Using_Parcel
 * @tc.desc: SetGet_MetaData_Enum_As_Int32_Using_Parcel
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_Enum_As_Int32_Using_Parcel, TestSize.Level1)
{
    MessageParcel parcel;
    int32_t valueOut = 0;
    int32_t valueIn = static_cast<int32_t>(Plugin::VideoRotation::VIDEO_ROTATION_90);
    meta_ = std::make_shared<Meta>();
    SetMetaData(*meta_, INT_ENUM_TESTKEY, valueIn);
    ASSERT_TRUE(meta_->ToParcel(parcel));
    ASSERT_TRUE(meta_->FromParcel(parcel));
    GetMetaData(*meta_, INT_ENUM_TESTKEY, valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_Enum_As_Int64_Using_Parcel
 * @tc.desc: SetGet_MetaData_Enum_As_Int64_Using_Parcel
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_Enum_As_Int64_Using_Parcel, TestSize.Level1)
{
    MessageParcel parcel;
    int64_t valueOut = 0;
    int64_t valueIn = static_cast<int64_t>(Plugin::AudioChannelLayout::HOA_ORDER1_FUMA);
    meta_ = std::make_shared<Meta>();
    SetMetaData(*meta_, LONG_ENUM_TESTKEY, valueIn);
    ASSERT_TRUE(meta_->ToParcel(parcel));
    ASSERT_TRUE(meta_->FromParcel(parcel));
    GetMetaData(*meta_, LONG_ENUM_TESTKEY, valueOut);
    EXPECT_EQ(valueOut, valueIn);
}

/**
 * @tc.name: SetGet_MetaData_Enum_As_Int32
 * @tc.desc: SetGet_MetaData_Enum_As_Int32
 * @tc.type: FUNC
 */
HWTEST_F(MetaInnerUnitTest, SetGet_MetaData_Enum_As_Int32, TestSize.Level1)
{
    MessageParcel parcel;
    int32_t valueOut = 0;
    int32_t valueIn = static_cast<int32_t>(Plugin::VideoRotation::VIDEO_ROTATION_90);
    meta_ = std::make_shared<Meta>();
    SetMetaData(*meta_, INT_ENUM_TESTKEY, valueIn);
    GetMetaData(*meta_, INT_ENUM_TESTKEY, valueOut);
    EXPECT_EQ(valueOut, valueIn);
}
} // namespace MetaFuncUT
} // namespace Media
} // namespace OHOS
