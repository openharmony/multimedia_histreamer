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

#define HST_LOG_TAG "SurfaceSinkPlugin"

#include "surface_sink_plugin.h"
#include <functional>
#include <algorithm>
#include "foundation/log.h"
#include "utils/constants.h"
#include "plugin/common/surface_buffer.h"

namespace {
using namespace OHOS::Media::Plugin;

std::shared_ptr<VideoSinkPlugin> VideoSinkPluginCreator(const std::string& name)
{
    return std::make_shared<SurfaceSinkPlugin>(name);
}

const Status SurfaceSinkRegister(const std::shared_ptr<Register>& reg)
{
    VideoSinkPluginDef definition;
    definition.name = "surface_sink";
    definition.rank = 100; // 100
    Capability cap(OHOS::Media::MEDIA_MIME_VIDEO_RAW);
    cap.AppendDiscreteKeys<VideoPixelFormat>(
        Capability::Key::VIDEO_PIXEL_FORMAT,
        {VideoPixelFormat::NV21, VideoPixelFormat::RGB24});
    definition.inCaps.emplace_back(cap);
    definition.creator = VideoSinkPluginCreator;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(SurfaceSink, LicenseType::APACHE_V2, SurfaceSinkRegister, [] {});
} // namespace

namespace OHOS {
namespace Media {
namespace Plugin {
static uint32_t TranslatePixelFormat(const VideoPixelFormat pixelFormat)
{
    uint32_t surfaceFormat = PixelFormat::PIXEL_FMT_BUTT;
    switch (pixelFormat) {
        case VideoPixelFormat::YUV420P:
            surfaceFormat = PixelFormat::PIXEL_FMT_YCBCR_420_P;
            break;
        case VideoPixelFormat::YUYV422:
            surfaceFormat = PixelFormat::PIXEL_FMT_YUYV_422_PKG;
            break;
        case VideoPixelFormat::RGB24:
            surfaceFormat = PixelFormat::PIXEL_FMT_RGBA_8888;
            break;
        case VideoPixelFormat::BGR24:
            surfaceFormat = PixelFormat::PIXEL_FMT_BGRA_8888;
            break;
        case VideoPixelFormat::YUV422P:
            surfaceFormat = PixelFormat::PIXEL_FMT_YUV_422_I;
            break;
        case VideoPixelFormat::YUV444P:
        case VideoPixelFormat::YUV410P:
        case VideoPixelFormat::YUV411P:
        case VideoPixelFormat::GRAY8:
        case VideoPixelFormat::MONOWHITE:
        case VideoPixelFormat::MONOBLACK:
        case VideoPixelFormat::PAL8:
        case VideoPixelFormat::YUVJ420P:
        case VideoPixelFormat::YUVJ422P:
        case VideoPixelFormat::YUVJ444P:
            break;
        case VideoPixelFormat::NV12:
            surfaceFormat = PixelFormat::PIXEL_FMT_YCBCR_420_SP;
            break;
        case VideoPixelFormat::NV21:
            surfaceFormat = PixelFormat::PIXEL_FMT_YCRCB_420_SP;
            break;
        default:
            break;
    }
    return surfaceFormat;
}

SurfaceSinkPlugin::SurfaceSinkPlugin(std::string name)
    : VideoSinkPlugin(std::move(name)),
      width_(DEFAULT_WIDTH),
      height_(DEFAULT_HEIGHT),
      pixelFormat_(VideoPixelFormat::NV21),
      maxSurfaceNum_(DEFAULT_BUFFER_NUM)
{
}

Status SurfaceSinkPlugin::Init()
{
    std::weak_ptr<SurfaceSinkPlugin> weakPtr(shared_from_this());
#ifdef DUMP_RAW_DATA
    dumpData_.open("./vsink_out.dat", std::ios::out | std::ios::binary);
#endif

    return Status::OK;
}

Status SurfaceSinkPlugin::Deinit()
{
#ifdef DUMP_RAW_DATA
    dumpData_.close();
#endif
    return Status::OK;
}


Status SurfaceSinkPlugin::Prepare()
{
    if (!surface_) {
        MEDIA_LOG_E("need surface config first");
        return Status::ERROR_UNKNOWN;
    }
    auto ret = surface_->SetQueueSize(maxSurfaceNum_);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        MEDIA_LOG_E("surface SetQueueSize fail");
        return Status::ERROR_UNKNOWN;
    }
    const constexpr int32_t strideAlign = 8; // 8
    auto format = TranslatePixelFormat(pixelFormat_);
    if (format == PixelFormat::PIXEL_FMT_BUTT) {
        MEDIA_LOG_E("surface can not support pixel format: %u", pixelFormat_);
        return Status::ERROR_UNKNOWN;
    }
    mAllocator_->Config(static_cast<int32_t>(width_), static_cast<int32_t>(height_), 0, format,
                        strideAlign);
    return Status::OK;
}

Status SurfaceSinkPlugin::Reset()
{
#ifdef DUMP_RAW_DATA
    dumpData_.close();
#endif
    return Status::OK;
}

Status SurfaceSinkPlugin::Start()
{
    MEDIA_LOG_I("video sink start ...");
    return Status::OK;
}

Status SurfaceSinkPlugin::Stop()
{
    MEDIA_LOG_I("video sink stop ...");
    return Status::OK;
}

bool SurfaceSinkPlugin::IsParameterSupported(Tag tag)
{
    return false;
}

Status SurfaceSinkPlugin::GetParameter(Tag tag, ValueType& value)
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status SurfaceSinkPlugin::SetParameter(Tag tag, const ValueType& value)
{
    switch (tag) {
        case Tag::VIDEO_WIDTH: {
            if (value.Type() == typeid(uint32_t)) {
                width_ = Plugin::AnyCast<uint32_t>(value);
                MEDIA_LOG_D("pixelWidth_: %u", pixelWidth_);
            }
            break;
        }
        case Tag::VIDEO_HEIGHT: {
            if (value.Type() == typeid(uint32_t)) {
                height_ = Plugin::AnyCast<uint32_t>(value);
                MEDIA_LOG_D("pixelHeight_: %u", pixelHeight_);
            }
            break;
        }
        case Tag::VIDEO_PIXEL_FORMAT: {
            if (value.Type() == typeid(VideoPixelFormat)) {
                pixelFormat_ = Plugin::AnyCast<VideoPixelFormat>(value);
                MEDIA_LOG_D("pixelFormat: %u", pixelFormat_);
            }
            break;
        }
        case Tag::VIDEO_SURFACE: {
            if (value.Type() == typeid(sptr<Surface>)) {
                surface_ = Plugin::AnyCast<sptr<Surface>>(value);
                if (!surface_) {
                    MEDIA_LOG_E("surface is null");
                    return Status::ERROR_INVALID_PARAMETER;
                }
                mAllocator_ = std::make_shared<SurfaceAllocator>(surface_);
            }
            break;
        }
        case Tag::VIDEO_MAX_SURFACE_NUM: {
            if (value.Type() == typeid(uint32_t)) {
                maxSurfaceNum_ = Plugin::AnyCast<uint32_t>(value);
                MEDIA_LOG_D("maxSurfaceNum_: %u", maxSurfaceNum_);
            }
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

std::shared_ptr<Allocator> SurfaceSinkPlugin::GetAllocator()
{
    return mAllocator_;
}

Status SurfaceSinkPlugin::SetCallback(Callback* cb)
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status SurfaceSinkPlugin::Pause()
{
    return Status::OK;
}

Status SurfaceSinkPlugin::Resume()
{
    return Status::OK;
}

Status SurfaceSinkPlugin::Write(const std::shared_ptr<Buffer>& inputInfo)
{
    MEDIA_LOG_D("sink write begin");
    if (inputInfo == nullptr || inputInfo->IsEmpty()) {
        return Status::OK;
    }
    auto memory = inputInfo->GetMemory();
    FALSE_RETURN_V(memory != nullptr, Status::ERROR_NULL_POINTER);
    FALSE_RETURN_V(memory->GetMemoryType() == MemoryType::SURFACE_BUFFER, Status::ERROR_INVALID_PARAMETER);
    std::shared_ptr<SurfaceMemory> surfaceMemory = std::dynamic_pointer_cast<SurfaceMemory>(memory);
    auto surfaceBuffer = surfaceMemory->GetSurfaceBuffer();
    FALSE_RETURN_V(surfaceBuffer != nullptr, Status::ERROR_NULL_POINTER);
    OHOS::BufferFlushConfig flushConfig = {
        {0, 0, static_cast<int32_t>(width_), static_cast<int32_t>(height_)},
    };
    auto ret = surface_->FlushBuffer(surfaceBuffer, surfaceMemory->GetFenceFd(), flushConfig);
    FALSE_RETURN_V(ret == OHOS::SurfaceError::SURFACE_ERROR_OK, Status::ERROR_UNKNOWN);
    return Status::OK;
}

Status SurfaceSinkPlugin::Flush()
{
    return Status::OK;
}

Status SurfaceSinkPlugin::GetLatency(uint64_t& nanoSec)
{
    nanoSec = 10; // 10 ns
    return Status::OK;
}

} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif