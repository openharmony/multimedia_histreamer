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

#define LOG_TAG "FilterPort"

#include "port.h"
#include <algorithm>
#include "filter.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
std::shared_ptr<InPort> EmptyInPort::port = std::make_shared<EmptyInPort>();
std::shared_ptr<OutPort> EmptyOutPort::port = std::make_shared<EmptyOutPort>();

const std::string& Port::GetName()
{
    return name;
}

const InfoTransfer* Port::GetOwnerFilter() const
{
    return filter;
}

std::shared_ptr<Port> Port::GetPeerPort()
{
    return nullptr;
}

ErrorCode InPort::Connect(std::shared_ptr<Port> port)
{
    prevPort = port;
    return SUCCESS;
}

ErrorCode InPort::Disconnect()
{
    prevPort.reset();
    return SUCCESS;
}

ErrorCode InPort::Activate(const std::vector<WorkMode>& modes, WorkMode& outMode)
{
    if (auto ptr = prevPort.lock()) {
        FAIL_RETURN(ptr->Activate(modes, workMode));
        outMode = workMode;
        return SUCCESS;
    }
    MEDIA_LOG_E("[Filter %s] InPort %s Activate error: prevPort destructed", filter->GetName().c_str(), name.c_str());
    return NULL_POINTER_ERROR;
}

std::shared_ptr<Port> InPort::GetPeerPort()
{
    return prevPort.lock();
}

bool InPort::Negotiate(const std::shared_ptr<const Meta>& inMeta, CapabilitySet& outCaps)
{
    return filter && filter->Negotiate(name, inMeta, outCaps);
}

void InPort::PushData(AVBufferPtr buffer)
{
    if (filter) {
        filter->PushData(name, buffer);
    } else {
        MEDIA_LOG_E("filter destructed");
    }
}

ErrorCode InPort::PullData(uint64_t offset, size_t size, AVBufferPtr& data)
{
    if (auto ptr = prevPort.lock()) {
        return ptr->PullData(offset, size, data);
    }
    MEDIA_LOG_E("prevPort destructed");
    return NULL_POINTER_ERROR;
}

ErrorCode OutPort::Connect(std::shared_ptr<Port> port)
{
    if (InSamePipeline(port)) {
        nextPort = port;
        return SUCCESS;
    }
    MEDIA_LOG_E("Connect filters that are not in the same pipeline.");
    return INVALID_PARAM_VALUE;
}

ErrorCode OutPort::Disconnect()
{
    nextPort.reset();
    return SUCCESS;
}

bool OutPort::InSamePipeline(std::shared_ptr<Port> port) const
{
    auto filter1 = GetOwnerFilter();
    FALSE_RETURN_V(filter1 != nullptr, false);
    auto filter2 = port->GetOwnerFilter();
    FALSE_RETURN_V(filter2 != nullptr, false);
    auto pipeline1 = filter1->GetOwnerPipeline();
    FALSE_RETURN_V(pipeline1 != nullptr, false);
    auto pipeline2 = filter2->GetOwnerPipeline();
    FALSE_RETURN_V(pipeline2 != nullptr, false);
    return pipeline1 == pipeline2;
}

ErrorCode OutPort::Activate(const std::vector<WorkMode>& modes, WorkMode& outMode)
{
    if (filter) {
        auto supportedModes = filter->GetWorkModes();
        for (auto mode : modes) {
            auto found = std::find(supportedModes.cbegin(), supportedModes.cend(), mode);
            if (found != supportedModes.cend()) {
                outMode = mode;
                workMode = mode;
                return SUCCESS; // 最先找到的兼容的mode，作为最后结果
            }
        }
    } else {
        MEDIA_LOG_E("filter destructed");
    }
    return NEGOTIATE_ERROR;
}

std::shared_ptr<Port> OutPort::GetPeerPort()
{
    return nextPort;
}

bool OutPort::Negotiate(const std::shared_ptr<const Meta>& inMeta, CapabilitySet& outCaps)
{
    return nextPort->Negotiate(inMeta, outCaps);
}

void OutPort::PushData(AVBufferPtr buffer)
{
    nextPort->PushData(buffer);
}

ErrorCode OutPort::PullData(uint64_t offset, size_t size, AVBufferPtr& data)
{
    if (filter) {
        return filter->PullData(name, offset, size, data);
    }
    MEDIA_LOG_E("filter destructed");
    return NULL_POINTER_ERROR;
}

ErrorCode EmptyInPort::Connect(std::shared_ptr<Port> port)
{
    MEDIA_LOG_E("Connect in EmptyInPort");
    return SUCCESS;
}
ErrorCode EmptyInPort::Activate(const std::vector<WorkMode>& modes, WorkMode& outMode)
{
    MEDIA_LOG_E("Activate in EmptyInPort");
    return SUCCESS;
}
bool EmptyInPort::Negotiate(const std::shared_ptr<const Meta>& inMeta, CapabilitySet& outCaps)
{
    MEDIA_LOG_E("Negotiate in EmptyInPort");
    return false;
}
void EmptyInPort::PushData(AVBufferPtr buffer)
{
    MEDIA_LOG_E("PushData in EmptyInPort");
}
ErrorCode EmptyInPort::PullData(uint64_t offset, size_t size, AVBufferPtr& data)
{
    MEDIA_LOG_E("PullData in EmptyInPort");
    return UNIMPLEMENT;
}

ErrorCode EmptyOutPort::Connect(std::shared_ptr<Port> port)
{
    MEDIA_LOG_E("Connect in EmptyOutPort");
    return SUCCESS;
}
ErrorCode EmptyOutPort::Activate(const std::vector<WorkMode>& modes, WorkMode& outMode)
{
    MEDIA_LOG_E("Activate in EmptyOutPort");
    return SUCCESS;
}
bool EmptyOutPort::Negotiate(const std::shared_ptr<const Meta>& inMeta, CapabilitySet& outCaps)
{
    MEDIA_LOG_E("Negotiate in EmptyOutPort");
    return false;
}
void EmptyOutPort::PushData(AVBufferPtr buffer)
{
    MEDIA_LOG_E("PushData in EmptyOutPort");
}
ErrorCode EmptyOutPort::PullData(uint64_t offset, size_t size, AVBufferPtr& data)
{
    MEDIA_LOG_E("PullData in EmptyOutPort");
    return UNIMPLEMENT;
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
