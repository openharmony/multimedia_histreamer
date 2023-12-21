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
#include <string>
#include "meta/any.h"
#include "meta/source_types.h"
#include "unittest_log.h"
#include <cstdlib>

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;

namespace OHOS {
namespace Media {
namespace AnyFuncUT {
class AnyInnerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp(void);

    void TearDown(void);
};

void AnyInnerUnitTest::SetUpTestCase(void) {}

void AnyInnerUnitTest::TearDownTestCase(void) {}

void AnyInnerUnitTest::SetUp(void)
{
    std::cout << "[SetUp]: SetUp!!!, test: ";
    const ::testing::TestInfo *testInfo_ = ::testing::UnitTest::GetInstance()->current_test_info();
    std::string testName = testInfo_->name();
    std::cout << testName << std::endl;
}

void AnyInnerUnitTest::TearDown(void)
{
    std::cout << "[TearDown]: over!!!" << std::endl;
}

/**
 * @tc.name: Any_Init
 * @tc.desc: Any_Init
 * @tc.type: FUNC
 */
HWTEST_F(AnyInnerUnitTest, Any_Init, TestSize.Level1)
{
    Any anyInit = 123;
    Any anyCopy(anyInit);
    int32_t valueIn = AnyCast<int32_t>(anyInit);
    int32_t valueOut = AnyCast<int32_t>(anyCopy);
    EXPECT_EQ(valueOut, valueIn);

    Any anyInitValueType(Plugins::SrcInputType::AUD_MIC);
    Plugins::SrcInputType valueOutSrcType = AnyCast<Plugins::SrcInputType>(anyInitValueType);
    EXPECT_EQ(static_cast<int32_t>(valueOutSrcType), 1);
}

/**
 * @tc.name: Any_Move
 * @tc.desc: Any_Move
 * @tc.type: FUNC
 */
HWTEST_F(AnyInnerUnitTest, Any_Move, TestSize.Level1)
{
    Any anyInit = 124;
    Any anyMove(std::move(anyInit));
    int32_t valueOut = AnyCast<int32_t>(anyMove);
    ASSERT_FALSE(anyInit.HasValue());
    EXPECT_EQ(valueOut, 124);
}

/**
 * @tc.name: Any_Copy
 * @tc.desc: Any_Copy
 * @tc.type: FUNC
 */
HWTEST_F(AnyInnerUnitTest, Any_Copy, TestSize.Level1)
{
    Any anyInit = 125;
    Any anyCopy(anyInit);
    int32_t valueOut = AnyCast<int32_t>(anyInit);
    int32_t valueOutCopy = AnyCast<int32_t>(anyInit);
    ASSERT_TRUE(anyInit.HasValue());
    EXPECT_EQ(valueOut, 125);
    EXPECT_EQ(valueOutCopy, 125);
}

/**
 * @tc.name: Any_Swap
 * @tc.desc: Any_Swap
 * @tc.type: FUNC
 */
HWTEST_F(AnyInnerUnitTest, Any_Swap, TestSize.Level1)
{
    Any anyFirst = 125;
    Any anySecond = 126;
    anyFirst.Swap(anySecond);
    int32_t valueOutFirst = AnyCast<int32_t>(anyFirst);
    int32_t valueOutSecond = AnyCast<int32_t>(anySecond);
    EXPECT_EQ(valueOutSecond, 125);
    EXPECT_EQ(valueOutFirst, 126);
    swap(anyFirst, anySecond);
    valueOutFirst = AnyCast<int32_t>(anyFirst);
    valueOutSecond = AnyCast<int32_t>(anySecond);
    EXPECT_EQ(valueOutSecond, 126);
    EXPECT_EQ(valueOutFirst, 125);
}

/**
 * @tc.name: Any_Cast
 * @tc.desc: Any_Cast
 * @tc.type: FUNC
 */
HWTEST_F(AnyInnerUnitTest, Any_Cast, TestSize.Level1)
{
    Any anyInt32 = 125;
    int32_t valueOutInt32 = AnyCast<int32_t>(anyInt32);
    EXPECT_EQ(valueOutInt32, 125);
    Any anyInt64 = (int64_t)125000000001111L;
    int64_t valueOutInt64 = AnyCast<int64_t>(anyInt64);
    EXPECT_EQ(valueOutInt64, 125000000001111L);
    Any anyBool = true;
    bool valueOutbool = AnyCast<bool>(anyBool);
    EXPECT_EQ(valueOutbool, true);
    Any anyFloat = static_cast<float>(1.1);
    float valueOutFloat = AnyCast<float>(anyFloat);
    EXPECT_EQ(valueOutFloat, static_cast<float>(1.1));
    Any anyDouble = static_cast<double>(1.2533);
    double valueOutDouble = AnyCast<double>(anyDouble);
    ASSERT_DOUBLE_EQ(valueOutDouble, static_cast<double>(1.2533));
    std::string str = "test string";
    Any anyString = str;
    std::string valueOutString = AnyCast<std::string>(anyString);
    EXPECT_EQ(valueOutString, str);
    std::vector<uint8_t> vecUint8{1, 2, 3};
    Any anyVecUInt8 = vecUint8;
    std::vector<uint8_t> valueOutVecUInt8 = AnyCast<std::vector<uint8_t>>(anyVecUInt8);
    EXPECT_EQ(valueOutVecUInt8, vecUint8);
}

/**
* @tc.name: Any_IsSameTypeWith
* @tc.desc: Any_IsSameTypeWith
* @tc.type: FUNC
*/
HWTEST_F(AnyInnerUnitTest, Any_IsSameTypeWith, TestSize.Level1)
{
    Any anyInt32 = 125;
    std::string_view strIntTypeName = anyInt32.GetTypeName<int32_t>();
    std::string_view strInt = std::string_view("int");
    EXPECT_EQ(strIntTypeName, strInt);
    ASSERT_TRUE(Any::IsSameTypeWith<int32_t>(anyInt32));

    Any anyInt64 = (int64_t)125000000001111L;
    ASSERT_TRUE(Any::IsSameTypeWith<int64_t>(anyInt64));

    Any anyBool = true;
    ASSERT_TRUE(Any::IsSameTypeWith<bool>(anyBool));
    std::string_view strBoolTypeName = anyBool.GetTypeName<bool>();
    std::string_view strBool = std::string_view("bool");
    EXPECT_EQ(strBoolTypeName, strBool);

    Any anyFloat = (float)1.1;
    ASSERT_TRUE(Any::IsSameTypeWith<float>(anyFloat));
    std::string_view strFloatTypeName = anyFloat.GetTypeName<float>();
    std::string_view strFloat = std::string_view("float");
    EXPECT_EQ(strFloatTypeName, strFloat);

    Any anyDouble = (double)1.2533;
    ASSERT_TRUE(Any::IsSameTypeWith<double>(anyDouble));
    std::string_view strDoubleTypeName = anyDouble.GetTypeName<double>();
    std::string_view strDouble = std::string_view("double");
    EXPECT_EQ(strDoubleTypeName, strDouble);

    std::string str = "test string";
    Any anyString = str;
    ASSERT_TRUE(Any::IsSameTypeWith<std::string>(anyString));

    std::vector<uint8_t> vecUint8{1, 2, 3};
    Any anyVecUInt8 = vecUint8;
    std::string_view strVecUint8TypeName = anyVecUInt8.GetTypeName<std::vector<uint8_t>>();
    std::string_view strVecUint8 = std::string_view("std::vector<unsigned char>");
    EXPECT_EQ(strVecUint8TypeName, strVecUint8);
    ASSERT_TRUE(Any::IsSameTypeWith<std::vector<uint8_t>>(anyVecUInt8));
}

/**
 * @tc.name: Any_Parcel
 * @tc.desc: Any_Parcel
 * @tc.type: FUNC
 */
HWTEST_F(AnyInnerUnitTest, Any_Parcel, TestSize.Level1)
{
    MessageParcel metaParcel;
    Any anyInt = 125;
    anyInt.ToParcel(metaParcel);
    Any anyIntParcel(0);
    anyIntParcel.FromParcel(metaParcel);
    int32_t valueOutNew = AnyCast<int32_t>(anyIntParcel);
    EXPECT_EQ(valueOutNew, 125);
}
} // namespace AnyFuncUT
} // namespace Media
} // namespace OHOS
