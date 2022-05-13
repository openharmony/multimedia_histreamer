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
#define HST_LOG_TAG "HlsStreaming"
#include "hls_streaming.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
HLSStreaming::HLSStreaming(const std::string &url) : AdaptiveStreaming(url)
{
}

HLSStreaming::~HLSStreaming()
{
}

void HLSStreaming::SetCurrentVariant(std::shared_ptr<M3U8VariantStream> &variant)
{
    currentVariant_ = variant;
}

bool HLSStreaming::UpdateM3U8()
{
    if (GetPlaylist(currentVariant_->m3u8_->uri_)) {
        std::string m3u8(playList);
        return currentVariant_->m3u8_->Update(m3u8);
    }
    MEDIA_LOG_E("HLSStreaming::UpdateM3U8 Error");
    return false;
}

bool HLSStreaming::ProcessManifest()
{
    if (GetPlaylist(GetUri())) {
        std::string masterM3U8(playList);
        master_ = std::make_shared<M3U8MasterPlaylist>(masterM3U8, GetUri());
        SetCurrentVariant(master_->defaultVariant_);
        if (!master_->isSimple_) {
            return UpdateM3U8();
        }
        return true;
    }
    MEDIA_LOG_E("HLSStreaming::ProcessManifest Error");
    return false;
}

bool HLSStreaming::UpdateManifest()
{
    return UpdateM3U8();
}

bool HLSStreaming::GetDownloadList(std::shared_ptr<BlockingQueue<std::string>> downloadList)
{
    std::shared_ptr<M3U8> m3u8 = currentVariant_->m3u8_;
    for (auto it = m3u8->files_.begin(); it != m3u8->files_.end(); it++) {
        downloadList->Push((*it)->uri_);
    }
    return true;
}
}
}
}
}