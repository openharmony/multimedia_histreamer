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

#ifndef HISTREAMER_VIDEO_CAPTURE_PLUGIN_H
#define HISTREAMER_VIDEO_CAPTURE_PLUGIN_H

#if !defined(OHOS_LITE) && defined(RECORDER_SUPPORT) && defined(VIDEO_SUPPORT)

#include <atomic>
#include "foundation/osal/thread/condition_variable.h"
#include "foundation/osal/thread/mutex.h"
#include "plugin/common/plugin_types.h"
#include "plugin/interface/source_plugin.h"
#include "refbase.h"
#include "surface/surface.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace VideoCapture {
class VideoCaptureAllocator : public Plugin::Allocator {
public:
    VideoCaptureAllocator() = default;
    ~VideoCaptureAllocator() override = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*
};



class VideoCapturePlugin : public Plugin::SourcePlugin {
public:
    explicit VideoCapturePlugin(std::string name);
    ~VideoCapturePlugin() override;

    Plugin::Status Init() override;
    Plugin::Status Deinit() override;
    Plugin::Status Prepare() override;
    Plugin::Status Reset() override;
    Plugin::Status Start() override;
    Plugin::Status Stop() override;
    bool IsParameterSupported(Plugin::Tag tag) override;
    Plugin::Status GetParameter(Plugin::Tag tag, Plugin::ValueType& value) override;
    Plugin::Status SetParameter(Plugin::Tag tag, const Plugin::ValueType& value) override;
    std::shared_ptr<Plugin::Allocator> GetAllocator() override;
    Plugin::Status SetCallback(Plugin::Callback* cb) override;
    Plugin::Status SetSource(std::shared_ptr<Plugin::MediaSource> source) override;
    Plugin::Status Read(std::shared_ptr<Plugin::Buffer>& buffer, size_t expectedLen) override;
    Plugin::Status GetSize(size_t& size) override;
    bool IsSeekable() override;
    Plugin::Status SeekTo(uint64_t offset) override;

protected:
    class SurfaceConsumerListener : public IBufferConsumerListener {
    public:
        explicit SurfaceConsumerListener(VideoCapturePlugin &owner) : owner_(owner) {}
        ~SurfaceConsumerListener() = default;
        void OnBufferAvailable() override;
    private:
        VideoCapturePlugin &owner_;
    };

private:
    void ConfigSurfaceConsumer();
    Status AcquireSurfaceBuffer();
    void OnBufferAvailable();

    OHOS::Media::OSAL::Mutex mutex_ {};
    OSAL::ConditionVariable readCond_;
    std::shared_ptr<VideoCaptureAllocator> mAllocator_ {nullptr};
    sptr<Surface> surfaceConsumer_ {nullptr};
    sptr<Surface> surfaceProducer_ {nullptr};
    std::atomic<bool> isStop_ {false};
    uint32_t width_ {0};
    uint32_t height_ {0};
    uint32_t bufferCnt_ {0};
    uint64_t curTimestampNs_ {0};
    uint64_t stopTimestampNs_ {0};
    uint64_t totalPauseTimeNs_ {0};

    sptr<SurfaceBuffer> surfaceBuffer_ {nullptr};
    int32_t fence_ {-1};
    int64_t timestamp_ {0};
    int32_t bufferSize_ {0};
    Rect damage_;
    int32_t isKeyFrame_ {0};
};
} // namespace VideoCapture
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif
#endif // HISTREAMER_VIDEO_CAPTURE_PLUGIN_H

