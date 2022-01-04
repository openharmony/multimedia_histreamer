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

#ifndef HISTREAMER_PIPELINE_CORE_SYNC_INFO_MANAGER_H
#define HISTREAMER_PIPELINE_CORE_SYNC_INFO_MANAGER_H

#include <memory>
#include <queue>

#include "sync_info_provider.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class SyncInfoManager {
public:
    SyncInfoManager(const SyncInfoManager&) = delete;
    SyncInfoManager operator=(const SyncInfoManager&) = delete;
    ~SyncInfoManager() = default;
    static SyncInfoManager& Instance()
    {
        static SyncInfoManager manager;
        return manager;
    }

    // todo how to decide the priority of providers
    void RegisterProvider(const std::shared_ptr<SyncInfoProvider>& provider);
    void UnRegisterProvider(const std::shared_ptr<SyncInfoProvider>& provider);

    SyncInfoProvider& GetProvider()
    {
        return providerProxy_;
    }
private:
    SyncInfoManager() = default;
    class SyncInfoProviderProxy : public SyncInfoProvider {
    public:
        ~SyncInfoProviderProxy() override = default;

        void SetStub(std::shared_ptr<SyncInfoProvider>& stub)
        {
            stub_ = stub;
        }

        ErrorCode CheckPts(int64_t pts, int64_t &ret) override
        {
            if (stub_ == nullptr) {
                return ErrorCode::ERROR_INVALID_OPERATION;
            }
            return stub_->CheckPts(pts, ret);
        }

        ErrorCode GetCurrentPosition(int64_t &position) override
        {
            if (stub_ == nullptr) {
                return ErrorCode::ERROR_INVALID_OPERATION;
            }
            return stub_->GetCurrentPosition(position);
        }

        ErrorCode GetCurrentTimeUs(int64_t &nowUs) override
        {
            if (stub_ == nullptr) {
                return ErrorCode::ERROR_INVALID_OPERATION;
            }
            return stub_->GetCurrentTimeUs(nowUs);
        }

    private:
        std::shared_ptr<SyncInfoProvider> stub_;
    };

    SyncInfoProviderProxy providerProxy_;

    std::priority_queue<std::shared_ptr<SyncInfoProvider>> providers_;
};
} // Pipeline
} // Media
} // OHOS
#endif // HISTREAMER_PIPELINE_CORE_SYNC_INFO_MANAGER_H
