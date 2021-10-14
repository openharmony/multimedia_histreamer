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

#ifdef VIDEO_SUPPORT

#define LOG_TAG "VideoSinkFilter"

#include "video_sink_filter.h"

#include "common/plugin_utils.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<VideoSinkFilter> g_registerFilterHelper("builtin.player.videosink");

const uint32_t VSINK_DEFAULT_BUFFER_NUM = 8;

VideoSinkFilter::VideoSinkFilter(const std::string& name) : FilterBase(name)
{
    MEDIA_LOG_I("VideoSinkFilter ctor called...");
}

VideoSinkFilter::~VideoSinkFilter()
{
    MEDIA_LOG_D("VideoSinkFilter deCtor.");
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
    if (inBufQueue_ != nullptr) {
        inBufQueue_->SetActive(false);
        inBufQueue_.reset();
    }
}

void VideoSinkFilter::Init(EventReceiver* receiver, FilterCallback* callback)
{
    FilterBase::Init(receiver, callback);
    outPorts_.clear();
    if (inBufQueue_ == nullptr) {
        inBufQueue_ = std::make_shared<BlockingQueue<AVBufferPtr>>("VideoSinkInBufQue", VSINK_DEFAULT_BUFFER_NUM);
    }
    if (renderTask_ == nullptr) {
        renderTask_ = std::make_shared<OHOS::Media::OSAL::Task>("VideoSinkRenderThread");
        renderTask_->RegisterHandler([this] { RenderFrame(); });
    }
}

ErrorCode VideoSinkFilter::SetParameter(int32_t key, const Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ERROR_STATE;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("SetParameter key %d is out of boundary", key);
        return INVALID_PARAM_VALUE;
    }
    RETURN_PLUGIN_NOT_FOUND_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode VideoSinkFilter::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ERROR_STATE;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("GetParameter key %d is out of boundary", key);
        return INVALID_PARAM_VALUE;
    }
    RETURN_PLUGIN_NOT_FOUND_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->GetParameter(tag, value));
}

bool VideoSinkFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& inMeta,
                                CapabilitySet& outCaps)
{
    MEDIA_LOG_D("video sink negotiate started");
    if (state_ != FilterState::PREPARING) {
        MEDIA_LOG_W("Video sink filter is not in preparing when negotiate");
        return false;
    }
    auto creator = [](const std::string& pluginName) {
        return Plugin::PluginManager::Instance().CreateVideoSinkPlugin(pluginName);
    };
    auto err =
        FindPluginAndUpdate<Plugin::VideoSink>(inMeta, Plugin::PluginType::VIDEO_SINK, plugin_, pluginInfo_, creator);
    RETURN_TARGET_ERR_MESSAGE_LOG_IF_FAIL(err, false, "cannot find matched video sink plugin");
    outCaps = pluginInfo_->inCaps;
    err = ConfigureLocked(inMeta);
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("sink configure error");
        Event event{
            .type = EventType::EVENT_ERROR,
            .param = err,
        };
        OnEvent(event);
        return false;
    }
    state_ = FilterState::READY;
    Event event{
        .type = EVENT_READY,
    };
    OnEvent(event);
    MEDIA_LOG_I("video sink send EVENT_READY");
    return true;
}

ErrorCode VideoSinkFilter::ConfigurePluginParams(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = ErrorCode::SUCCESS;
    uint32_t width;
    if (meta->GetUint32(Plugin::MetaID::VIDEO_WIDTH, width)) {
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_WIDTH, width));
        RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "Set plugin width fail");
    }
    uint32_t height;
    if (meta->GetUint32(Plugin::MetaID::VIDEO_HEIGHT, height)) {
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_HEIGHT, height));
        RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "Set plugin height fail");
    }
    uint32_t pixelFormat;
    if (meta->GetUint32(Plugin::MetaID::VIDEO_PIXEL_FORMAT, pixelFormat)) {
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_PIXEL_FORMAT, pixelFormat));
        RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "Set plugin pixel format fail");
    }
    MEDIA_LOG_D("width: %u, height: %u, pixelFormat: %u", width, height, pixelFormat);
    return err;
}

ErrorCode VideoSinkFilter::ConfigureLocked(const std::shared_ptr<const Plugin::Meta>& meta)
{
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Init()), "Init plugin error");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(ConfigurePluginParams(meta), "Configure plugin params fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Prepare()), "Prepare plugin fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Start()), "Start plugin fail");
    return ErrorCode::SUCCESS;
}

bool VideoSinkFilter::DoSync()
{
    // Add av sync handle here
    return true;
}

void VideoSinkFilter::RenderFrame()
{
    MEDIA_LOG_D("RenderFrame called");
    auto oneBuffer = inBufQueue_->Pop();
    if (oneBuffer == nullptr) {
        MEDIA_LOG_W("Video sink find nullptr in esBufferQ");
        return;
    }
    if (DoSync() == false) {
        return;
    }
    auto err = plugin_->Write(oneBuffer);
    if (err != Plugin::Status::OK) {
        MEDIA_LOG_E("Video sink write failed");
        return;
    }
}

ErrorCode VideoSinkFilter::PushData(const std::string& inPort, AVBufferPtr buffer)
{
    MEDIA_LOG_D("video sink push data started, state_: %d", state_.load());
    if (isFlushing_ || state_.load() == FilterState::INITIALIZED) {
        MEDIA_LOG_I("video sink is flushing ignore this buffer");
        return ErrorCode::SUCCESS;
    }
    if (state_.load() != FilterState::RUNNING) {
        pushThreadIsBlocking_ = true;
        OSAL::ScopedLock lock(mutex_);
        startWorkingCondition_.Wait(lock, [this] {
            return state_ == FilterState::RUNNING || state_ == FilterState::INITIALIZED || isFlushing_;
        });
        pushThreadIsBlocking_ = false;
    }
    if (isFlushing_ || state_.load() == FilterState::INITIALIZED) {
        MEDIA_LOG_I("PushData return due to: isFlushing_ = %d, state_ = %d", isFlushing_,
                    static_cast<int>(state_.load()));
        return ErrorCode::SUCCESS;
    }

    if (buffer->GetMemory()->GetSize() == 0) {
        Event event{
            .type = EVENT_COMPLETE,
        };
        MEDIA_LOG_D("video sink push data send event_complete");
        OnEvent(event);
        MEDIA_LOG_D("video sink push data end");
        return ErrorCode::SUCCESS;
    }
    inBufQueue_->Push(buffer);
    MEDIA_LOG_D("video sink push data end");
    return ErrorCode::SUCCESS;
}

ErrorCode VideoSinkFilter::Start()
{
    MEDIA_LOG_D("start called");
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_W("sink is not ready when start, state_: %d", state_.load());
        return ERROR_STATE;
    }
    inBufQueue_->SetActive(true);
    renderTask_->Start();
    auto err = FilterBase::Start();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Video sink filter start error");
        return err;
    }
    plugin_->Start();
    if (pushThreadIsBlocking_.load()) {
        startWorkingCondition_.NotifyOne();
    }
    return ErrorCode::SUCCESS;
}

ErrorCode VideoSinkFilter::Stop()
{
    MEDIA_LOG_I("VideoSinkFilter stop called.");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(FilterBase::Stop(), "Video sink stop fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Stop()), "Stop plugin fail");
    if (pushThreadIsBlocking_.load()) {
        startWorkingCondition_.NotifyOne();
    }
    inBufQueue_->SetActive(false);
    renderTask_->Pause();
    return ErrorCode::SUCCESS;
}

ErrorCode VideoSinkFilter::Pause()
{
    MEDIA_LOG_D("Video sink filter pause start");
    if (state_ != FilterState::READY && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("video sink cannot pause when not working");
        return ERROR_STATE;
    }
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(FilterBase::Pause(), "Video sink pause fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Pause()), "Pause plugin fail");
    inBufQueue_->SetActive(false);
    renderTask_->Pause();
    MEDIA_LOG_D("Video sink filter pause end");
    return ErrorCode::SUCCESS;
}

ErrorCode VideoSinkFilter::Resume()
{
    MEDIA_LOG_D("Video sink filter resume");
    // only worked when state_ is paused
    if (state_ == FilterState::PAUSED) {
        state_ = FilterState::RUNNING;
        if (pushThreadIsBlocking_) {
            startWorkingCondition_.NotifyOne();
        }
        auto err = TranslatePluginStatus(plugin_->Resume());
        if (err != ErrorCode::SUCCESS) {
            return err;
        }
        inBufQueue_->SetActive(true);
        renderTask_->Start();
    }
    return ErrorCode::SUCCESS;
}

void VideoSinkFilter::FlushStart()
{
    MEDIA_LOG_D("FlushStart entered");
    isFlushing_ = true;
    if (pushThreadIsBlocking_) {
        startWorkingCondition_.NotifyOne();
    }
    if (inBufQueue_) {
        inBufQueue_->SetActive(false);
    }
    auto err = TranslatePluginStatus(plugin_->Flush());
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_W("Video sink filter flush end");
    }
}

void VideoSinkFilter::FlushEnd()
{
    MEDIA_LOG_D("FlushEnd entered");
    isFlushing_ = false;
    if (inBufQueue_) {
        inBufQueue_->SetActive(true);
    }
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif