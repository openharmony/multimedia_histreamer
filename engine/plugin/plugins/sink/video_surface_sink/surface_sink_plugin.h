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

#ifndef HISTREAMER_SURFACE_SINK_PLUGIN_H
#define HISTREAMER_SURFACE_SINK_PLUGIN_H

#ifdef VIDEO_SUPPORT

#include <atomic>
#include <memory>
#include "refbase.h"
#include "surface/surface.h"
#include "display_type.h"
#include "common/graphic_common.h"
#include "plugin/interface/video_sink_plugin.h"
#include "plugin/common/plugin_video_tags.h"

#ifdef DUMP_RAW_DATA
#include <fstream>
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
using PairAddr = std::pair<void*, sptr<SurfaceBuffer>>;
#define DEFAULT_WIDTH      640
#define DEFAULT_HEIGHT     480
#define DEFAULT_BUFFER_NUM 8

class SurfaceAllocator : public Allocator {
public:
    SurfaceAllocator(sptr<Surface> surface = nullptr)
    {
        surface_ = surface;
        surfaceMap_.reserve(DEFAULT_BUFFER_NUM);
    }
    ~SurfaceAllocator() override = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*

    void Config(int32_t width, int32_t height, int32_t usage, int32_t format, int32_t strideAlign)
    {
        requestConfig_ = {
            width, height, strideAlign, format,
            usage | HBM_USE_CPU_READ | HBM_USE_CPU_WRITE | HBM_USE_MEM_DMA, 0
        };
        bufferCnt_ = 0;
    }

    sptr<SurfaceBuffer> GetSurfaceBuffer(void* addr);

private:
    sptr<Surface> surface_ {nullptr};
    std::vector<PairAddr> surfaceMap_ {};
    BufferRequestConfig requestConfig_;
    uint32_t bufferCnt_ {0};
};

class SurfaceSinkPlugin : public VideoSinkPlugin, public std::enable_shared_from_this<SurfaceSinkPlugin> {
public:
    explicit SurfaceSinkPlugin(std::string name);
    ~SurfaceSinkPlugin() override = default;

    Status Init() override;

    Status Deinit() override;

    Status Prepare() override;

    Status Reset() override;

    Status Start() override;

    Status Stop() override;

    bool IsParameterSupported(Tag tag) override;

    Status GetParameter(Tag tag, ValueType &value) override;

    Status SetParameter(Tag tag, const ValueType &value) override;

    std::shared_ptr<Allocator> GetAllocator() override;

    Status SetCallback(Callback* cb) override;

    Status Pause() override;

    Status Resume() override;

    Status Write(const std::shared_ptr<Buffer> &input) override;

    Status Flush() override;

    Status GetLatency(uint64_t &nanoSec) override;

private:

    uint32_t width_;
    uint32_t height_;
    uint32_t stride_;
    VideoPixelFormat pixelFormat_;
    sptr<Surface> surface_ {nullptr};
    std::shared_ptr<SurfaceAllocator> mAllocator_ {nullptr};
    uint32_t maxSurfaceNum_;

#ifdef DUMP_RAW_DATA
    std::ofstream dumpData_;
#endif
};
}
}
}

#endif

#endif // MEDIA_PIPELINE_SDL_VIDEO_SINK_PLUGIN_H
