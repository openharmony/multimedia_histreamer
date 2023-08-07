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

#define HST_LOG_TAG "HstEngineFactory"

#include "hst_engine_factory.h"
#include <memory>
#include "foundation/log.h"
#include "parameter.h"
#include "scene/player/standard/hiplayer_impl.h"
#include "scene/recorder/standard/hirecorder_impl.h"

namespace OHOS {
namespace Media {
int32_t HstEngineFactory::Score(Scene scene, const std::string& uri)
{
    MEDIA_LOG_I("Score in");
    if (scene == Scene::SCENE_PLAYBACK || scene == Scene::SCENE_RECORDER) {
        char useHistreamer[10] = {0}; // 10 for system parameter usage
        auto res = GetParameter("debug.media_service.histreamer", "0", useHistreamer, sizeof(useHistreamer));
        if (res == 1 && useHistreamer[0] == '1') {
            MEDIA_LOG_I("enable histreamer");
            return MAX_SCORE;
        }
    }
    return MIN_SCORE;
}

#ifdef SUPPORT_PLAYER
std::unique_ptr<IPlayerEngine> HstEngineFactory::CreatePlayerEngine(int32_t appUid, int32_t appPid, uint32_t appTokenId)
{
    (void)appTokenId;
    MEDIA_LOG_I("CreatePlayerEngine enter.");
    auto player = std::unique_ptr<HiPlayerImpl>(new (std::nothrow) HiPlayerImpl(appUid, appPid));
    if (player && player->Init() == ErrorCode::SUCCESS) {
        return player;
    }
    MEDIA_LOG_E("create player failed or player init failed");
    return nullptr;
}
#endif

#ifdef SUPPORT_RECORDER
std::unique_ptr<IRecorderEngine> HstEngineFactory::CreateRecorderEngine(
    int32_t appUid, int32_t appPid, uint32_t appTokenId, uint64_t appFullTokenId)
{
    MEDIA_LOG_I("CreateRecorderEngine enter.");
    auto recorder = std::unique_ptr<Record::HiRecorderImpl>(new (std::nothrow) Record::HiRecorderImpl(
        appUid, appPid, appTokenId, appFullTokenId));
    if (recorder && recorder->Init() == ErrorCode::SUCCESS) {
        return recorder;
    }
    MEDIA_LOG_E("create recorder failed or recorder init failed");
    return nullptr;
}
#endif

#ifdef SUPPORT_METADATA
std::unique_ptr<IAVMetadataHelperEngine> HstEngineFactory::CreateAVMetadataHelperEngine()
{
    MEDIA_LOG_W("CreateAVMetadataHelperEngine not supported now, return nullptr.");
    return nullptr;
}
#endif

#ifdef SUPPORT_CODEC
std::unique_ptr<IAVCodecEngine> HstEngineFactory::CreateAVCodecEngine()
{
    MEDIA_LOG_W("CreateAVCodecEngine not supported now, return nullptr.");
    return nullptr;
}

std::unique_ptr<IAVCodecListEngine> HstEngineFactory::CreateAVCodecListEngine()
{
    MEDIA_LOG_W("CreateAVCodecListEngine not supported now, return nullptr.");
    return nullptr;
}
#endif

#ifdef SUPPORT_MUXER
std::unique_ptr<IAVMuxerEngine> HstEngineFactory::CreateAVMuxerEngine()
{
    MEDIA_LOG_W("CreateAVMuxerEngine not supported now, return nullptr.");
    return nullptr;
}
#endif
}  // namespace Media
}  // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define HST_EXPORT __declspec(dllexport)
#else
#if defined(__GNUC__) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#define HST_EXPORT __attribute__((visibility("default")))
#else
#define HST_EXPORT
#endif
#endif

HST_EXPORT OHOS::Media::IEngineFactory* CreateEngineFactory()
{
    return new (std::nothrow) OHOS::Media::HstEngineFactory();
}
#undef HST_EXPORT
#ifdef __cplusplus
}
#endif