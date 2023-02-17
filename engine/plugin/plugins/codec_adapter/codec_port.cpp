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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "CodecPort"
#include "codec_port.h"
#include "codec_utils.h"
#include "foundation/log.h"
#include "hdf_base.h"

namespace {
    constexpr uint32_t HDI_FRAME_RATE_MOVE = 16; // hdi frame rate need move 16
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

Status CodecPort::Config(TagMap& tagMap)
{
    auto ret = HdiGetParameter(codecComp_, OMX_IndexParamPortDefinition, portDef_);
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_PARAMETER, "HdiGetParameter failed");
    portDef_.format.video.eCompressionFormat = CodingTypeHstToHdi(Plugin::AnyCast<std::string>(tagMap[Tag::MIME]));
    portDef_.format.video.eColorFormat = FormatHstToOmx(Plugin::AnyCast<VideoPixelFormat>(
        tagMap[Tag::VIDEO_PIXEL_FORMAT]));
    portDef_.format.video.nFrameHeight = Plugin::AnyCast<uint32_t>(tagMap[Tag::VIDEO_HEIGHT]);
    portDef_.format.video.nFrameWidth = Plugin::AnyCast<uint32_t>(tagMap[Tag::VIDEO_WIDTH]);
    portDef_.format.video.xFramerate = Plugin::AnyCast<uint32_t>(tagMap[Tag::VIDEO_FRAME_RATE])
        << HDI_FRAME_RATE_MOVE;
    MEDIA_LOG_D("frame_rate" PUBLIC_LOG_U32, portDef_.format.video.xFramerate);
    ret = HdiSetParameter(codecComp_, OMX_IndexParamPortDefinition, portDef_);
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_INVALID_PARAMETER, "HdiSetParameter failed");
    return Status::OK;
}

Status CodecPort::QueryParam(PortInfo& portInfo)
{
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