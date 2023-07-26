/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "plugin/plugins/ffmpeg_adapter/utils/avc_config_data_parser.h"

using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin::Ffmpeg;

HWTEST(FFmpegAvcConfigDataParserTest, testParseConfigData, TestSize.Level1)
{
    const uint8_t* ptr1 = nullptr;
    auto parser1 = std::make_shared<AVCConfigDataParser>(ptr1, 0);
    bool res1 = parser1->ParseConfigData();
    ASSERT_FALSE(res1);

    uint8_t nums[9] = {1, 2, 4, 16, 91, 128, 160, 232, 255};
    for (size_t i = 0; i < 9; i++) {
        uint8_t num = nums[i];
        const uint8_t* ptr = &num;
        size_t size = sizeof(num);
        auto parser = std::make_shared<AVCConfigDataParser>(ptr, size);
        bool res = parser->ParseConfigData();
        if (num == 232 || num == 160 || num == 128) {
            ASSERT_FALSE(res);
        } else {
            ASSERT_TRUE(res);
        }
    }
}

HWTEST(FFmpegAvcConfigDataParserTest, testIsNeedAddFrameHeader, TestSize.Level1)
{
    uint8_t num = 10;
    const uint8_t* ptr1 = &num;
    auto parser = std::make_shared<AVCConfigDataParser>(ptr1, 1);
    bool res = parser->IsNeedAddFrameHeader();
    ASSERT_FALSE(res);
}

HWTEST(FFmpegAvcConfigDataParserTest, testGetNewConfigData, TestSize.Level1)
{
    const uint8_t* ptr1 = nullptr;
    auto parser1 = std::make_shared<AVCConfigDataParser>(ptr1, 0);
    std::shared_ptr<uint8_t> newCfgData;
    size_t newCfgDataSize;
    bool res1 = parser1->GetNewConfigData(newCfgData, newCfgDataSize);
    ASSERT_FALSE(res1);

    uint8_t num = 160;
    const uint8_t* ptr2 = &num;
    auto parser2 = std::make_shared<AVCConfigDataParser>(ptr2, 3);
    std::shared_ptr<uint8_t> data;
    size_t size;
    bool res2 = parser2->GetNewConfigData(data, size);
    ASSERT_FALSE(res2);
}

} // namespace Test
} // namespace Media
} // namespace OHOS