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
#if defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "CodecPort"

#include "codec_port.h"
#include "codec_utils.h"
#include "foundation/log.h"
#include "hdf_base.h"
#include "plugin/common/plugin_buffer.h"

namespace {
    constexpr uint32_t HDI_FRAME_RATE_MOVE = 16; // hdi frame rate need move 16
    constexpr uint32_t HDI_VIDEO_ALIGNMENT = 16;
    constexpr uint32_t HDI_VIDEO_WIDTH_ALIGNMENT = 32;
}
namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
CodecPort::CodecPort(CodecComponentType* component, uint32_t portIndex, const CompVerInfo& verInfo)
    : codecComp_(component), verInfo_(verInfo)
{
    InitOmxParam(portDef_, verInfo_);
    portDef_.nPortIndex = portIndex;
    verInfo_ = verInfo;
}

Status CodecPort::Config(Meta& meta)
{
    MEDIA_LOG_D("Config Start");
    auto ret = HdiGetParameter(codecComp_, OMX_IndexParamPortDefinition, portDef_);
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_PARAMETER, "HdiGetParameter portDef failed");
    std::string mime;
    uint32_t height;
    uint32_t width;
    uint32_t frameRate;
    int64_t bitRate;
    Plugin::VideoPixelFormat vdecFormat;
    FALSE_LOG(meta.Get<Tag::MIME>(mime));
    FALSE_LOG(meta.Get<Tag::VIDEO_PIXEL_FORMAT>(vdecFormat));
    FALSE_LOG(meta.Get<Tag::VIDEO_HEIGHT>(height));
    FALSE_LOG(meta.Get<Tag::VIDEO_WIDTH>(width));
    FALSE_LOG(meta.Get<Tag::VIDEO_FRAME_RATE>(frameRate));
    FALSE_LOG(meta.Get<Tag::MEDIA_BITRATE>(bitRate));
    portDef_.format.video.eCompressionFormat = CodingTypeHstToHdi(mime);
    portDef_.format.video.eColorFormat = FormatHstToOmx(vdecFormat);
    portDef_.format.video.nFrameHeight = AlignUp(height, HDI_VIDEO_ALIGNMENT);
    portDef_.format.video.nSliceHeight = AlignUp(height, HDI_VIDEO_ALIGNMENT);
    portDef_.format.video.nFrameWidth = AlignUp(width, HDI_VIDEO_WIDTH_ALIGNMENT);
    portDef_.format.video.nStride = static_cast<int32_t>(AlignUp(width, HDI_VIDEO_ALIGNMENT));
    portDef_.format.video.xFramerate = frameRate << HDI_FRAME_RATE_MOVE;
    MEDIA_LOG_D("frame_rate: " PUBLIC_LOG_U32, portDef_.format.video.xFramerate);
    portDef_.format.video.nBitrate = static_cast<uint32_t>(bitRate);
    ret = HdiSetParameter(codecComp_, OMX_IndexParamPortDefinition, portDef_);
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_PARAMETER, "HdiSetParameter failed");
    return Status::OK;
}

Status CodecPort::QueryParam(PortInfo& portInfo)
{
    MEDIA_LOG_D("QueryParam Start");
    auto ret = HdiGetParameter(codecComp_, OMX_IndexParamPortDefinition, portDef_);
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_PARAMETER, "HdiGetParameter failed");
    portInfo.bufferCount = portDef_.nBufferCountActual;
    portInfo.bufferSize = portDef_.nBufferSize;
    portInfo.enabled = portDef_.bEnabled;
    return Status::OK;
}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif