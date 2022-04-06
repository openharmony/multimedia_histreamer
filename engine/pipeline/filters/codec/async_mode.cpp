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
#include "async_mode.h"
#include "common/plugin_utils.h"
#include "osal/utils/util.h"
#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
#include "plugin/common/surface_memory.h"
#endif

namespace {
constexpr uint32_t DEFAULT_TRY_DECODE_TIME = 10;
constexpr uint32_t DEFAULT_TRY_RENDER_TIME = 10;
}

namespace OHOS {
namespace Media {
namespace Pipeline {
AsyncMode::AsyncMode()
{
    MEDIA_LOG_I("ThreadMode: ASYNC");
}

AsyncMode::~AsyncMode()
{
    MEDIA_LOG_D("Async mode dtor called");
}

ErrorCode AsyncMode::Release()
{
    MEDIA_LOG_I("AsyncMode Release start.");
    stopped_ = true;

    // 先停止线程 然后释放bufferQ 如果顺序反过来 可能导致线程访问已经释放的锁
    if (handleFrameTask_) {
        handleFrameTask_->Stop();
        handleFrameTask_.reset();
    }
    if (pushTask_ != nullptr) {
        pushTask_->Stop();
        pushTask_.reset();
    }
    if (inBufQue_) {
        inBufQue_->SetActive(false);
        inBufQue_.reset();
    }
    {
        OSAL::ScopedLock l(renderMutex_);
        while (!outBufQue_.empty()) {
            outBufQue_.pop();
        }
    }
    MEDIA_LOG_I("AsyncMode Release end.");
    return ErrorCode::SUCCESS;
}

ErrorCode AsyncMode::Configure(const std::string &inPort, const std::shared_ptr<const Plugin::Meta> &upstreamMeta)
{
    stopped_ = false;
    if (handleFrameTask_) {
        handleFrameTask_->Start();
    }
    if (pushTask_) {
        pushTask_->Start();
    }
    FAIL_RETURN_MSG(TranslatePluginStatus(plugin_->SetParameter(Tag::THREAD_MODE, Plugin::ThreadMode::ASYNC)),
        "Set plugin threadMode fail");
    FAIL_RETURN_MSG(QueueAllBufferInPoolToPluginLocked(), "Configure plugin output buffers error");
    return CodecMode::Configure(inPort, upstreamMeta);
}

ErrorCode AsyncMode::PushData(const std::string &inPort, const AVBufferPtr& buffer, int64_t offset)
{
    inBufQue_->Push(buffer);
    return ErrorCode::SUCCESS;
}

ErrorCode AsyncMode::Stop()
{
    MEDIA_LOG_I("AsyncMode stop start.");
    stopped_ = true;
    pushTask_->Pause();
    inBufQue_->SetActive(false);
    {
        OSAL::ScopedLock l(renderMutex_);
        while (!outBufQue_.empty()) {
            outBufQue_.pop();
        }
    }
    if (handleFrameTask_) {
        handleFrameTask_->Pause();
    }
    outBufPool_.reset();
    MEDIA_LOG_I("AsyncMode stop end.");
    return ErrorCode::SUCCESS;
}

void AsyncMode::FlushStart()
{
    MEDIA_LOG_D("AsyncMode FlushStart entered.");
    if (inBufQue_) {
        inBufQue_->SetActive(false);
    }
    if (handleFrameTask_) {
        handleFrameTask_->PauseAsync();
    }
    if (pushTask_) {
        pushTask_->PauseAsync();
    }
    MEDIA_LOG_D("AsyncMode FlushStart exit.");
}

void AsyncMode::FlushEnd()
{
    MEDIA_LOG_I("AsyncMode FlushEnd entered");
    if (inBufQue_) {
        inBufQue_->SetActive(true);
    }
    if (handleFrameTask_) {
        handleFrameTask_->Start();
    }
    if (pushTask_) {
        pushTask_->Start();
    }
    if (plugin_) {
        QueueAllBufferInPoolToPluginLocked();
    }
}

ErrorCode AsyncMode::HandleFrame()
{
    MEDIA_LOG_D("AsyncMode handle frame called");
    auto oneBuffer = inBufQue_->Pop(200); // timeout 200 ms
    if (oneBuffer == nullptr) {
        MEDIA_LOG_W("decoder find nullptr in esBufferQ");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    Plugin::Status status = Plugin::Status::OK;
    do {
        status = plugin_->QueueInputBuffer(oneBuffer, 0);
        if (status == Plugin::Status::OK || status == Plugin::Status::END_OF_STREAM || stopped_) {
            break;
        }
        MEDIA_LOG_D("Send data to plugin error: " PUBLIC_LOG_D32, status);
        OSAL::SleepFor(DEFAULT_TRY_DECODE_TIME);
    } while (true);
    MEDIA_LOG_D("Async handle frame finished");
    return TranslatePluginStatus(status);
}

ErrorCode AsyncMode::FinishFrame()
{
    MEDIA_LOG_D("FinishFrame begin");
    bool isRendered = false;
    {
        OSAL::ScopedLock l(renderMutex_);
        std::shared_ptr<AVBuffer> frameBuffer = nullptr;
        if (!outBufQue_.empty()) {
            frameBuffer = outBufQue_.front();
        }
        if (frameBuffer != nullptr) {
            auto oPort = outPorts_[0];
            if (oPort->GetWorkMode() == WorkMode::PUSH) {
                oPort->PushData(frameBuffer, -1);
                isRendered = true;
                outBufQue_.pop();
            } else {
                MEDIA_LOG_W("decoder out port works in pull mode");
                return ErrorCode::ERROR_INVALID_OPERATION;
            }
            frameBuffer.reset();
        }
    }
    auto newOutBuffer = outBufPool_->AllocateBufferNonBlocking();
    if (CheckBufferValidity(newOutBuffer) == ErrorCode::SUCCESS) {
        newOutBuffer->Reset();
        plugin_->QueueOutputBuffer(newOutBuffer, 0);
    }
    if (!isRendered) {
        OSAL::SleepFor(DEFAULT_TRY_RENDER_TIME);
    }
    MEDIA_LOG_D("AsyncMode finish frame success");
    return ErrorCode::SUCCESS;
}

void AsyncMode::OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer>& buffer)
{
    OSAL::ScopedLock l(renderMutex_);
    outBufQue_.push(buffer);
}

ErrorCode AsyncMode::Prepare()
{
    MEDIA_LOG_I("AsyncMode prepare called.");
    if (!inBufQue_) {
        inBufQue_ = std::make_shared<BlockingQueue<AVBufferPtr>>("asyncFilterInBufQue", GetInBufferPoolSize());
    } else {
        inBufQue_->SetActive(true);
    }
    if (!handleFrameTask_) {
        handleFrameTask_ = std::make_shared<OSAL::Task>("asyncHandleFrameThread");
        handleFrameTask_->RegisterHandler([this] { (void)HandleFrame(); });
    }
    if (!pushTask_) {
        pushTask_ = std::make_shared<OSAL::Task>("asyncPushThread");
        pushTask_->RegisterHandler([this] { (void)FinishFrame(); });
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AsyncMode::QueueAllBufferInPoolToPluginLocked()
{
    ErrorCode err = ErrorCode::SUCCESS;
    while (!outBufPool_->Empty()) {
        auto buf = outBufPool_->AllocateBuffer();
        if (CheckBufferValidity(buf) != ErrorCode::SUCCESS) {
            MEDIA_LOG_W("cannot allocate buffer in buffer pool");
            continue;
        }
        err = TranslatePluginStatus(plugin_->QueueOutputBuffer(buf, -1));
        if (err != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("queue output buffer error");
        }
    }
    return err;
}

ErrorCode AsyncMode::CheckBufferValidity(std::shared_ptr<AVBuffer>& buffer)
{
    if (buffer == nullptr) {
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    auto memory = buffer->GetMemory();
    if (memory == nullptr) {
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
    if (memory->GetMemoryType() == Plugin::MemoryType::SURFACE_BUFFER) {
        std::shared_ptr<Plugin::SurfaceMemory> surfaceMemory =
            Plugin::ReinterpretPointerCast<Plugin::SurfaceMemory>(memory);

        // trigger surface memory to request surface buffer again when it is surface buffer
        FALSE_RETURN_V_MSG_E(surfaceMemory->GetSurfaceBuffer() != nullptr, ErrorCode::ERROR_NO_MEMORY,
                             "get surface buffer fail");
    }
#endif
    return ErrorCode::SUCCESS;
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS