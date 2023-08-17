/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef I_PLAYER_ENGINE_H
#define I_PLAYER_ENGINE_H

#include <map>
#include <vector>
#include <cstdint>
#include <string>
#include <refbase.h>
#include "player.h"
#include "nocopyable.h"

namespace OHOS {
class Surface;

namespace Media {
class IPlayerEngineObs : public std::enable_shared_from_this<IPlayerEngineObs> {
public:
    virtual ~IPlayerEngineObs() = default;
    virtual void OnError(PlayerErrorType errorType, int32_t errorCode) = 0;
    virtual void OnErrorMessage(int32_t errorCode, const std::string &errorMsg)
    {
        (void)errorCode;
        (void)errorMsg;
    }
    virtual void OnInfo(PlayerOnInfoType type, int32_t extra, const Format &infoBody) = 0;
};

class IPlayerEngine {
public:
    virtual ~IPlayerEngine() = default;

    virtual int32_t SetSource(const std::string &url) = 0;
    virtual int32_t SetSource(const std::shared_ptr<IMediaDataSource> &dataSrc) = 0;
    virtual int32_t Play() = 0;
    virtual int32_t Prepare() = 0;
    virtual int32_t PrepareAsync() = 0;
    virtual int32_t Pause() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t Reset() = 0;
    virtual int32_t SetVolume(float leftVolume, float rightVolume) = 0;
    virtual int32_t Seek(int32_t mSeconds, PlayerSeekMode mode) = 0;
    virtual int32_t GetCurrentTime(int32_t &currentTime) = 0;
    virtual int32_t GetVideoTrackInfo(std::vector<Format> &videoTrack) = 0;
    virtual int32_t GetAudioTrackInfo(std::vector<Format> &audioTrack) = 0;
    virtual int32_t GetVideoWidth() = 0;
    virtual int32_t GetVideoHeight() = 0;
    virtual int32_t GetDuration(int32_t &duration) = 0;
    virtual int32_t SetPlaybackSpeed(PlaybackRateMode mode) = 0;
    virtual int32_t GetPlaybackSpeed(PlaybackRateMode &mode) = 0;
    virtual int32_t SetVideoSurface(sptr<Surface> surface) = 0;
    virtual int32_t SetLooping(bool loop) = 0;
    virtual int32_t SetParameter(const Format &param) = 0;
    virtual int32_t SetObs(const std::weak_ptr<IPlayerEngineObs> &obs) = 0;
    virtual int32_t SelectBitRate(uint32_t bitRate)
    {
        (void)bitRate;
        return 0;
    }
    virtual int32_t SetVideoScaleType(VideoScaleType videoScaleType)
    {
        (void)videoScaleType;
        return 0;
    }
    virtual int32_t SetAudioRendererInfo(const int32_t contentType, const int32_t streamUsage,
        const int32_t rendererFlag)
    {
        (void)contentType;
        (void)streamUsage;
        (void)rendererFlag;
        return 0;
    }
    virtual int32_t SetAudioInterruptMode(const int32_t interruptMode)
    {
        (void)interruptMode;
        return 0;
    }
};
} // namespace Media
} // namespace OHOS
#endif // I_PLAYER_ENGINE_H
