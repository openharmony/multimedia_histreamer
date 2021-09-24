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

#include "base.h"
#include "plugin/interface/plugin_base.h"

using namespace OHOS::Media::Plugin;

Base::Base(uint32_t pkgVer, uint32_t apiVer, std::shared_ptr<PluginBase> base)
    : pkgVersion(pkgVer), apiVersion(apiVer), plugin(std::move(base))
{
}

Status Base::Init()
{
    return plugin->Init();
}

Status Base::Deinit()
{
    return plugin->Deinit();
}

Status Base::Prepare()
{
    return plugin->Prepare();
}

Status Base::Reset()
{
    return plugin->Reset();
}

Status Base::Start()
{
    return plugin->Start();
}

Status Base::Stop()
{
    return plugin->Stop();
}

bool Base::IsParameterSupported(Tag tag)
{
    return plugin->IsParameterSupported(tag);
}

Status Base::GetParameter(Tag tag, ValueType& value)
{
    return plugin->GetParameter(tag, value);
}

Status Base::SetParameter(Tag tag, const ValueType& value)
{
    return plugin->SetParameter(tag, value);
}

Status Base::GetState(State &state)
{
    return plugin->GetState(state);
}

std::shared_ptr<Allocator> Base::GetAllocator()
{
    return plugin->GetAllocator();
}
