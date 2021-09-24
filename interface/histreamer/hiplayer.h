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

#ifndef HISTREAMER_HIPLAYER_H
#define HISTREAMER_HIPLAYER_H

#include <memory>
#include "player_interface.h"

namespace OHOS {
namespace Media {
class HiPlayer : public Media::PlayerInterface {
public:
    HiPlayer();
    ~HiPlayer() override;
    int32_t SetSource(const Media::Source& source) override;
    int32_t Prepare() override;
    int32_t Play() override;
    bool IsPlaying() override;
    int32_t Pause() override;
    int32_t Stop() override;
    int32_t Rewind(int64_t mSeconds, int32_t mode) override;
    int32_t SetVolume(float leftVolume, float rightVolume) override;
    int32_t SetSurface(Surface* surface) override;
    bool IsSingleLooping() override;
    int32_t GetPlayerState(int32_t& state) override;
    int32_t GetCurrentPosition(int64_t& currentPosition) override;
    int32_t GetDuration(int64_t& duration) override;
    int32_t GetVideoWidth(int32_t& videoWidth) override;
    int32_t GetVideoHeight(int32_t& videoHeight) override;
    int32_t SetPlaybackSpeed(float speed) override;
    int32_t GetPlaybackSpeed(float& speed) override;
    int32_t SetAudioStreamType(int32_t type) override;
    void GetAudioStreamType(int32_t& type) override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t SetLoop(bool loop) override;
    void SetPlayerCallback(const std::shared_ptr<Media::PlayerCallback>& cb) override;
    int32_t Init() override;
    int32_t DeInit() override;
    int32_t SetParameter(const Media::Format& params) override;

private:
    class HiPlayerImpl;
    std::shared_ptr<HiPlayerImpl> player_;
    Media::PlayerStates curState_;
    bool isInited_;
    bool isSingleLoop_;
    int audioStreamType_;
};

std::shared_ptr<Media::PlayerInterface> CreateHiPlayer();
} // namespace Media
} // namespace OHOS
#endif // MULTIMEDIA_HIPLAYER_H
