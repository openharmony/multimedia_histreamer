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
#include <chrono>
#include <thread>
#include "helper/test_player.hpp"
#include "testngpp/testngpp.hpp"

using namespace OHOS::Media::Test;


FIXTURE(DataDrivenSingleVideoPlayerTestFast)
{
    DATA_PROVIDER(myurls, 1,
    DATA_GROUP(std::string(RESOURCE_DIR "/MP4/9_AVC_1280x720_59.940fps_AAC_128Kbps_2channels.mp4")));

    // @test(data="myurls", tags=video_play_fast)
    PTEST((std::string url), Test single player play url video, and finished automatically)
    {
        std::unique_ptr<TestPlayer> player = TestPlayer::Create();
        ASSERT_EQ(0, player->SetSource(TestSource(url)));
        ASSERT_EQ(0, player->Prepare());
        ASSERT_EQ(0, player->Play());
        while (player->IsPlaying()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};
