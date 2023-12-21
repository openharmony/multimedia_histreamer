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
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include "common/status.h"
#include "meta/format.h"
#include "meta/meta.h"
#include "securec.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
namespace {

constexpr float EPSINON_FLOAT = 0.0001;
constexpr double EPSINON_DOUBLE = 0.0001;

constexpr int32_t INT_VALUE = 124;
constexpr int64_t LONG_VALUE = 12435;
constexpr double DOUBLE_VALUE = 666.625;
const std::string STRING_VALUE = "STRING_VALUE";
const std::vector<uint8_t> BUFFER_VALUE{1, 2, 3, 4, 255, 0, 255};

#define INT_TESTKEY Tag::APP_PID
#define INT_ENUM_TESTKEY Tag::VIDEO_ROTATION
#define LONG_TESTKEY Tag::MEDIA_DURATION
#define LONG_ENUM_TESTKEY Tag::AUDIO_CHANNEL_LAYOUT
#define DOUBLE_TESTKEY Tag::VIDEO_CAPTURE_RATE
#define STRING_TESTKEY Tag::MEDIA_FILE_URI
#define BUFFER_TESTKEY Tag::MEDIA_COVER
} // namespace

namespace OHOS {
namespace Media {

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
HWTEST_F(AVFormatUnitTest, Format_Value_001, TestSize.Level1)
{
    const std::string_view floatKey = "FloatKey";
    float floatValue = 1.0;

    int32_t getIntValue = 0;
    int64_t getLongValue = 0;
    float getFloatValue = 0.0;
    double getDoubleValue = 0.0;
    std::string getStringValue = "";

    EXPECT_TRUE(format_->PutIntValue(INT_TESTKEY, INT_VALUE));
    EXPECT_TRUE(format_->GetIntValue(INT_TESTKEY, getIntValue));
    EXPECT_TRUE(getIntValue == INT_VALUE);
    EXPECT_FALSE(format_->GetLongValue(INT_TESTKEY, getLongValue));

    EXPECT_TRUE(format_->PutLongValue(LONG_TESTKEY, LONG_VALUE));
    EXPECT_TRUE(format_->GetLongValue(LONG_TESTKEY, getLongValue));
    EXPECT_TRUE(getLongValue == LONG_VALUE);
    EXPECT_FALSE(format_->GetIntValue(LONG_TESTKEY, getIntValue));

    EXPECT_TRUE(format_->PutFloatValue(floatKey, floatValue));
    EXPECT_TRUE(format_->GetFloatValue(floatKey, getFloatValue));
    EXPECT_TRUE(fabs(floatValue - getFloatValue) < EPSINON_FLOAT);
    EXPECT_FALSE(format_->GetDoubleValue(floatKey, getDoubleValue));

    EXPECT_TRUE(format_->PutDoubleValue(DOUBLE_TESTKEY, DOUBLE_VALUE));
    EXPECT_TRUE(format_->GetDoubleValue(DOUBLE_TESTKEY, getDoubleValue));
    EXPECT_TRUE(fabs(DOUBLE_VALUE - getDoubleValue) < EPSINON_DOUBLE);
    EXPECT_FALSE(format_->GetFloatValue(DOUBLE_TESTKEY, getFloatValue));

    EXPECT_TRUE(format_->PutStringValue(STRING_TESTKEY, STRING_VALUE.c_str()));
    EXPECT_TRUE(format_->GetStringValue(STRING_TESTKEY, getStringValue));
    EXPECT_TRUE(STRING_VALUE == getStringValue);
}

/**
 * @tc.name: Format_Buffer_001
 * @tc.desc: format put and get buffer
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_Buffer_001, TestSize.Level1)
{
    constexpr size_t size = 3;
    const std::string_view key = "BufferKey";
    uint8_t buffer[size] = {'a', 'b', 'b'};

    EXPECT_TRUE(format_->PutBuffer(key, buffer, size));
    uint8_t *getBuffer;
    size_t getSize;
    EXPECT_TRUE(format_->GetBuffer(key, &getBuffer, getSize));
    EXPECT_TRUE(getSize == size);
    for (size_t i = 0; i < size; i++) {
        EXPECT_TRUE(buffer[i] == getBuffer[i]);
    }

    std::string getString;
    EXPECT_FALSE(format_->GetStringValue(key, getString));
}

/**
 * @tc.name: Format_DumpInfo_001
 * @tc.desc:
 *     1. set format;
 *     2. dmpinfo;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_DumpInfo_001, TestSize.Level1)
{
    const std::string_view floatKey = "FloatKey";
    float floatValue = 1.0;

    EXPECT_TRUE(format_->PutIntValue(INT_TESTKEY, INT_VALUE));
    EXPECT_TRUE(format_->PutLongValue(LONG_TESTKEY, LONG_VALUE));
    EXPECT_TRUE(format_->PutFloatValue(floatKey, floatValue));
    EXPECT_TRUE(format_->PutDoubleValue(DOUBLE_TESTKEY, DOUBLE_VALUE));
    EXPECT_TRUE(format_->PutStringValue(STRING_TESTKEY, STRING_VALUE.c_str()));

    std::string dumpInfo = format_->DumpInfo();
    std::cout << "dumpInfo: [" << dumpInfo << "]\n";

    std::stringstream dumpStream;
    dumpStream << floatKey << " = " << floatValue;
    EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                             << "dumpInfo: [" << dumpInfo << "]\n";

    dumpStream.str("");
    dumpStream << DOUBLE_TESTKEY << " = " << DOUBLE_VALUE;
    EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                             << "dumpInfo: [" << dumpInfo << "]\n";

    dumpStream.str("");
    dumpStream << INT_TESTKEY << " = " << INT_VALUE;
    EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                             << "dumpInfo: [" << dumpInfo << "]\n";

    dumpStream.str("");
    dumpStream << LONG_TESTKEY << " = " << LONG_VALUE;
    EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                             << "dumpInfo: [" << dumpInfo << "]\n";

    dumpStream.str("");
    dumpStream << STRING_TESTKEY << " = " << STRING_VALUE;
    EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                             << "dumpInfo: [" << dumpInfo << "]\n";
}

#ifndef AVFORMAT_CAPI_UNIT_TEST
/**
 * @tc.name: Format_DumpInfo_002
 * @tc.desc:
 *     1. set format;
 *     2. meta trans by parcel;
 *     3. dmpinfo;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_DumpInfo_002, TestSize.Level1)
{
    std::shared_ptr<Format> format = std::make_shared<Format>();
    EXPECT_TRUE(format->PutIntValue(INT_TESTKEY, INT_VALUE));
    EXPECT_TRUE(format->PutLongValue(LONG_TESTKEY, LONG_VALUE));
    EXPECT_TRUE(format->PutDoubleValue(DOUBLE_TESTKEY, DOUBLE_VALUE));
    EXPECT_TRUE(format->PutStringValue(STRING_TESTKEY, STRING_VALUE.c_str()));

    std::string dumpInfo = format->Stringify();
    std::cout << "dumpInfo: [" << dumpInfo << "]\n";
    std::stringstream dumpStream;
    auto checkFunc = [&dumpStream, &dumpInfo]() {
        dumpStream.str("");
        dumpStream << DOUBLE_TESTKEY << " = " << DOUBLE_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << INT_TESTKEY << " = " << INT_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << LONG_TESTKEY << " = " << LONG_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << STRING_TESTKEY << " = " << STRING_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";
    };
    std::cout << "before trans by parcel:\n";
    checkFunc();

    MessageParcel parcel;
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();

    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format->SetMeta(std::move(meta));
    dumpInfo = format->Stringify();
    std::cout << "after trans by parcel:\n";
    checkFunc();
}

/**
 * @tc.name: Format_DumpInfo_003
 * @tc.desc:
 *     1. set meta to format;
 *     2. meta trans by parcel;
 *     3. dmpinfo;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_DumpInfo_003, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    meta->SetData(INT_TESTKEY, INT_VALUE);
    meta->SetData(LONG_TESTKEY, LONG_VALUE);
    meta->SetData(DOUBLE_TESTKEY, DOUBLE_VALUE);
    meta->SetData(STRING_TESTKEY, STRING_VALUE);

    format->SetMeta(meta);
    std::string dumpInfo = format->Stringify();
    std::cout << "dumpInfo: [" << dumpInfo << "]\n";
    std::stringstream dumpStream;
    auto checkFunc = [&dumpStream, &dumpInfo]() {
        dumpStream.str("");
        dumpStream << DOUBLE_TESTKEY << " = " << DOUBLE_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << INT_TESTKEY << " = " << INT_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << LONG_TESTKEY << " = " << LONG_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << STRING_TESTKEY << " = " << STRING_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";
    };
    std::cout << "before trans by parcel:\n";
    checkFunc();

    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format = std::make_shared<Format>();
    format->SetMeta(std::move(meta));
    dumpInfo = format->Stringify();
    std::cout << "after trans by parcel:\n";
    checkFunc();
}

/**
 * @tc.name: Format_DumpInfo_004
 * @tc.desc:
 *     1. set buffer to format;
 *     2. meta trans by parcel;
 *     3. dmpinfo;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_DumpInfo_004, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();

    format->PutBuffer(BUFFER_TESTKEY, BUFFER_VALUE.data(), BUFFER_VALUE.size());
    std::string dumpInfo = format->Stringify();
    std::cout << "dumpInfo: [" << dumpInfo << "]\n";
    std::stringstream dumpStream;
    auto checkFunc = [&dumpStream, &dumpInfo]() {
        dumpStream.str("");
        dumpStream << BUFFER_TESTKEY << ", bufferSize = " << BUFFER_VALUE.size();
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";
    };
    std::cout << "before trans by parcel:\n";
    checkFunc();

    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format = std::make_shared<Format>();
    format->SetMeta(std::move(meta));
    dumpInfo = format->Stringify();
    std::cout << "after trans by parcel:\n";
    checkFunc();
}

/**
 * @tc.name: Format_DumpInfo_005
 * @tc.desc:
 *     1. set buffer to meta;
 *     2. meta trans by parcel;
 *     3. dmpinfo;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_DumpInfo_005, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    meta->SetData(BUFFER_TESTKEY, BUFFER_VALUE);
    format->SetMeta(meta);

    std::string dumpInfo = format->Stringify();
    std::cout << "dumpInfo: [" << dumpInfo << "]\n";
    std::stringstream dumpStream;
    auto checkFunc = [&dumpStream, &dumpInfo]() {
        dumpStream.str("");
        dumpStream << BUFFER_TESTKEY << ", bufferSize = " << BUFFER_VALUE.size();
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";
    };
    std::cout << "before trans by parcel:\n";
    checkFunc();

    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format = std::make_shared<Format>();
    format->SetMeta(std::move(meta));
    dumpInfo = format->Stringify();
    std::cout << "after trans by parcel:\n";
    checkFunc();
}

/**
 * @tc.name: Format_DumpInfo_006
 * @tc.desc:
 *     1. set enum to format;
 *     2. meta trans by parcel;
 *     3. dmpinfo;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_DumpInfo_006, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    EXPECT_TRUE(format->PutIntValue(INT_ENUM_TESTKEY, INT_VALUE));
    EXPECT_TRUE(format->PutLongValue(LONG_ENUM_TESTKEY, LONG_VALUE));

    std::string dumpInfo = format->Stringify();
    std::cout << "dumpInfo: [" << dumpInfo << "]\n";
    std::stringstream dumpStream;
    auto checkFunc = [&dumpStream, &dumpInfo]() {
        dumpStream.str("");
        dumpStream << INT_ENUM_TESTKEY << " = " << INT_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << LONG_ENUM_TESTKEY << " = " << LONG_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";
    };
    std::cout << "before trans by parcel:\n";
    checkFunc();

    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format = std::make_shared<Format>();
    format->SetMeta(std::move(meta));
    dumpInfo = format->Stringify();
    std::cout << "after trans by parcel:\n";
    checkFunc();
}

/**
 * @tc.name: Format_DumpInfo_007
 * @tc.desc:
 *     1. set enum to meta;
 *     2. meta trans by parcel;
 *     3. dmpinfo;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_DumpInfo_007, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    meta->SetData(INT_ENUM_TESTKEY, INT_VALUE);
    meta->SetData(LONG_ENUM_TESTKEY, LONG_VALUE);

    format->SetMeta(meta);
    std::string dumpInfo = format->Stringify();
    std::cout << "dumpInfo: [" << dumpInfo << "]\n";
    std::stringstream dumpStream;
    auto checkFunc = [&dumpStream, &dumpInfo]() {
        dumpStream.str("");
        dumpStream << INT_ENUM_TESTKEY << " = " << INT_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";

        dumpStream.str("");
        dumpStream << LONG_ENUM_TESTKEY << " = " << LONG_VALUE;
        EXPECT_NE(dumpInfo.find(dumpStream.str()), string::npos) << "dumpStream: [" << dumpStream.str() << "]\n"
                                                                 << "dumpInfo: [" << dumpInfo << "]\n";
    };
    std::cout << "before trans by parcel:\n";
    checkFunc();

    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format = std::make_shared<Format>();
    format->SetMeta(std::move(meta));
    dumpInfo = format->Stringify();
    std::cout << "after trans by parcel:\n";
    checkFunc();
}

void CheckFormatMap(Format::FormatDataMap &formatMap)
{
    const std::string floatKey = "FloatKey";
    float floatValue = 1.0;
    auto iter = formatMap.find(INT_TESTKEY);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_INT32);
    EXPECT_EQ(iter->second.val.int32Val, INT_VALUE);

    iter = formatMap.find(LONG_TESTKEY);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_INT64);
    EXPECT_EQ(iter->second.val.int64Val, LONG_VALUE);

    iter = formatMap.find(floatKey);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_FLOAT);
    EXPECT_EQ(iter->second.val.floatVal, floatValue);

    iter = formatMap.find(DOUBLE_TESTKEY);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_DOUBLE);
    EXPECT_EQ(iter->second.val.doubleVal, DOUBLE_VALUE);

    iter = formatMap.find(STRING_TESTKEY);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_STRING);
    EXPECT_EQ(iter->second.stringVal, STRING_VALUE);

    iter = formatMap.find(BUFFER_TESTKEY);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_ADDR);
    EXPECT_EQ(iter->second.size, BUFFER_VALUE.size());
    for (size_t i = 0; i < iter->second.size; ++i) {
        EXPECT_EQ(iter->second.addr[i], BUFFER_VALUE[i]);
    }

    iter = formatMap.find(INT_ENUM_TESTKEY);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_INT32);
    EXPECT_EQ(iter->second.val.int32Val, static_cast<int32_t>(Plugins::VideoRotation::VIDEO_ROTATION_90));

    iter = formatMap.find(LONG_ENUM_TESTKEY);
    ASSERT_NE(iter, formatMap.end());
    EXPECT_EQ(iter->second.type, FORMAT_TYPE_INT64);
    EXPECT_EQ(iter->second.val.int64Val, static_cast<int64_t>(Plugins::AudioChannelLayout::STEREO));
}
/**
 * @tc.name: Format_GetFormatMap_001
 * @tc.desc:
 *     1. set values to meta;
 *     2. get formatMap;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_GetFormatMap_001, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    const std::string floatKey = "FloatKey";
    float floatValue = 1.0;

    meta->SetData(INT_TESTKEY, INT_VALUE);
    meta->SetData(LONG_TESTKEY, LONG_VALUE);
    meta->SetData(floatKey, floatValue);
    meta->SetData(DOUBLE_TESTKEY, DOUBLE_VALUE);
    meta->SetData(STRING_TESTKEY, STRING_VALUE);
    meta->SetData(BUFFER_TESTKEY, BUFFER_VALUE);
    meta->SetData(INT_ENUM_TESTKEY, Plugins::VideoRotation::VIDEO_ROTATION_90);
    meta->SetData(LONG_ENUM_TESTKEY, Plugins::AudioChannelLayout::STEREO);

    format->SetMeta(meta);
    Format::FormatDataMap formatMap = format->GetFormatMap();
    CheckFormatMap(formatMap);
}

/**
 * @tc.name: Format_GetFormatMap_002
 * @tc.desc:
 *     1. set values to meta;
 *     2. meta trans by parcel;
 *     3. get formatMap;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_GetFormatMap_002, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    const std::string floatKey = "FloatKey";
    float floatValue = 1.0;

    meta->SetData(INT_TESTKEY, INT_VALUE);
    meta->SetData(LONG_TESTKEY, LONG_VALUE);
    meta->SetData(floatKey, floatValue);
    meta->SetData(DOUBLE_TESTKEY, DOUBLE_VALUE);
    meta->SetData(STRING_TESTKEY, STRING_VALUE);
    meta->SetData(BUFFER_TESTKEY, BUFFER_VALUE);
    meta->SetData(INT_ENUM_TESTKEY, Plugins::VideoRotation::VIDEO_ROTATION_90);
    meta->SetData(LONG_ENUM_TESTKEY, Plugins::AudioChannelLayout::STEREO);

    format->SetMeta(meta);
    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format = std::make_shared<Format>();
    format->SetMeta(std::move(meta));
    Format::FormatDataMap formatMap = format->GetFormatMap();
    CheckFormatMap(formatMap);
}

void CheckValueType(std::shared_ptr<Format> &format)
{
    const std::string floatKey = "FloatKey";

    EXPECT_EQ(format->GetValueType(INT_TESTKEY), FORMAT_TYPE_INT32);

    EXPECT_EQ(format->GetValueType(LONG_TESTKEY), FORMAT_TYPE_INT64);

    EXPECT_EQ(format->GetValueType(floatKey), FORMAT_TYPE_FLOAT);

    EXPECT_EQ(format->GetValueType(DOUBLE_TESTKEY), FORMAT_TYPE_DOUBLE);

    EXPECT_EQ(format->GetValueType(STRING_TESTKEY), FORMAT_TYPE_STRING);

    EXPECT_EQ(format->GetValueType(BUFFER_TESTKEY), FORMAT_TYPE_ADDR);

    EXPECT_EQ(format->GetValueType(INT_ENUM_TESTKEY), FORMAT_TYPE_INT32);

    EXPECT_EQ(format->GetValueType(LONG_ENUM_TESTKEY), FORMAT_TYPE_INT64);
}

/**
 * @tc.name: Format_GetValueType_001
 * @tc.desc:
 *     1. set values to meta;
 *     2. get format value type;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_GetValueType_001, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    const std::string floatKey = "FloatKey";
    float floatValue = 1.0;

    meta->SetData(INT_TESTKEY, INT_VALUE);
    meta->SetData(LONG_TESTKEY, LONG_VALUE);
    meta->SetData(floatKey, floatValue);
    meta->SetData(DOUBLE_TESTKEY, DOUBLE_VALUE);
    meta->SetData(STRING_TESTKEY, STRING_VALUE);
    meta->SetData(BUFFER_TESTKEY, BUFFER_VALUE);
    meta->SetData(INT_ENUM_TESTKEY, Plugins::VideoRotation::VIDEO_ROTATION_90);
    meta->SetData(LONG_ENUM_TESTKEY, Plugins::AudioChannelLayout::STEREO);

    format->SetMeta(std::move(meta));
    CheckValueType(format);
}

/**
 * @tc.name: Format_GetValueType_002
 * @tc.desc:
 *     1. set values to meta;
 *     2. meta trans by parcel;
 *     3. get format value type;
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(AVFormatUnitTest, Format_GetValueType_002, TestSize.Level1)
{
    MessageParcel parcel;
    std::shared_ptr<Format> format = std::make_shared<Format>();
    std::shared_ptr<Meta> meta = std::make_shared<Meta>();
    const std::string floatKey = "FloatKey";
    float floatValue = 1.0;

    meta->SetData(INT_TESTKEY, INT_VALUE);
    meta->SetData(LONG_TESTKEY, LONG_VALUE);
    meta->SetData(floatKey, floatValue);
    meta->SetData(DOUBLE_TESTKEY, DOUBLE_VALUE);
    meta->SetData(STRING_TESTKEY, STRING_VALUE);
    meta->SetData(BUFFER_TESTKEY, BUFFER_VALUE);
    meta->SetData(INT_ENUM_TESTKEY, Plugins::VideoRotation::VIDEO_ROTATION_90);
    meta->SetData(LONG_ENUM_TESTKEY, Plugins::AudioChannelLayout::STEREO);

    format->SetMeta(meta);
    meta = format->GetMeta();
    ASSERT_TRUE(meta->ToParcel(parcel));
    ASSERT_TRUE(meta->FromParcel(parcel));

    format = std::make_shared<Format>();
    format->SetMeta(std::move(meta));
    CheckValueType(format);
}
#endif
} // namespace Media
} // namespace OHOS