/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef HISTREAMER_PLUGIN_CONVERT_H
#define HISTREAMER_PLUGIN_CONVERT_H
#undef memcpy_s
#include <memory>
#include <vector>
#include "plugin/common/plugin_types.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
namespace Ffmpeg {
struct ResamplePara {
    uint32_t channels {2}; // 2: STEREO
    uint32_t sampleRate {0};
    uint32_t bitsPerSample {0};
    int64_t channelLayout {0};
    AVSampleFormat srcFfFmt {AV_SAMPLE_FMT_NONE};
    uint32_t destSamplesPerFrame {0};
    AVSampleFormat destFmt {AV_SAMPLE_FMT_S16};
};

class Resample {
public:
    Status Init(const ResamplePara& resamplePara);
    Status Convert(const uint8_t* srcBuffer, const size_t srcLength, uint8_t*& destBuffer, size_t& destLength);
private:
    ResamplePara resamplePara_ {};
#if defined(_WIN32) || !defined(OHOS_LITE)
    std::vector<uint8_t> resampleCache_ {};
    std::vector<uint8_t*> resampleChannelAddr_ {};
    std::shared_ptr<SwrContext> swrCtx_ {nullptr};
#endif
};

#if defined(VIDEO_SUPPORT)
struct ScalePara {
    int32_t srcWidth {0};
    int32_t srcHeight {0};
    AVPixelFormat srcFfFmt {AVPixelFormat::AV_PIX_FMT_NONE};
    int32_t dstWidth {0};
    int32_t dstHeight {0};
    AVPixelFormat dstFfFmt {AVPixelFormat::AV_PIX_FMT_RGBA};
    int32_t align {16};
};

struct Scale {
public:
    Status Init(const ScalePara& scalePara, uint8_t** dstData, int32_t* dstLineSize);
    Status Convert(uint8_t** srcData, const int32_t* srcLineSize, uint8_t** dstData, int32_t* dstLineSize);
private:
    ScalePara scalePara_ {};
    std::shared_ptr<SwsContext> swsCtx_ {nullptr};
};
#endif
} // namespace Ffmpeg
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CONVERT_H
