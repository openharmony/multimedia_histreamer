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
#include "player/standard/hiplayer_impl.h"

namespace OHOS {
namespace Media {
int32_t HstEngineFactory::Score(Scene scene, const std::string& uri)
{
    (void)scene;
    (void)uri;
    return MIN_SCORE + 1;
}

std::unique_ptr<IPlayerEngine> HstEngineFactory::CreatePlayerEngine()
{
    MEDIA_LOG_I("CreatePlayerEngine enter.");
    auto player = std::unique_ptr<HiPlayerImpl>(new (std::nothrow) HiPlayerImpl());
    if (player && player->Init() == ErrorCode::SUCCESS) {
        return player;
    }
    return nullptr;
}

std::unique_ptr<IRecorderEngine> HstEngineFactory::CreateRecorderEngine()
{
    MEDIA_LOG_W("CreateRecorderEngine enter.");
    return nullptr;
}

std::unique_ptr<IAVMetadataHelperEngine> HstEngineFactory::CreateAVMetadataHelperEngine()
{
    MEDIA_LOG_W("CreateAVMetadataHelperEngine not supported now, return nullptr.");
    return nullptr;
}
}  // namespace Media
}  // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32)
__declspec(dllexport)
#endif
__attribute__((visibility("default"))) OHOS::Media::IEngineFactory* CreateEngineFactory()
{
    return new (std::nothrow) OHOS::Media::HstEngineFactory();
}

#ifdef __cplusplus
}
#endif