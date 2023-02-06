/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "HdiCodecAdapter"

#include "hdi_codec_adapter.h"
#include <utility>
#include "codec_callback_type_stub.h"
#include "codec_callback_if.h"
#include "codec_component_if.h"
#include "codec_omx_ext.h"
#include "foundation/log.h"
#include "hdf_base.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
Status RegisterHdiAdapterPlugins(const std::shared_ptr<OHOS::Media::Plugin::Register>& reg)
{
    g_compManager = std::make_shared<HdiCodecManager>();
    return g_compManager->RegisterCodecPlugins(reg);
}

void UnRegisterHdiAdapterPlugins()
{
    g_compManager->UnRegisterCodecPlugins();
    g_compManager = nullptr;
}

PLUGIN_DEFINITION(HdiAdapter, LicenseType::APACHE_V2, RegisterHdiAdapterPlugins, UnRegisterHdiAdapterPlugins);

HdiCodecAdapter::HdiCodecAdapter(std::string componentName, std::shared_ptr<CodecManager>& codecManager)
    : CodecPlugin(std::move(componentName)), codecMgr_(codecManager)
{
}

Status HdiCodecAdapter::Init()
{
    return Status::OK;
}

Status HdiCodecAdapter::Deinit() 
{
    return Status::OK;
}

Status HdiCodecAdapter::Prepare() 
{
    return Status::OK;
}

Status HdiCodecAdapter::Reset() 
{
    return Status::OK;
}

Status HdiCodecAdapter::Start() 
{
    return Status::OK;
}

Status HdiCodecAdapter::Stop() 
{
    return Status::OK;
}

Status HdiCodecAdapter::Flush() 
{
    return Status::OK;
}

Status HdiCodecAdapter::GetParameter(Plugin::Tag tag, ValueType &value) 
{
    return Status::OK;
}

Status HdiCodecAdapter::SetParameter(Plugin::Tag tag, const ValueType &value) 
{
    return Status::OK;
}

Status HdiCodecAdapter::QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    return Status::OK;
}

Status HdiCodecAdapter::QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs)
{
    return Status::OK;
}

Status HdiCodecAdapter::SetCallback(Callback *cb) 
{
    return Status::OK;
}

Status HdiCodecAdapter::SetDataCallback(DataCallback *dataCallback) 
{
    return Status::OK;
}

void HdiCodecAdapter::NotifyInputBufferDone(const std::shared_ptr<Buffer> &input) 
{

}

void HdiCodecAdapter::NotifyOutputBufferDone(const std::shared_ptr<Buffer> &output)
{

}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif