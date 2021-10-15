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

#define LOG_TAG "StreamSourcePlugin"

#include "stream_source_plugin.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/core/plugin_manager.h"

namespace OHOS {
namespace Media {
namespace Plugin {
std::shared_ptr<SourcePlugin> StreamSourcePluginCreater(const std::string& name)
{
    return std::make_shared<StreamSourcePlugin>(name);
}

const Status StreamSourceRegister(const std::shared_ptr<Register>& reg)
{
    SourcePluginDef definition;
    definition.name = "StreamSource";
    definition.description = "Stream source";
    definition.rank = 100; // 100: max rank
    definition.protocol = "stream";
    definition.creator = StreamSourcePluginCreater;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(StreamSource, LicenseType::APACHE_V2, StreamSourceRegister, [] {});

void* StreamSourceAllocator::Alloc(size_t size)
{
    if (size == 0) {
        return nullptr;
    }
    return reinterpret_cast<void*>(new (std::nothrow) uint8_t[size]);
}

void StreamSourceAllocator::Free(void* ptr) // NOLINT: void*
{
    if (ptr != nullptr) {
        delete[](uint8_t*) ptr;
    }
}

StreamSourceCallback::StreamSourceCallback(std::shared_ptr<StreamSourcePlugin> dataSource,
                                           std::shared_ptr<StreamSource>& stream)
    : dataSource_(dataSource), streamSource_(stream)
{
}

uint8_t* StreamSourceCallback::GetBuffer(size_t index)
{
    auto bufferPtr = dataSource_->FindBuffer(index);
    return bufferPtr->GetMemory()->GetWritableData(bufferPtr->GetMemory()->GetCapacity());
}

void StreamSourceCallback::QueueBuffer(size_t index, size_t offset, size_t size, int64_t timestampUs, uint32_t flags)
{
    auto bufferPtr = dataSource_->FindBuffer(index);
    dataSource_->EraseBuffer(index);
    bufferPtr->GetMemory()->GetWritableData(size);
    dataSource_->EnqueBuffer(bufferPtr);
}

StreamSourcePlugin::StreamSourcePlugin(std::string name)
    : SourcePlugin(std::move(name)),
      bufferPool_(0),
      state_(State::CREATED),
      isSeekable_(false),
      waitBuffers_(),
      bufferQueue_("SourceBuffQue")
{
    MEDIA_LOG_D("ctor called");
}

StreamSourcePlugin::~StreamSourcePlugin()
{
    MEDIA_LOG_D("dtor called");
    state_ = State::DESTROYED;
}

Status StreamSourcePlugin::Init()
{
    MEDIA_LOG_D("IN");
    bufferPool_.Init(DEFAULT_FRAME_SIZE);
    mAllocator_ = std::make_shared<StreamSourceAllocator>();
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status StreamSourcePlugin::Deinit()
{
    MEDIA_LOG_D("IN");
    state_ = State::DESTROYED;
    return Status::OK;
}

Status StreamSourcePlugin::Prepare()
{
    MEDIA_LOG_D("IN");
    state_ = State::PREPARED;
    return Status::OK;
}

Status StreamSourcePlugin::Reset()
{
    MEDIA_LOG_D("IN");
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status StreamSourcePlugin::Start()
{
    MEDIA_LOG_D("IN");
    bufferPool_.SetActive(true);
    taskPtr_->Start();
    state_ = State::RUNNING;
    return Status::OK;
}

Status StreamSourcePlugin::Stop()
{
    MEDIA_LOG_D("IN");
    bufferQueue_.SetActive(false);
    taskPtr_->Stop();
    state_ = State::PREPARED;
    return Status::OK;
}

bool StreamSourcePlugin::IsParameterSupported(Tag tag)
{
    MEDIA_LOG_D("IN");
    return true;
}

Status StreamSourcePlugin::GetParameter(Tag tag, ValueType& value)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StreamSourcePlugin::SetParameter(Tag tag, const ValueType& value)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

std::shared_ptr<Allocator> StreamSourcePlugin::GetAllocator()
{
    MEDIA_LOG_D("IN");
    return mAllocator_;
}

Status StreamSourcePlugin::SetCallback(const std::shared_ptr<Callback>& cb)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StreamSourcePlugin::SetSource(std::string& uri, std::shared_ptr<std::map<std::string, ValueType>> params)
{
    if (uri.compare("stream://") || (params == nullptr)) {
        MEDIA_LOG_E("Bad uri: %s", uri.c_str());
        return Status::ERROR_INVALID_DATA;
    }
    std::shared_ptr<MediaSource> source_ = nullptr;
    for (const auto& iter_ : *params) {
        std::string key_ = iter_.first;
        ValueType val_ = iter_.second;
        if ((key_.compare("StreamSource") == 0) && (val_.Type() == typeid(std::shared_ptr<MediaSource>))) {
            source_ = Plugin::AnyCast<std::shared_ptr<MediaSource>>(val_);
            break;
        }
    }
    if (source_ == nullptr) {
        MEDIA_LOG_E("Bad source");
        return Status::ERROR_INVALID_DATA;
    }
    std::shared_ptr<StreamSource> stream_ = source_->GetSourceStream();
    if (stream_ == nullptr) {
        MEDIA_LOG_E("Get StreamSource fail");
        return Status::ERROR_INVALID_DATA;
    }

    streamCallback_ = std::make_shared<StreamSourceCallback>(shared_from_this(), stream_);
    stream_->SetStreamCallback(streamCallback_);
    streamSource_ = stream_;
    taskPtr_ = std::make_shared<OSAL::Task>("StreamSource");
    taskPtr_->RegisterHandler(std::bind(&StreamSourcePlugin::NotifyAvilableBufferLoop, this));
    return Status::OK;
}

Status StreamSourcePlugin::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    AVBufferPtr bufPtr_ = bufferQueue_.Pop(); // the cached buffer
    auto availSize = bufPtr_->GetMemory()->GetSize();
    MEDIA_LOG_D("availSize: %zu, expectedLen: %zu\n", availSize, expectedLen);
    if (buffer->IsEmpty()) { // No buffer provided, use the cached buffer.
        buffer = bufPtr_;
        return Status::OK;
    } else { // Buffer provided, copy it.
        if (buffer->GetMemory()->GetCapacity() < availSize) {
            MEDIA_LOG_D("buffer->length: %zu is smaller than %zu\n", buffer->GetMemory()->GetCapacity(), availSize);
            return Status::ERROR_NO_MEMORY;
        }
        buffer->GetMemory()->Write(bufPtr_->GetMemory()->GetReadOnlyData(), availSize);
    }
    return Status::OK;
}

Status StreamSourcePlugin::GetSize(size_t& size)
{
    MEDIA_LOG_D("IN");
    size = -1;
    return Status::ERROR_WRONG_STATE;
}

bool StreamSourcePlugin::IsSeekable()
{
    MEDIA_LOG_D("IN");
    return isSeekable_;
}

Status StreamSourcePlugin::SeekTo(uint64_t offset)
{
    MEDIA_LOG_D("IN");
    return Status::ERROR_UNIMPLEMENTED;
}

AVBufferPtr StreamSourcePlugin::AllocateBuffer()
{
    return bufferPool_.AllocateBuffer();
}

AVBufferPtr StreamSourcePlugin::FindBuffer(size_t idx)
{
    OSAL::ScopedLock lock(mutex_);
    auto it = waitBuffers_.find(idx);
    if (it != waitBuffers_.end()) {
        return it->second;
    }
    return nullptr;
}

void StreamSourcePlugin::EraseBuffer(size_t idx)
{
    OSAL::ScopedLock lock(mutex_);
    waitBuffers_.erase(idx);
}

void StreamSourcePlugin::EnqueBuffer(AVBufferPtr& bufferPtr)
{
    bufferQueue_.Push(bufferPtr);
}

void StreamSourcePlugin::NotifyAvilableBufferLoop()
{
    auto bufferPtr = AllocateBuffer();
    if (bufferPtr == nullptr) {
        MEDIA_LOG_E("Alloc buffer fail");
        return;
    }
    size_t idx = GetUniqueIdx();
    {
        OSAL::ScopedLock lock(mutex_);
        waitBuffers_[idx] = bufferPtr;
    }
    std::shared_ptr<StreamSource> stream = streamSource_.lock();
    stream->OnBufferAvailable(idx, 0, bufferPtr->GetMemory()->GetCapacity());
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
