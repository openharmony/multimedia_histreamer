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

#define LOG_TAG "FilterBase"

#include "filter_base.h"
#include <algorithm>
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
FilterBase::FilterBase(std::string name)
    : name_(std::move(name)), state_(FilterState::CREATED), eventReceiver_(nullptr), callback_(nullptr)
{
    inPorts_.reserve(MAX_PORT_NUMBER);
    outPorts_.reserve(MAX_PORT_NUMBER);
    routeMap_.reserve(MAX_ROUTE_NUMBER);
}

void FilterBase::Init(EventReceiver* receiver, FilterCallback* callback)
{
    this->eventReceiver_ = receiver;
    this->callback_ = callback;
    InitPorts();
    state_ = FilterState::INITIALIZED;
}

std::shared_ptr<InPort> FilterBase::GetInPort(const std::string& name)
{
    auto port = FindPort(inPorts_, name);
    FALSE_RETURN_V(port != nullptr, EmptyInPort::GetInstance());
    return port;
}

std::shared_ptr<OutPort> FilterBase::GetOutPort(const std::string& name)
{
    auto port = FindPort(outPorts_, name);
    FALSE_RETURN_V(port != nullptr, EmptyOutPort::GetInstance());
    return port;
}

ErrorCode FilterBase::Prepare()
{
    state_ = FilterState::PREPARING;

    // Filter默认InPort按Push方式获取数据
    // 只有 Demuxer 的 Prepare() 需要覆写此实现， Demuxer的 InPort可能按 Pull 或 Push 方式获取数据
    WorkMode mode;
    return GetInPort(PORT_NAME_DEFAULT)->Activate({WorkMode::PUSH}, mode);
}

ErrorCode FilterBase::Start()
{
    state_ = FilterState::RUNNING;
    return ErrorCode::SUCCESS;
}

ErrorCode FilterBase::Pause()
{
    state_ = FilterState::PAUSED;
    return ErrorCode::SUCCESS;
}

ErrorCode FilterBase::Stop()
{
    state_ = FilterState::INITIALIZED;
    mediaTypeCntMap_.clear();
    return ErrorCode::SUCCESS;
}

void FilterBase::UnlinkPrevFilters()
{
    for (auto&& inPort : inPorts_) {
        auto peer = inPort->GetPeerPort();
        inPort->Disconnect();
        if (peer) {
            peer->Disconnect();
        }
    }
}

std::vector<Filter*> FilterBase::GetNextFilters()
{
    std::vector<Filter*> nextFilters;
    nextFilters.reserve(outPorts_.size());
    for (auto&& outPort : outPorts_) {
        auto peerPort = outPort->GetPeerPort();
        if (!peerPort) {
            MEDIA_LOG_E("Filter %s outport %s has no peer port (%zu).", name_.c_str(), outPort->GetName().c_str(),
                        outPorts_.size());
            continue;
        }
        auto filter = const_cast<Filter*>(dynamic_cast<const Filter*>(peerPort->GetOwnerFilter()));
        if (filter) {
            nextFilters.emplace_back(filter);
        }
    }
    return nextFilters;
}

ErrorCode FilterBase::PushData(const std::string& inPort, AVBufferPtr buffer)
{
    UNUSED_VARIABLE(inPort);
    UNUSED_VARIABLE(buffer);
    return ErrorCode::SUCCESS;
}

ErrorCode FilterBase::PullData(const std::string& outPort, uint64_t offset, size_t size, AVBufferPtr& data)
{
    UNUSED_VARIABLE(outPort);
    UNUSED_VARIABLE(offset);
    UNUSED_VARIABLE(size);
    UNUSED_VARIABLE(data);
    return ErrorCode::SUCCESS;
}

void FilterBase::OnEvent(Event event)
{
    // Receive event from port, pass it to pipeline
    if (eventReceiver_) {
        eventReceiver_->OnEvent(event);
    }
}

void FilterBase::InitPorts()
{
    inPorts_.clear();
    outPorts_.clear();

    auto inPort = std::make_shared<InPort>(this);
    inPorts_.push_back(inPort);

    auto outPort = std::make_shared<OutPort>(this);
    outPorts_.push_back(outPort);
    routeMap_.emplace_back(inPort->GetName(), outPort->GetName());
}

template <typename T>
T FilterBase::FindPort(const std::vector<T>& list, const std::string& name)
{
    auto find = std::find_if(list.begin(), list.end(), [name](const T& item) {
        if (item == nullptr) {
            return false;
        }
        return name == item->GetName();
    });
    if (find != list.end()) {
        return *find;
    }
    MEDIA_LOG_E("Find port(%s) failed.", name.c_str());
    return nullptr;
}

std::string FilterBase::NamePort(const std::string& mime)
{
    auto type = mime.substr(0, mime.find_first_of('/'));
    if (type.empty()) {
        type = "default";
    }
    auto cnt = ++(mediaTypeCntMap_[name_ + type]);
    auto portName = type + "_" + std::to_string(cnt);
    return portName;
}

PInPort FilterBase::GetRouteInPort(const std::string& outPortName)
{
    auto ite = std::find_if(routeMap_.begin(), routeMap_.end(),
                            [&outPortName](const PairPort& pp) { return outPortName == pp.second; });
    if (ite == routeMap_.end()) {
        MEDIA_LOG_W("out port %s has no route map port", outPortName.c_str());
        return nullptr;
    }
    return GetInPort(ite->first);
}

POutPort FilterBase::GetRouteOutPort(const std::string& inPortName)
{
    auto ite = std::find_if(routeMap_.begin(), routeMap_.end(),
                            [&inPortName](const PairPort& pp) { return inPortName == pp.first; });
    if (ite == routeMap_.end()) {
        MEDIA_LOG_W("in port %s has no route map port", inPortName.c_str());
        return nullptr;
    }
    return GetOutPort(ite->second);
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
