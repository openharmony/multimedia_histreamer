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
#ifndef HISTREAMER_ST_TEST_PLAYER
#define HISTREAMER_ST_TEST_PLAYER

#include <cstring>
#include <format.h>
#include <string>
#include <vector>
#include "media_data_source_impl.h"
#include "plugin/common/plugin_types.h"
#include "securec.h"
#include "player.h"

#ifndef WIN32
#define HST_TEST(UtObject, function, level) HWTEST_F(UtObject, function, level)
#else
#define HST_TEST(UtObject, function, level) TEST_F(UtObject, function)
#endif

namespace OHOS::Media::Test {
enum class TestSourceType : int32_t {
    URI = 0,
    STREAM = 1
};

class TestSource {
public:
    static TestSource CreateTestSource(const std::string& url, TestSourceType type, Plugin::Seekable seekable);
    explicit TestSource(std::string url) : url_(std::move(url)), type_(TestSourceType::URI) {}
    TestSource(std::string url, Plugin::Seekable seekable) : url_(std::move(url)),
        type_(TestSourceType::STREAM), seekable_(seekable) {}

    std::string url_;
    TestSourceType type_;
    Plugin::Seekable seekable_ {Plugin::Seekable::INVALID};
};

class TestPlayer {
public:
    static std::unique_ptr<TestPlayer> Create();
    virtual ~TestPlayer() = default;
    virtual int32_t SetSource(const TestSource& source) = 0;
    virtual int32_t SetSingleLoop(bool loop) = 0;
    virtual bool IsPlaying() = 0;
    virtual int32_t Prepare() = 0;
    virtual int32_t Play() = 0;
    virtual int32_t Pause() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t Reset() = 0;
    virtual int32_t Release() = 0;
    virtual int32_t Seek(int64_t timeMs, PlayerSeekMode mode = PlayerSeekMode::SEEK_NEXT_SYNC) = 0;
    virtual int32_t GetCurrentTime(int64_t& currentMs) = 0;
    virtual int32_t GetDuration(int64_t& durationMs) = 0;
    virtual int32_t SetVolume(float leftVolume, float rightVolume) = 0;
    virtual int32_t GetAudioTrackInfo(std::vector<Format> &audioTrack) = 0;
    virtual int32_t GetVideoTrackInfo(std::vector<Format> &videoTrack) = 0;
    virtual int32_t SetPlaybackSpeed(PlaybackRateMode mode)  = 0;
    virtual int32_t GetPlaybackSpeed(PlaybackRateMode &mode) = 0;
};
}
#endif