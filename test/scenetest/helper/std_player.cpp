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
#include "test_player.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <scene/player/standard/hiplayer_impl.h>

#include "foundation/log.h"
#include "media_errors.h"

using namespace OHOS::Media;

namespace OHOS::Media::Test {
bool g_playFinished = false;
bool g_seekFinished = false;

class PlayerCallbackImpl : public IPlayerEngineObs {
public:
    void SetPlayer(IPlayerEngine* player)
    {
        player_ = player;
    }

    void OnError(PlayerErrorType errorType, int32_t errorCode) override
    {
        if (errorCode == MSERR_SEEK_FAILED) {
            g_seekFinished = true;
        }
    }
    void OnInfo(PlayerOnInfoType type, int32_t extra, const Format& infoBody) override
    {
        if (type == INFO_TYPE_EOS && !g_playFinished) {
            player_->Seek(0, PlayerSeekMode::SEEK_PREVIOUS_SYNC);
        }
        if (type == INFO_TYPE_STATE_CHANGE && extra == PLAYER_PLAYBACK_COMPLETE) {
            g_playFinished = true;
        }
        if (type == INFO_TYPE_STATE_CHANGE && extra == PLAYER_STATE_ERROR) {
            g_playFinished = true;
        }
        if (type == INFO_TYPE_SEEKDONE) {
            g_seekFinished = true;
        }
    }
private:
    IPlayerEngine* player_ {nullptr};
};
std::shared_ptr<IPlayerEngineObs> gCallback = std::make_shared<PlayerCallbackImpl>();

class TestPlayerImpl : public TestPlayer {
public:
    explicit TestPlayerImpl(std::unique_ptr<IPlayerEngine> player) : player_(std::move(player)) {}
    int32_t SetSource(const TestSource& source) override;
    int32_t SetSingleLoop(bool loop) override;
    bool IsPlaying() override;
    int32_t Prepare() override;
    int32_t Play() override;
    int32_t Pause() override;
    int32_t Stop() override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t Seek(int64_t timeMs, PlayerSeekMode mode = PlayerSeekMode::SEEK_NEXT_SYNC) override;
    int32_t GetCurrentTime(int64_t& currentMs) override;
    int32_t GetDuration(int64_t& durationMs) override;
    int32_t SetVolume(float leftVolume, float rightVolume) override;
    int32_t GetAudioTrackInfo(std::vector<Format> &audioTrack) override;
    int32_t GetVideoTrackInfo(std::vector<Format> &videoTrack) override;
    int32_t SetPlaybackSpeed(PlaybackRateMode mode) override;
    int32_t GetPlaybackSpeed(PlaybackRateMode &mode) override;
private:
    std::unique_ptr<IPlayerEngine> player_;
    std::atomic<PlayerStates> pipelineStates_ {PlayerStates::PLAYER_IDLE};
};

TestSource TestSource::CreateTestSource(const std::string& url, TestSourceType type, Plugin::Seekable seekable)
{
    switch (type) {
        case TestSourceType::URI:
            return TestSource(url);
        case TestSourceType::STREAM:
            return TestSource(url, seekable);
    }
    return TestSource("");
}

std::unique_ptr<TestPlayer> TestPlayer::Create()
{
    auto player = std::make_unique<HiPlayerImpl>(0, 0);
    player -> Init();
    std::static_pointer_cast<PlayerCallbackImpl>(gCallback)->SetPlayer(player.get());
    player->SetObs(gCallback);
    g_playFinished = false;
    return std::make_unique<TestPlayerImpl>(std::move(player));
}

int32_t TestPlayerImpl::SetSource(const TestSource& source)
{
    int32_t ret = -1;
    if (source.type_ == TestSourceType::URI) {
        ret = player_->SetSource(source.url_);
    } else if (source.type_ == TestSourceType::STREAM) {
        auto src = std::make_shared<IMediaDataSourceImpl>(source.url_, source.seekable_);
        ret = player_->SetSource(src);
    }
    if (ret == 0) {
        pipelineStates_.store(PlayerStates::PLAYER_INITIALIZED);
    }
    return ret;
}

int32_t TestPlayerImpl::SetSingleLoop(bool loop)
{
    return player_->SetLooping(loop);
}

bool TestPlayerImpl::IsPlaying()
{
    if (g_playFinished) {
        pipelineStates_.store(PlayerStates::PLAYER_PLAYBACK_COMPLETE);
    }
    return !g_playFinished;
}

int32_t TestPlayerImpl::Prepare()
{
    int32_t ret = player_->Prepare();
    if (ret == 0) {
        pipelineStates_.store(PlayerStates::PLAYER_PREPARED);
    }
    return ret;
}

int32_t TestPlayerImpl::Play()
{
    if (pipelineStates_.load()  == PlayerStates::PLAYER_PREPARED ||
        pipelineStates_.load()  == PlayerStates::PLAYER_PLAYBACK_COMPLETE ||
        pipelineStates_.load()  == PlayerStates::PLAYER_PAUSED) {
        g_playFinished = false;
        int32_t ret = player_->Play();
        if (ret == 0) {
            pipelineStates_.store(PlayerStates::PLAYER_STARTED);
        }
        return ret;
    }
    return MSERR_INVALID_OPERATION;
}

int32_t TestPlayerImpl::Pause()
{
    if (pipelineStates_.load()  != PlayerStates::PLAYER_STARTED) {
        return MSERR_INVALID_OPERATION;
    }

    int32_t ret = player_->Pause();
    if (ret == 0) {
        pipelineStates_.store(PlayerStates::PLAYER_PAUSED);
    }
    return ret;
}

int32_t TestPlayerImpl::Stop()
{
    if (pipelineStates_.load() == PlayerStates::PLAYER_PREPARED ||
        pipelineStates_.load() == PlayerStates::PLAYER_STARTED ||
        pipelineStates_.load() == PlayerStates::PLAYER_PLAYBACK_COMPLETE ||
        pipelineStates_.load()== PlayerStates::PLAYER_PAUSED) {
        g_playFinished = true;
        int32_t ret = player_->Stop();
        if (ret == 0) {
            pipelineStates_.store(PlayerStates::PLAYER_STOPPED);
        }
        return ret;
    }
    return MSERR_INVALID_OPERATION;
}

int32_t TestPlayerImpl::Reset()
{
    if (pipelineStates_.load()  == PlayerStates::PLAYER_IDLE) {
        return MSERR_INVALID_OPERATION;
    }
    int32_t ret = player_->Reset();
    if (ret == 0) {
        pipelineStates_.store(PlayerStates::PLAYER_IDLE);
    }
    return ret;
}

int32_t TestPlayerImpl::Release()
{
    player_ = nullptr;
    return ERR_OK;
}

int32_t TestPlayerImpl::Seek(int64_t timeMs, PlayerSeekMode mode)
{
    int32_t ret = player_->Seek(timeMs, mode);
    NZERO_RETURN(ret);
    while (!g_seekFinished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 50
    }
    FALSE_RETURN_V(g_seekFinished, false);
    g_seekFinished = false;
    return ret;
}

int32_t TestPlayerImpl::GetCurrentTime(int64_t& currentMs)
{
    int32_t currentTimeMS = 0;
    int32_t ret = player_->GetCurrentTime(currentTimeMS);
    currentMs = currentTimeMS;
    return ret;
}

int32_t TestPlayerImpl::GetDuration(int64_t& durationMs)
{
    int32_t duration;
    int32_t ret = player_->GetDuration(duration);
    durationMs = duration;
    return ret;
}
int32_t TestPlayerImpl::SetPlaybackSpeed(PlaybackRateMode mode)
{
    return player_->SetPlaybackSpeed(mode);
}

int32_t TestPlayerImpl::GetPlaybackSpeed(PlaybackRateMode& mode)
{
    return player_->GetPlaybackSpeed(mode);
}

int32_t TestPlayerImpl::SetVolume(float leftVolume, float rightVolume)
{
    int32_t ret = player_->SetVolume(leftVolume, rightVolume);
    return ret;
}

int32_t TestPlayerImpl::GetAudioTrackInfo(std::vector<Format> &audioTrack)
{
    int32_t ret = player_->GetAudioTrackInfo(audioTrack);
    return ret;
}

int32_t TestPlayerImpl::GetVideoTrackInfo(std::vector<Format> &videoTrack)
{
    int32_t ret = player_->GetVideoTrackInfo(videoTrack);
    return ret;
}
}
#endif