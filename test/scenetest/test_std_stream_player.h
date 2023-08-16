/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_LITE

#include <thread>
#include <chrono>
#include "helper/test_player.hpp"
#include "plugin/common/plugin_types.h"
#include "testngpp/testngpp.hpp"

using namespace OHOS::Media::Test;

// @fixture(tags=stdstream)
FIXTURE(TestStdStreamPlayerFast)
{
    DATA_PROVIDER(musicUrls, 1,
    DATA_GROUP(std::string(RESOURCE_DIR "/MP3/mp3_stream")));

    // @test(data="musicUrls")
    PTEST((std::string url), Test single player play std stream seekable, and finished automatically)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        auto testsrc = TestSource::CreateTestSource(url, TestSourceType::STREAM,
                                                    OHOS::Media::Plugin::Seekable::SEEKABLE);
        ASSERT_EQ(0, player->SetSource(testsrc));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    // @test(data="musicUrls")
    PTEST((std::string url), Test single player play std stream unseekable, and finished automatically)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        auto testsrc = TestSource::CreateTestSource(url, TestSourceType::STREAM,
                                                    OHOS::Media::Plugin::Seekable::UNSEEKABLE);
        ASSERT_EQ(0, player->SetSource(testsrc));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};
#endif
