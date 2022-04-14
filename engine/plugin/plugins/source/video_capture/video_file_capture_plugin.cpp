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

#include "video_file_capture_plugin.h"

#if !defined(OHOS_LITE) && defined(RECORDER_SUPPORT) && defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "VideoCapturePlugin"

#include "video_capture_plugin.h"
#include <algorithm>
#include <cmath>
#include "foundation/log.h"
#include "foundation/osal/utils/util.h"
#include "foundation/pre_defines.h"
#include "plugin/common/plugin_time.h"
#include "utils/constants.h"

namespace {
// register plugins
using namespace OHOS::Media::Plugin;
using namespace VideoCapture;
const size_t DATA_SIZE = 25344; // 25344=176*144 the size of yuv file

Status VideoFileCaptureRegister(const std::shared_ptr<Register> &reg)
{
    SourcePluginDef definition;
    definition.packageName = "StubVideoCapture";
    definition.name = "VideoFileCapture";
    definition.description = "Video capture from audio service";
    definition.rank = 100; // 100: max rank
    definition.inputType = SrcInputType::VID_SURFACE_YUV;
    definition.creator = [](const std::string& name) -> std::shared_ptr<SourcePlugin> {
        return std::make_shared<VideoFileCapturePlugin>(name);
    };
    Capability outCaps(OHOS::Media::MEDIA_MIME_VIDEO_RAW);
    outCaps.AppendDiscreteKeys<VideoPixelFormat>(Capability::Key::VIDEO_PIXEL_FORMAT, {VideoPixelFormat::NV21});
    definition.outCaps.push_back(outCaps);
    // add es outCaps later
    return reg->AddPlugin(definition);
}
PLUGIN_DEFINITION(StubVideoCapture, LicenseType::APACHE_V2, VideoFileCaptureRegister, [] {});
}

namespace OHOS {
namespace Media {
namespace Plugin {
namespace VideoCapture {
constexpr int32_t DEFAULT_SURFACE_QUEUE_SIZE = 6;
constexpr int32_t DEFAULT_SURFACE_SIZE = 1024 * 1024;
constexpr int32_t DEFAULT_VIDEO_WIDTH = 1920;
constexpr int32_t DEFAULT_VIDEO_HEIGHT = 1080;
constexpr int32_t DEFAULT_STRIDE_ALIGN = 16;

VideoFileCapturePlugin::VideoFileCapturePlugin(std::string name)
    : SourcePlugin(std::move(name)), width_(DEFAULT_VIDEO_WIDTH), height_(DEFAULT_VIDEO_HEIGHT)
{
    MEDIA_LOG_D("IN");
}

VideoFileCapturePlugin::~VideoFileCapturePlugin()
{
    MEDIA_LOG_D("IN");
}

Status VideoFileCapturePlugin::Init()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status VideoFileCapturePlugin::Deinit()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status VideoFileCapturePlugin::Prepare()
{
    MEDIA_LOG_D("IN");
    std::string filePath = RESOURCE_DIR "/VideoData.yuv";
    std::string fullPath;
    if (OSAL::ConvertFullPath(filePath, fullPath) && !fullPath.empty()) {
        filePath = fullPath;
    }
    fp_ = fopen(filePath.c_str(), "rb");
    FALSE_RETURN_V(fp_ != nullptr, Status::ERROR_NULL_POINTER);
    bufferSize_ = DATA_SIZE;
    return Status::OK;
}

Status VideoFileCapturePlugin::Reset()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status VideoFileCapturePlugin::Start()
{
    MEDIA_LOG_D("IN");
    OSAL::ScopedLock lock(mutex_);
    if (isStop_.load()) {
        if (curTimestampNs_ < stopTimestampNs_) {
            MEDIA_LOG_E("Get wrong audio time");
        }
        totalPauseTimeNs_ += std::fabs(curTimestampNs_ - stopTimestampNs_);
        isStop_ = false;
    }
    return Status::OK;
}

Status VideoFileCapturePlugin::Stop()
{
    MEDIA_LOG_D("IN");
    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
    }
    bufferSize_ = 0;
    OSAL::ScopedLock lock(mutex_);
    if (!isStop_.load()) {
        stopTimestampNs_ = curTimestampNs_;
        isStop_ = true;
    }
    return Status::OK;
}

Status VideoFileCapturePlugin::GetParameter(Tag tag, ValueType& value)
{
    MEDIA_LOG_D("IN");
    switch (tag) {
        case Tag::VIDEO_SURFACE: {
            value = nullptr;
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

Status VideoFileCapturePlugin::SetParameter(Tag tag, const ValueType& value)
{
    switch (tag) {
        case Tag::VIDEO_HEIGHT: {
            if (value.SameTypeWith(typeid(uint32_t))) {
                height_ = Plugin::AnyCast<uint32_t>(value);
            }
            break;
        }
        case Tag::VIDEO_WIDTH: {
            if (value.SameTypeWith(typeid(uint32_t))) {
                width_ = Plugin::AnyCast<uint32_t>(value);
            }
            break;
        }
        case Tag::VIDEO_CAPTURE_RATE: {
            if (value.SameTypeWith(typeid(double))) {
                captureRate_ = Plugin::AnyCast<double>(value);
            }
            break;
        }
        case Tag::VIDEO_PIXEL_FORMAT: {
            if (value.SameTypeWith(typeid(VideoPixelFormat))) {
                pixelFormat_ = Plugin::AnyCast<VideoPixelFormat>(value);
            }
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

std::shared_ptr<Allocator> VideoFileCapturePlugin::GetAllocator()
{
    MEDIA_LOG_D("IN");
    return nullptr;
}

Status VideoFileCapturePlugin::SetCallback(Callback* cb)
{
    MEDIA_LOG_D("IN");
    UNUSED_VARIABLE(cb);
    return Status::ERROR_UNIMPLEMENTED;
}

Status VideoFileCapturePlugin::SetSource(std::shared_ptr<MediaSource> source)
{
    UNUSED_VARIABLE(source);
    return Status::ERROR_UNIMPLEMENTED;
}

void VideoFileCapturePlugin::SetVideoBufferMeta(std::shared_ptr<BufferMeta>& bufferMeta)
{
    std::shared_ptr<VideoBufferMeta> videoMeta = std::dynamic_pointer_cast<VideoBufferMeta>(bufferMeta);
    videoMeta->width = width_;
    videoMeta->height = height_;
    videoMeta->videoPixelFormat = VideoPixelFormat::NV21;
    size_t lineSize = AlignUp(width_, DEFAULT_STRIDE_ALIGN);
    if ((lineSize / 2) % DEFAULT_STRIDE_ALIGN) { // 2
        lineSize = AlignUp(width_, DEFAULT_STRIDE_ALIGN * 2); // 2
    }
    videoMeta->stride.emplace_back(lineSize); // lineSize[0]
    videoMeta->stride.emplace_back(lineSize / 2); // lineSize[1], 2
    videoMeta->stride.emplace_back(lineSize / 2); // lineSize[2], 2
    videoMeta->planes = videoMeta->stride.size();
}

Status VideoFileCapturePlugin::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    OSAL::ScopedLock lock(mutex_);
    if (!buffer) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    auto bufferMeta = buffer->GetBufferMeta();
    if (!bufferMeta || bufferMeta->GetType() != BufferMetaType::VIDEO || fp_ == nullptr) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    std::shared_ptr<Memory> bufData;
    if (buffer->IsEmpty()) {
        bufData = buffer->AllocMemory(GetAllocator(), expectedLen);
    } else {
        bufData = buffer->GetMemory();
    }
    if (bufData->GetMemoryType() != MemoryType::VIRTUAL_ADDR || bufData->GetCapacity() <= 0) {
        return Status::ERROR_NO_MEMORY;
    }
    readCond_.Wait(lock, [this] { return bufferCnt_ > 0 || isStop_.load(); });
    if (isStop_.load()) {
        MEDIA_LOG_I("flush or EOS, skip read buffer");
        return Status::END_OF_STREAM;
    }
    uint8_t fileData[DATA_SIZE+1] = { 0 };
    (void)fread(fileData, DATA_SIZE, 1, fp_);
    if (bufData->Write(static_cast<const uint8_t*>(fileData), bufferSize_) != bufferSize_) {
        MEDIA_LOG_E("write buffer data fail");
        return Status::ERROR_UNKNOWN;
    }
    Ns2HstTime(curTimestampNs_ - totalPauseTimeNs_, reinterpret_cast<int64_t &>(buffer->pts));
    SetVideoBufferMeta(bufferMeta);
    bufferCnt_--;
    return Status::OK;
}

Status VideoFileCapturePlugin::GetSize(size_t& size)
{
    if (bufferSize_ == 0) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    size = bufferSize_;
    OSAL::ScopedLock lock(mutex_);
    bufferCnt_++;
    if (bufferCnt_ == 1) {
        readCond_.NotifyAll();
    }
    MEDIA_LOG_D("bufferSize_: " PUBLIC_LOG_ZU, size);
    return Status::OK;
}

bool VideoFileCapturePlugin::IsSeekable()
{
    return false;
}

Status VideoFileCapturePlugin::SeekTo(uint64_t offset)
{
    UNUSED_VARIABLE(offset);
    return Status::ERROR_UNIMPLEMENTED;
}
} // namespace VideoCapture
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif