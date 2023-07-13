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
#include "foundation/log.h"
#include "plugin/convert/ffmpeg_convert.h"
#include "securec.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace Ffmpeg {
Status Resample::Init(const ResamplePara& resamplePara)
{
    resamplePara_ = resamplePara;
#if defined(_WIN32) || !defined(OHOS_LITE)
    if (resamplePara_.bitsPerSample != 8 && resamplePara_.bitsPerSample != 24) { // 8 24
        auto destFrameSize = av_samples_get_buffer_size(nullptr, resamplePara_.channels,
                                                        resamplePara_.destSamplesPerFrame, resamplePara_.destFmt, 0);
        resampleCache_.reserve(destFrameSize);
        resampleChannelAddr_.reserve(resamplePara_.channels);
        auto tmp = resampleChannelAddr_.data();
        av_samples_fill_arrays(tmp, nullptr, resampleCache_.data(), resamplePara_.channels,
                               resamplePara_.destSamplesPerFrame, resamplePara_.destFmt, 0);
        auto swrContext = swr_alloc();
        if (swrContext == nullptr) {
            MEDIA_LOG_E("cannot allocate swr context");
            return Status::ERROR_NO_MEMORY;
        }
        swrContext = swr_alloc_set_opts(swrContext, resamplePara_.channelLayout, resamplePara_.destFmt,
                                        resamplePara_.sampleRate, resamplePara_.channelLayout,
                                        resamplePara_.srcFfFmt, resamplePara_.sampleRate, 0, nullptr);
        if (swr_init(swrContext) != 0) {
            MEDIA_LOG_E("swr init error");
            return Status::ERROR_UNKNOWN;
        }
        swrCtx_ = std::shared_ptr<SwrContext>(swrContext, [](SwrContext *ptr) {
            if (ptr) {
                swr_free(&ptr);
            }
        });
    }
#endif
    return Status::OK;
}

Status Resample::Convert(const uint8_t* srcBuffer, const size_t srcLength, uint8_t*& destBuffer, size_t& destLength)
{
#if defined(_WIN32) || !defined(OHOS_LITE)
    if (resamplePara_.bitsPerSample == 8) { // 8
        FALSE_RETURN_V_MSG(resamplePara_.destFmt == AV_SAMPLE_FMT_S16, Status::ERROR_UNIMPLEMENTED,
                           "resample 8bit to other format can not support");
        destLength = srcLength * 2;  // 2
        resampleCache_.reserve(destLength);
        resampleCache_.assign(destLength, 0);
        for (size_t i {0}; i < destLength / 2; i++) { // 2
            auto resCode = memcpy_s(&resampleCache_[0] + i * 2 + 1, sizeof(uint8_t), srcBuffer + i, 1); // 0 2 1
            FALSE_RETURN_V_MSG_E(resCode == EOK, Status::ERROR_INVALID_OPERATION, "Memcpy failed at 8 bits/sample.");
            *(&resampleCache_[0] + i * 2 + 1) += 0x80; // 2 0x80
        }
        destBuffer = resampleCache_.data();
    } else if (resamplePara_.bitsPerSample == 24) {  // 24
        FALSE_RETURN_V_MSG(resamplePara_.destFmt == AV_SAMPLE_FMT_S16, Status::ERROR_UNIMPLEMENTED,
                           "resample 24bit to other format can not support");
        destLength = srcLength / 3 * 2; // 3 2
        resampleCache_.reserve(destLength);
        resampleCache_.assign(destLength, 0);
        for (size_t i = 0; i < destLength / 2; i++) { // 2
            auto resCode
                = memcpy_s(&resampleCache_[0] + i * 2, sizeof(uint8_t) * 2, srcBuffer + i * 3 + 1, 2); // 2 3 1
            FALSE_RETURN_V_MSG_E(resCode == EOK, Status::ERROR_INVALID_OPERATION, "Memcpy failed at 24 bits/sample.");
        }
        destBuffer = resampleCache_.data();
    } else {
        size_t lineSize = srcLength / resamplePara_.channels;
        std::vector<const uint8_t*> tmpInput(resamplePara_.channels);
        tmpInput[0] = srcBuffer;
        if (av_sample_fmt_is_planar(resamplePara_.srcFfFmt)) {
            for (size_t i = 1; i < tmpInput.size(); ++i) {
                tmpInput[i] = tmpInput[i-1] + lineSize;
            }
        }
        auto samples = lineSize / static_cast<size_t>(av_get_bytes_per_sample(resamplePara_.srcFfFmt));
        auto res = swr_convert(swrCtx_.get(), resampleChannelAddr_.data(), resamplePara_.destSamplesPerFrame,
                               tmpInput.data(), samples);
        if (res < 0) {
            MEDIA_LOG_E("resample input failed");
            destLength = 0;
        } else {
            destBuffer = resampleCache_.data();
            size_t bytesPerSample = static_cast<size_t>(av_get_bytes_per_sample(resamplePara_.destFmt));
            destLength = static_cast<size_t>(res) * bytesPerSample * resamplePara_.channels;
        }
    }
#endif
    return Status::OK;
}

#if defined(VIDEO_SUPPORT)
Status Scale::Init(const ScalePara& scalePara, uint8_t** dstData, int32_t* dstLineSize)
{
    scalePara_ = scalePara;
    if (swsCtx_ != nullptr) {
        return Status::OK;
    }
    auto swsContext = sws_getContext(scalePara_.srcWidth, scalePara_.srcHeight, scalePara_.srcFfFmt,
                                     scalePara_.dstWidth, scalePara_.dstHeight, scalePara_.dstFfFmt,
                                     SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    FALSE_RETURN_V_MSG_E(swsContext != nullptr, Status::ERROR_UNKNOWN, "sws_getContext fail");
    swsCtx_ = std::shared_ptr<SwsContext>(swsContext, [](struct SwsContext *ptr) {
        if (ptr != nullptr) {
            sws_freeContext(ptr);
        }
    });
    auto ret = av_image_alloc(dstData, dstLineSize, scalePara_.dstWidth, scalePara_.dstHeight,
                              scalePara_.dstFfFmt, scalePara_.align);
    FALSE_RETURN_V_MSG_E(ret >= 0, Status::ERROR_UNKNOWN, "could not allocate destination image" PUBLIC_LOG_D32, ret);
    MEDIA_LOG_D("av_image_alloc call, ret: " PUBLIC_LOG_U32 "dstPixelFormat_: " PUBLIC_LOG_U32,
            ret, scalePara_.dstFfFmt);
    // av_image_alloc can make sure that dstLineSize last element is 0
    for (int32_t i = 0; dstLineSize[i] > 0; i++) {
        MEDIA_LOG_D("dstLineSize[" PUBLIC_LOG_D32 "]: " PUBLIC_LOG_D32, i, dstLineSize[i]);
        if (dstData[i] && !dstLineSize[i]) {
            MEDIA_LOG_E("scale frame is broken, i: " PUBLIC_LOG_D32, i);
            return Status::ERROR_UNKNOWN;
        }
    }
    return Status::OK;
}

Status Scale::Convert(uint8_t** srcData, const int32_t* srcLineSize, uint8_t** dstData, int32_t* dstLineSize)
{
    auto res = sws_scale(swsCtx_.get(), srcData, srcLineSize, 0, scalePara_.srcHeight,
                         dstData, dstLineSize);
    FALSE_RETURN_V_MSG_E(res >= 0, Status::ERROR_UNKNOWN, "sws_scale fail: " PUBLIC_LOG_D32, res);
    return Status::OK;
}
#endif
} // namespace Ffmpeg
} // namespace Plugin
} // namespace Media
} // namespace OHOS