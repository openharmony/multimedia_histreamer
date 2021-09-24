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

#include "codec.h"

#include <utility>
#include "interface/codec_plugin.h"

using namespace OHOS::Media::Plugin;

Codec::Codec(uint32_t pkgVer, uint32_t apiVer, std::shared_ptr<CodecPlugin> plugin)
    : Base(pkgVer, apiVer, plugin), codec_(std::move(plugin))
{
}

Status Codec::QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    return codec_->QueueInputBuffer(inputBuffer, timeoutMs);
}

Status Codec::QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs)
{
    return codec_->QueueOutputBuffer(outputBuffers, timeoutMs);
}

Status Codec::Flush()
{
    return codec_->Flush();
}

struct DataCallbackWrapper : DataCallback {
    DataCallbackWrapper(uint32_t pkgVersion, std::weak_ptr<DataCallbackHelper> dataCallback)
        : version(pkgVersion), helper(dataCallback)
    {
    }

    ~DataCallbackWrapper() override = default;

    void OnInputBufferDone(const std::shared_ptr<Buffer>& input) override
    {
        auto callback = helper.lock();
        if (callback) {
            callback->OnInputBufferDone(input);
        }
    }

    void OnOutputBufferDone(const std::shared_ptr<Buffer>& output) override
    {
        auto callback = helper.lock();
        if (callback) {
            callback->OnOutputBufferDone(output);
        }
    }

private:
    __attribute__((unused)) uint32_t version;
    std::weak_ptr<DataCallbackHelper> helper;
};

Status Codec::SetDataCallback(const std::weak_ptr<DataCallbackHelper>& helper)
{
    dataCallback_ = std::make_shared<DataCallbackWrapper>(pkgVersion, helper);
    return codec_->SetDataCallback(dataCallback_);
}
