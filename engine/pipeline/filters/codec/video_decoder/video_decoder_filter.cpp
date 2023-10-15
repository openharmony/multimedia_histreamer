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

#ifdef VIDEO_SUPPORT

#define HST_LOG_TAG "VideoDecoderFilter"

#include "pipeline/filters/codec/video_decoder/video_decoder_filter.h"
#include "foundation/cpp_ext/memory_ext.h"
#include "foundation/log.h"
#include "foundation/osal/utils/util.h"
#include "foundation/utils/constants.h"
#include "foundation/utils/steady_clock.h"
#include "pipeline/factory/filter_factory.h"
#include "pipeline/filters/codec/codec_filter_factory.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/plugin_video_tags.h"
#include "plugin/common/surface_allocator.h"

namespace {
const uint32_t DEFAULT_IN_BUFFER_POOL_SIZE = 8;
const uint32_t DEFAULT_OUT_BUFFER_POOL_SIZE = 8;
const float VIDEO_PIX_DEPTH = 1.5;
const uint32_t VIDEO_ALIGN_SIZE = 16;
}

namespace OHOS {
namespace Media {
namespace Pipeline {
#ifdef OHOS_LITE
static AutoRegisterFilter<VideoDecoderFilter> g_registerVideoDecoderFilter("builtin.player.videodecoder",
    [](const std::string& name) { return CreateCodecFilter(name, FilterCodecMode::VIDEO_ASYNC_DECODER); });
#else
static AutoRegisterFilter<VideoDecoderFilter> g_registerVideoDecoderFilter("builtin.player.videodecoder",
    [](const std::string& name) { return CreateCodecFilter(name, FilterCodecMode::VIDEO_ASYNC_DECODER); });
#endif
VideoDecoderFilter::VideoDecoderFilter(const std::string& name, std::shared_ptr<CodecMode> codecMode)
    : CodecFilterBase(name)
{
    MEDIA_LOG_I("video decoder ctor called");
    filterType_ = FilterType::VIDEO_DECODER;
    bufferMetaType_ = Plugin::BufferMetaType::VIDEO;
    pluginType_ = Plugin::PluginType::VIDEO_DECODER;
    codecMode_ = std::move(codecMode);
}

VideoDecoderFilter::~VideoDecoderFilter()
{
    MEDIA_LOG_D("video decoder dtor called");
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
    (void)codecMode_->Release();
}

ErrorCode VideoDecoderFilter::Prepare()
{
    MEDIA_LOG_I("video decoder prepare called.");
    codecMode_->SetBufferPoolSize(static_cast<uint32_t>(DEFAULT_IN_BUFFER_POOL_SIZE),
                                  static_cast<uint32_t>(DEFAULT_OUT_BUFFER_POOL_SIZE));
    (void)codecMode_->Prepare();
    return CodecFilterBase::Prepare();
}

ErrorCode VideoDecoderFilter::Start()
{
    return CodecFilterBase::Start();
}

ErrorCode VideoDecoderFilter::Stop()
{
    MEDIA_LOG_D("video decoder stop start.");
    FAIL_RETURN(CodecFilterBase::Stop());
    MEDIA_LOG_D("video decoder stop end.");
    return ErrorCode::SUCCESS;
}

void VideoDecoderFilter::FlushStart()
{
    MEDIA_LOG_I("Video decoder FlushStart entered.");
    codecMode_->FlushStart();
    CodecFilterBase::FlushStart();
}

void VideoDecoderFilter::FlushEnd()
{
    MEDIA_LOG_I("Video decoder FlushEnd entered.");
    codecMode_->FlushEnd();
    CodecFilterBase::FlushEnd();
}

bool VideoDecoderFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta,
                                   Plugin::Meta& upstreamParams, Plugin::Meta& downstreamParams)
{
    PROFILE_BEGIN("video decoder configure begin");
    FALSE_RETURN_V(CodecFilterBase::Configure(inPort, upstreamMeta, upstreamParams, downstreamParams), false);
    PROFILE_END("video decoder configure end");
    return true;
}

bool VideoDecoderFilter::Negotiate(const std::string& inPort,
                                   const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                   Plugin::Capability& negotiatedCap,
                                   const Plugin::Meta& upstreamParams,
                                   Plugin::Meta& downstreamParams)
{
    FALSE_RETURN_V(CodecFilterBase::Negotiate(inPort, upstreamCap, negotiatedCap, upstreamParams, downstreamParams),
                   false);
    MEDIA_LOG_D("video decoder negotiate end");
    return true;
}

uint32_t VideoDecoderFilter::GetOutBufferPoolSize()
{
    return DEFAULT_OUT_BUFFER_POOL_SIZE;
}

uint32_t VideoDecoderFilter::CalculateBufferSize(const std::shared_ptr<const Plugin::Meta>& meta)
{
    uint32_t bufferSize = 0;
    uint32_t vdecWidth;
    uint32_t vdecHeight;
    Plugin::VideoPixelFormat vdecFormat;

    FALSE_RETURN_V(meta->Get<Plugin::Tag::VIDEO_WIDTH>(vdecWidth), 0);
    FALSE_RETURN_V(meta->Get<Plugin::Tag::VIDEO_HEIGHT>(vdecHeight), 0);
    FALSE_RETURN_V(meta->Get<Plugin::Tag::VIDEO_PIXEL_FORMAT>(vdecFormat), 0);

    // YUV420: size = stride * height * 1.5
    uint32_t stride = Plugin::AlignUp(vdecWidth, VIDEO_ALIGN_SIZE);
    if (vdecFormat == Plugin::VideoPixelFormat::YUV420P ||
        vdecFormat == Plugin::VideoPixelFormat::NV21 ||
        vdecFormat == Plugin::VideoPixelFormat::NV12) {
        bufferSize = static_cast<uint32_t>(Plugin::AlignUp(stride, VIDEO_ALIGN_SIZE) *
                                           Plugin::AlignUp(vdecHeight, VIDEO_ALIGN_SIZE) * VIDEO_PIX_DEPTH);
        MEDIA_LOG_D("YUV output buffer size: " PUBLIC_LOG_U32, bufferSize);
    } else if (vdecFormat == Plugin::VideoPixelFormat::RGBA ||
               vdecFormat == Plugin::VideoPixelFormat::ARGB ||
               vdecFormat == Plugin::VideoPixelFormat::ABGR ||
               vdecFormat == Plugin::VideoPixelFormat::BGRA) {
        bufferSize = static_cast<uint32_t>(Plugin::AlignUp(stride, VIDEO_ALIGN_SIZE) *
                                            Plugin::AlignUp(vdecHeight, VIDEO_ALIGN_SIZE) * 4); // 4: 32bit
        MEDIA_LOG_D("RGBA output buffer size: " PUBLIC_LOG_U32, bufferSize);
    } else {
        // need to check video sink support and calc buffer size
        MEDIA_LOG_E("Unsupported video pixel format: " PUBLIC_LOG_U32, vdecFormat);
    }
    return bufferSize;
}

Plugin::Meta VideoDecoderFilter::GetNegotiateParams(const Plugin::Meta& upstreamParams)
{
    // video, need to get the max buffer num from plugin capability when use hdi as codec plugin interfaces
    Plugin::Meta proposeParams = upstreamParams;
    proposeParams.Set<Plugin::Tag::VIDEO_MAX_SURFACE_NUM>(DEFAULT_OUT_BUFFER_POOL_SIZE);
    return proposeParams;
}

std::shared_ptr<Allocator> VideoDecoderFilter::GetAllocator()
{
#ifndef OHOS_LITE
    // Use sink allocator first, zero copy while passing data
    Plugin::Tag tag = Plugin::Tag::BUFFER_ALLOCATOR;
    auto ite = sinkParams_.Find(tag);
    if (ite != std::end(sinkParams_)) {
        if (Plugin::Any::IsSameTypeWith<std::shared_ptr<Plugin::SurfaceAllocator>>(ite->second)) {
            MEDIA_LOG_D("Get SurfaceAllocator from sink");
            return Plugin::AnyCast<std::shared_ptr<Plugin::SurfaceAllocator>>(ite->second);
        }
    }
#endif
    return plugin_->GetAllocator();
}

void VideoDecoderFilter::UpdateParams(const std::shared_ptr<const Plugin::Meta>& upMeta,
                                      std::shared_ptr<Plugin::Meta>& meta)
{
    MEDIA_LOG_D("UpdateParams begin");
}

void VideoDecoderFilter::OnInputBufferDone(const std::shared_ptr<Plugin::Buffer>& input)
{
    MEDIA_LOG_DD("VideoDecoderFilter::OnInputBufferDone");
}

void VideoDecoderFilter::OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer>& output)
{
    codecMode_->OnOutputBufferDone(output);
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif
