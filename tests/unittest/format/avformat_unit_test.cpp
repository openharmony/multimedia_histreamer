/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "avformat_unit_test.h"
#include <cmath>
#include "gtest/gtest.h"
#include "avcodec_errors.h"
#include "securec.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::MediaAVCodec;
using namespace testing::ext;

constexpr float EPSINON_FLOAT = 0.0001;
constexpr double EPSINON_DOUBLE = 0.0001;

void AVFormatUnitTest::SetUpTestCase(void) {}

void AVFormatUnitTest::TearDownTestCase(void) {}

void AVFormatUnitTest::SetUp(void)
{
    format_ = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format_);
}

void AVFormatUnitTest::TearDown(void)
{
    if (format_ != nullptr) {
        format_->Destroy();
    }
}

/**
 * @tc.name: Format_Value_001
 * @tc.desc: format put and get value
 * @tc.type: FUNC
 * @tc.require: issueI5OX06 issueI5P8N0
 */
HWTEST_F(AVFormatUnitTest, Format_Value_001, TestSize.Level0)
{
    const std::string_view intKey = "IntKey";
    const std::string_view longKey = "LongKey";
    const std::string_view floatKey = "FloatKey";
    const std::string_view doubleKey = "DoubleKey";
    const std::string_view stringKey = "StringKey";

    int32_t intValue = 1;
    int64_t longValue = 1;
    float floatValue = 1.0;
    double doubleValue = 1.0;
    const std::string stringValue = "StringValue";

    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    float getFloatValue = 0.0;
    double getDoubleValue = 0.0;
    std::string getStringValue = "";

    EXPECT_TRUE(format_->PutIntValue(intKey, intValue));
    EXPECT_TRUE(format_->GetIntValue(intKey, getIntValue));
    EXPECT_TRUE(intValue == getIntValue);
    EXPECT_FALSE(format_->GetLongValue(intKey, getLongValue));

    EXPECT_TRUE(format_->PutLongValue(longKey, intValue));
    EXPECT_TRUE(format_->GetLongValue(longKey, getLongValue));
    EXPECT_TRUE(longValue == getLongValue);
    EXPECT_FALSE(format_->GetIntValue(longKey, getIntValue));

    EXPECT_TRUE(format_->PutFloatValue(floatKey, floatValue));
    EXPECT_TRUE(format_->GetFloatValue(floatKey, getFloatValue));
    EXPECT_TRUE(fabs(floatValue - getFloatValue) < EPSINON_FLOAT);
    EXPECT_FALSE(format_->GetDoubleValue(floatKey, getDoubleValue));

    EXPECT_TRUE(format_->PutDoubleValue(doubleKey, doubleValue));
    EXPECT_TRUE(format_->GetDoubleValue(doubleKey, getDoubleValue));
    EXPECT_TRUE(fabs(doubleValue - getDoubleValue) < EPSINON_DOUBLE);
    EXPECT_FALSE(format_->GetFloatValue(doubleKey, getFloatValue));

    EXPECT_TRUE(format_->PutStringValue(stringKey, stringValue.c_str()));
    EXPECT_TRUE(format_->GetStringValue(stringKey, getStringValue));
    EXPECT_TRUE(stringValue == getStringValue);
}

/**
 * @tc.name: Format_Buffer_001
 * @tc.desc: format put and get buffer
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_Buffer_001, TestSize.Level0)
{
    constexpr size_t size = 3;
    const std::string_view key = "BufferKey";
    uint8_t buffer[size] = {'a', 'b', 'b'};

    EXPECT_TRUE(format_->PutBuffer(key, buffer, size));
    uint8_t *getBuffer;
    size_t getSize;
    EXPECT_TRUE(format_->GetBuffer(key, &getBuffer, getSize));
    EXPECT_TRUE(size == getSize);
    for (int32_t i = 0; i < size; i++) {
        EXPECT_TRUE(buffer[i] == getBuffer[i]);
    }

    std::string getString;
    EXPECT_FALSE(format_->GetStringValue(key, getString));
}