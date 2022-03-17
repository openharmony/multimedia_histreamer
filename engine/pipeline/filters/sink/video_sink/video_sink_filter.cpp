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

#define HST_LOG_TAG "VideoSinkFilter"

#include "video_sink_filter.h"

#include "common/plugin_utils.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"
#include "osal/utils/util.h"
#include "pipeline/core/clock_manager.h"
#include "plugin/common/plugin_time.h"
#include "plugin/common/surface_allocator.h"
#include "utils/steady_clock.h"

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
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("SetParameter key " PUBLIC_LOG_D32 "is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode VideoSinkFilter::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("GetParameter key " PUBLIC_LOG_D32 "is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->GetParameter(tag, value));
}

void VideoSinkFilter::HandleNegotiateParams(const Plugin::TagMap& upstreamParams, Plugin::TagMap& downstreamParams)
{
#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
    Plugin::Tag tag = Plugin::Tag::VIDEO_MAX_SURFACE_NUM;
    auto ite = upstreamParams.find(tag);
    if (ite != std::end(upstreamParams)) {
        if (ite->second.SameTypeWith(typeid(uint32_t))) {
            auto ret = plugin_->SetParameter(tag, Plugin::AnyCast<uint32_t>(ite->second));
            if (ret != Plugin::Status::OK) {
                MEDIA_LOG_W("Set max surface num to plugin fail");
            }
        }
    }
    auto pluginAllocator = plugin_->GetAllocator();
    if (pluginAllocator != nullptr && pluginAllocator->GetMemoryType() == Plugin::MemoryType::SURFACE_BUFFER) {
        auto allocator = Plugin::ReinterpretPointerCast<Plugin::SurfaceAllocator>(pluginAllocator);
        downstreamParams.emplace(std::make_pair(Tag::BUFFER_ALLOCATOR, allocator));
    }
#endif
}

bool VideoSinkFilter::CreateVideoSinkPlugin(const std::shared_ptr<Plugin::PluginInfo>& selectedPluginInfo)
{
    if (plugin_ != nullptr) {
        if (pluginInfo_ != nullptr && pluginInfo_->name == selectedPluginInfo->name) {
            if (plugin_->Reset() == Plugin::Status::OK) {
                return true;
            }
            MEDIA_LOG_W("reuse previous plugin " PUBLIC_LOG_S " failed, will create new plugin",
                        pluginInfo_->name.c_str());
        }
        plugin_->Deinit();
    }

    plugin_ = Plugin::PluginManager::Instance().CreateVideoSinkPlugin(selectedPluginInfo->name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("cannot create plugin " PUBLIC_LOG_S, selectedPluginInfo->name.c_str());
        return false;
    }

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
    if (surface_ != nullptr) {
        auto ret = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_SURFACE, surface_));
        if (ret != ErrorCode::SUCCESS) {
            MEDIA_LOG_W("Set surface to plugin fail, ret: " PUBLIC_LOG_U32, ret);
            return false;
        }
    }
#endif

    auto err = TranslatePluginStatus(plugin_->Init());
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("plugin " PUBLIC_LOG "s init error", selectedPluginInfo->name.c_str());
        return false;
    }
    pluginInfo_ = selectedPluginInfo;
    return true;
}

bool VideoSinkFilter::Negotiate(const std::string& inPort,
                                const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                Plugin::Capability& negotiatedCap,
                                const Plugin::TagMap& upstreamParams,
                                Plugin::TagMap& downstreamParams)
{
    PROFILE_BEGIN("video sink negotiate start");
    if (state_ != FilterState::PREPARING) {
        MEDIA_LOG_W("Video sink filter is not in preparing when negotiate");
        return false;
    }
    auto candidatePlugins = FindAvailablePlugins(*upstreamCap, Plugin::PluginType::VIDEO_SINK);
    if (candidatePlugins.empty()) {
        MEDIA_LOG_E("no available video sink plugin");
        return false;
    }
    // always use first one
    std::shared_ptr<Plugin::PluginInfo> selectedPluginInfo = candidatePlugins[0].first;
    MEDIA_LOG_E("select plugin " PUBLIC_LOG_S, selectedPluginInfo->name.c_str());
    for (const auto& onCap : selectedPluginInfo->inCaps) {
        if (onCap.keys.count(CapabilityID::VIDEO_PIXEL_FORMAT) == 0) {
            MEDIA_LOG_E("each in caps of sink must contains valid video pixel format");
            return false;
        }
    }
    negotiatedCap = candidatePlugins[0].second;
    if (!CreateVideoSinkPlugin(selectedPluginInfo)) {
        return false;
    }
    HandleNegotiateParams(upstreamParams, downstreamParams);
    PROFILE_END("video sink negotiate end");
    return true;
}

bool VideoSinkFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta)
{
    PROFILE_BEGIN("video sink configure start");
    if (plugin_ == nullptr || pluginInfo_ == nullptr) {
        MEDIA_LOG_E("cannot configure decoder when no plugin available");
        return false;
    }
    auto err = ConfigureNoLocked(upstreamMeta);
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("sink configure error");
        OnEvent(Event{name_, EventType::EVENT_ERROR, {err}});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent(Event{name_, EventType::EVENT_READY, {}});
    MEDIA_LOG_I("video sink send EVENT_READY");
    PROFILE_END("video sink configure end");
    return true;
}

ErrorCode VideoSinkFilter::ConfigurePluginParams(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = ErrorCode::SUCCESS;
    uint32_t width;
    if (meta->GetUint32(Plugin::MetaID::VIDEO_WIDTH, width)) {
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_WIDTH, width));
        FAIL_RETURN_MSG(err, "Set plugin width fail");
    }
    uint32_t height;
    if (meta->GetUint32(Plugin::MetaID::VIDEO_HEIGHT, height)) {
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_HEIGHT, height));
        FAIL_RETURN_MSG(err, "Set plugin height fail");
    }
    Plugin::VideoPixelFormat pixelFormat;
    if (meta->GetData<Plugin::VideoPixelFormat>(Plugin::MetaID::VIDEO_PIXEL_FORMAT, pixelFormat)) {
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_PIXEL_FORMAT, pixelFormat));
        FAIL_RETURN_MSG(err, "Set plugin pixel format fail");
    }
    MEDIA_LOG_D("width: " PUBLIC_LOG_U32 ", height: " PUBLIC_LOG_U32 ", pixelFormat: " PUBLIC_LOG_U32,
                width, height, pixelFormat);
    return err;
}

ErrorCode VideoSinkFilter::ConfigureNoLocked(const std::shared_ptr<const Plugin::Meta>& meta)
{
    FAIL_RETURN_MSG(TranslatePluginStatus(plugin_->Init()), "Init plugin error");
    plugin_->SetCallback(this);
    FAIL_RETURN_MSG(ConfigurePluginParams(meta), "Configure plugin params fail");
    FAIL_RETURN_MSG(TranslatePluginStatus(plugin_->Prepare()), "Prepare plugin fail");
    FAIL_RETURN_MSG(TranslatePluginStatus(plugin_->Start()), "Start plugin fail");
    return ErrorCode::SUCCESS;
}

bool VideoSinkFilter::DoSync(int64_t pts) const
{
    int64_t  delta {0};
    int64_t  tempOut {0};
    if (frameCnt_ == 0 || pts == 0) {
        return true;
    }
    if (ClockManager::Instance().GetProvider().CheckPts(pts, delta) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("DoSync CheckPts fail");
        return false;
    }
    if (delta > 0) {
        tempOut = Plugin::HstTime2Ms(delta);
        if (tempOut > 100) { // 100ms
            MEDIA_LOG_D("DoSync early " PUBLIC_LOG_D64 " ms", tempOut);
            OHOS::Media::OSAL::SleepFor(tempOut);
            return true;
        }
    } else if (delta < 0) {
        tempOut = Plugin::HstTime2Ms(-delta);
        if (tempOut > 40) { // 40ms drop frame
            MEDIA_LOG_E("DoSync later " PUBLIC_LOG_D64 " ms", tempOut);
            return false;
        }
    }
    return true;
}

void VideoSinkFilter::RenderFrame()
{
    uint64_t latencyNano {0};
    MEDIA_LOG_D("RenderFrame called");
    auto oneBuffer = inBufQueue_->Pop();
    if (oneBuffer == nullptr) {
        MEDIA_LOG_W("Video sink find nullptr in esBufferQ");
        return;
    }

    Plugin::Status status = plugin_->GetLatency(latencyNano);
    if (status != Plugin::Status::OK) {
        MEDIA_LOG_E("Video sink GetLatency fail errorcode = " PUBLIC_LOG_D32,
                    CppExt::to_underlying(TranslatePluginStatus(status)));
        return;
    }

    if (INT64_MAX - latencyNano < oneBuffer->pts) {
        MEDIA_LOG_E("Video pts(" PUBLIC_LOG_U64 ") + latency overflow.", oneBuffer->pts);
        return;
    }

    if (!DoSync(oneBuffer->pts + latencyNano)) {
        return;
    }
    auto err = plugin_->Write(oneBuffer);
    if (err != Plugin::Status::OK) {
        MEDIA_LOG_E("Video sink write failed");
        return;
    }
}

ErrorCode VideoSinkFilter::PushData(const std::string& inPort, const AVBufferPtr& buffer, int64_t offset)
{
    MEDIA_LOG_D("video sink push data started, state_: " PUBLIC_LOG_D32, state_.load());
    frameCnt_++;
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
        MEDIA_LOG_I("PushData return due to: isFlushing_ = " PUBLIC_LOG_D32 ", state_ = " PUBLIC_LOG_D32,
                    isFlushing_, static_cast<int>(state_.load()));
        return ErrorCode::SUCCESS;
    }

    if (buffer->GetMemory()->GetSize() == 0) {
        Event event{
            .srcFilter = name_,
            .type = EventType::EVENT_COMPLETE,
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
        MEDIA_LOG_W("sink is not ready when start, state_: " PUBLIC_LOG_D32, state_.load());
        return ErrorCode::ERROR_INVALID_OPERATION;
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
    FAIL_RETURN_MSG(FilterBase::Stop(), "Video sink stop fail");
    FAIL_RETURN_MSG(TranslatePluginStatus(plugin_->Stop()), "Stop plugin fail");
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
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    FAIL_RETURN_MSG(FilterBase::Pause(), "Video sink pause fail");
    FAIL_RETURN_MSG(TranslatePluginStatus(plugin_->Pause()), "Pause plugin fail");
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
        frameCnt_ = 0;
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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
ErrorCode VideoSinkFilter::SetVideoSurface(sptr<Surface> surface)
{
    if (!surface) {
        MEDIA_LOG_W("surface is null");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if (plugin_) {
        auto ret = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_SURFACE, surface));
        if (ret != ErrorCode::SUCCESS) {
            MEDIA_LOG_W("Set surface to plugin fail");
            return ret;
        }
    } else {
        surface_ = surface;
    }
    return ErrorCode::SUCCESS;
}
#endif
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif