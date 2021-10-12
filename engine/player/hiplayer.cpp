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

#define LOG_TAG "HiPlayer"

#include "histreamer/hiplayer.h"

#include <memory>
#include "hiplayer_impl.h"

namespace {
const float MAX_MEDIA_VOLUME = 300.0f;
}

namespace OHOS {
namespace Media {
HiPlayer::HiPlayer()
    : player_(HiPlayerImpl::CreateHiPlayerImpl()),
      curState_(Media::PlayerStates::PLAYER_IDLE),
      isInited_(false),
      isSingleLoop_(false),
      audioStreamType_(-1)
{
    MEDIA_LOG_I("ctor entered.");
}

HiPlayer::~HiPlayer()
{
    MEDIA_LOG_I("dtor entered.");
    player_ = nullptr;
    isInited_ = false;
    isSingleLoop_ = false;
}

int32_t HiPlayer::SetSource(const Media::Source& source)
{
    MEDIA_LOG_I("SetSource entered.");
    Init();
    auto src = std::make_shared<Media::Source>(source);
    int ret = -1;
    if (player_ && player_->SetSource(src) == SUCCESS) {
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    int ret = -1;
    if (player_ && player_->Prepare() == SUCCESS) {
        curState_ = Media::PlayerStates::PLAYER_PREPARED;
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::Play()
{
    MEDIA_LOG_I("Play entered.");
    int ret = -1;
    if (curState_ == Media::PlayerStates::PLAYER_PAUSED) {
        if (player_ && player_->Resume() == SUCCESS) {
            ret = 0;
        }
    } else if (player_ && player_->Play() == SUCCESS) {
        curState_ = Media::PlayerStates::PLAYER_STARTED;
        ret = 0;
    }
    return ret;
}

bool HiPlayer::IsPlaying()
{
    return player_ && curState_ == Media::PlayerStates::PLAYER_STARTED;
}

int32_t HiPlayer::Pause()
{
    MEDIA_LOG_I("Pause entered.");
    int ret = -1;
    if (player_ && player_->Pause() == SUCCESS) {
        curState_ = Media::PlayerStates::PLAYER_PAUSED;
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::Stop()
{
    MEDIA_LOG_I("Stop entered.");
    int ret = -1;
    if (player_ && player_->Stop() == SUCCESS) {
        curState_ = Media::PlayerStates::PLAYER_STOPPED;
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::Rewind(int64_t mSeconds, int32_t mode)
{
    MEDIA_LOG_I("Rewind entered.");
    int ret = -1;
    size_t pos = 0;
    if (player_ && player_->Seek(mSeconds, pos) == SUCCESS) {
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::SetVolume(float leftVolume, float rightVolume)
{
    if ((curState_ != Media::PlayerStates::PLAYER_STARTED) && (curState_ != Media::PlayerStates::PLAYER_PAUSED) &&
        (curState_ != Media::PlayerStates::PLAYER_PREPARED)) {
        MEDIA_LOG_E("cannot set volume in state %d", curState_);
        return -1;
    }
    if (leftVolume < 0 || leftVolume > MAX_MEDIA_VOLUME || rightVolume < 0 || rightVolume > MAX_MEDIA_VOLUME) {
        MEDIA_LOG_E("volume not valid, should be in range [0,300]");
        return -1;
    }
    float volume = 0.f;
    if (leftVolume < 1e-6 && rightVolume >= 1e-6) { // 1e-6
        volume = rightVolume;
    } else if (rightVolume < 1e-6 && leftVolume >= 1e-6) { // 1e-6
        volume = leftVolume;
    } else {
        volume = (leftVolume + rightVolume) / 2; // 2
    }
    volume /= MAX_MEDIA_VOLUME; // normalize to 0~1
    int ret = -1;               // -1
    MEDIA_LOG_W("set volume %.3f", volume);
    if (player_ && player_->SetVolume(volume) == SUCCESS) {
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::SetSurface(Surface* surface)
{
    return 0;
}

bool HiPlayer::IsSingleLooping()
{
    return isSingleLoop_;
}

int32_t HiPlayer::GetPlayerState(int32_t& state)
{
    if (!player_) {
        return -1;
    }
    state = static_cast<int32_t>(curState_);
    return 0;
}

int32_t HiPlayer::GetCurrentPosition(int64_t& currentPosition)
{
    int ret = -1;
    if (player_ && player_->GetCurrentTime(currentPosition) == SUCCESS) {
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::GetDuration(int64_t& duration)
{
    int ret = -1;
    size_t durationTime = 0;
    if (player_ && player_->GetDuration(durationTime) == SUCCESS) {
        duration = static_cast<int64_t>(durationTime);
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::GetVideoWidth(int32_t& videoWidth)
{
    return -1;
}

int32_t HiPlayer::GetVideoHeight(int32_t& videoHeight)
{
    return -1;
}

int32_t HiPlayer::SetPlaybackSpeed(float speed)
{
    return 0;
}

int32_t HiPlayer::GetPlaybackSpeed(float& speed)
{
    speed = 1.0;
    return 0;
}

int32_t HiPlayer::SetAudioStreamType(int32_t type)
{
    if (curState_ != Media::PlayerStates::PLAYER_IDLE && curState_ != Media::PlayerStates::PLAYER_INITIALIZED) {
        return -1;
    }
    audioStreamType_ = type;
    return 0;
}

void HiPlayer::GetAudioStreamType(int32_t& type)
{
    type = audioStreamType_;
}

int32_t HiPlayer::Reset()
{
    int ret = -1;
    if (player_ && player_->Stop() == SUCCESS) {
        ret = 0;
    }
    return ret;
}

int32_t HiPlayer::Release()
{
    return Reset();
}

int32_t HiPlayer::SetLoop(bool loop)
{
    MEDIA_LOG_I("SetLoop entered.");
    int ret = -1;
    if (player_ && player_->SetSingleLoop(loop) == SUCCESS) {
        isSingleLoop_ = loop;
        ret = 0;
    }
    return ret;
}

void HiPlayer::SetPlayerCallback(const std::shared_ptr<Media::PlayerCallback>& cb)
{
    if (player_ && player_->SetCallback(cb) == SUCCESS) {
        MEDIA_LOG_I("SetPlayerCallback succ.");
    } else {
        MEDIA_LOG_E("SetPlayerCallback failed.");
    }
}

int32_t HiPlayer::Init()
{
    MEDIA_LOG_I("Init entered.");
    if (isInited_) {
        return 0;
    }
    if (player_) {
        player_->Init();
    }
    curState_ = Media::PlayerStates::PLAYER_INITIALIZED;
    return 0;
}

int32_t HiPlayer::DeInit()
{
    return 0;
}

int32_t HiPlayer::SetParameter(const Media::Format& params)
{
    return 0;
}

std::shared_ptr<Media::PlayerInterface> CreateHiPlayer()
{
    return std::shared_ptr<HiPlayer>(new (std::nothrow) HiPlayer());
}
} // namespace Media
} // namespace OHOS
