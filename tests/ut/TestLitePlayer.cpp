/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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
#include "scene/lite/hiplayer.h"

namespace OHOS::Media::Test {
class UtTestLitePlayer : public ::testing::Test {
public:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
    }
};

bool g_playFinished = false;

class PlayerCallbackImpl : public PlayerCallback {
    void OnPlaybackComplete() override
    {
        g_playFinished = true;
    }

    void OnError(int32_t errorType, int32_t errorCode) override
    {
    }

    void OnInfo(int type, int extra) override
    {
    }

    void OnVideoSizeChanged(int width, int height) override
    {
    }

    void OnRewindToComplete() override
    {
    }
};

void StartPlayer(std::string url)
{
    std::cout << "Use media_lite interface player." << std::endl;
    g_playFinished = false;
    auto player = OHOS::Media::CreateHiPlayer();
    player->Init();
    auto callback = std::make_shared<PlayerCallbackImpl>();
    player->SetPlayerCallback(callback);
    OHOS::Media::Source source(url);
    player->SetSource(source);
    player->SetLoop(false);
    player->Prepare();
    player->Play();
    while (!g_playFinished) {
        //OSAL::SleepFor(50);
    }
}

TEST_F(UtTestLitePlayer, Can_play_local_music_two_times)
{
    StartPlayer("../../resource/short_music.mp3");
    StartPlayer("../../resource/short_music.mp3");
}
}

