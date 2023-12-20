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

#ifndef HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_DEFINE_H
#define HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_DEFINE_H

#include "common/status.h"
#include "buffer/avbuffer.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Media {

constexpr uint32_t AVBUFFER_QUEUE_MAX_QUEUE_SIZE = 32;

class AVBufferQueueProducer;
class AVBufferQueueConsumer;

class IBrokerListener : public IRemoteBroker {
public:
    IBrokerListener() = default;
    ~IBrokerListener() noexcept override = default;
    virtual void OnBufferFilled(std::shared_ptr<AVBuffer>&) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"Media.IAVBufferFilledListener")
};

class IProducerListener : public IRemoteBroker {
public:
    IProducerListener() = default;
    ~IProducerListener() noexcept override = default;
    virtual void OnBufferAvailable() = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"Media.IProducerListener")
};

// consumer不具备跨进程能力
class IConsumerListener : public RefBase {
public:
    IConsumerListener() = default;
    ~IConsumerListener() noexcept override = default;
    virtual void OnBufferAvailable() = 0;
};

} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_DEFINE_H
