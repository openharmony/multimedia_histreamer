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
#include <mutex>
#include "hls_streaming.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
void HLSStreaming::FragmentListUpdateLoop()
{
    OSAL::SleepFor(5000); // 5000 how often is playlist updated
    UpdateManifest();
}

void HLSStreaming::ProcessManifest(std::string url)
{
    url_ = url;
    master_ = nullptr;
    Open(url);
}

void HLSStreaming::UpdateManifest()
{
    Open(currentVariant_->m3u8_->uri_);
}

void HLSStreaming::SetFragmentListCallback(FragmentListChangeCallback* callback)
{
    callback_ = callback;
}

double HLSStreaming::GetDuration() const
{
    if (!master_) {
        return 0;
    }
    return master_->bLive_ ? -1.0 : master_->duration_; // -1.0
}

bool HLSStreaming::IsStreaming() const
{
    if (master_ == nullptr) {
        return true;
    }
    return master_->bLive_;
}

void HLSStreaming::ParseManifest()
{
    if (!master_) {
        master_ = std::make_shared<M3U8MasterPlaylist>(playList_, url_);
        currentVariant_ = master_->defaultVariant_;
        if (!master_->isSimple_) {
            UpdateManifest();
        }
    } else {
        currentVariant_->m3u8_->Update(playList_);
        auto files = currentVariant_->m3u8_->files_;
        auto fragmentList = std::vector<std::string>();
        fragmentList.reserve(files.size());
        for (auto &file: files) {
            fragmentList.push_back(file->uri_);
        }
        callback_->OnFragmentListChanged(fragmentList);
    }
}
}
}
}
}