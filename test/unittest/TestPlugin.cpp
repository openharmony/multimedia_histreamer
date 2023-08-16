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

#include "gtest/gtest.h"
#include "plugin/common/plugin_meta.h"
#include "pipeline/filters/common/plugin_utils.h"
#include "plugin/common/plugin_tags.h"
#include "plugin/common/plugin_types.h"
#include "plugin/plugins/codec_adapter/codec_utils.h"
#include "pipeline/filters/common/plugin_settings.h"
#include "pipeline/core/filter_type.h"
#include "plugin/convert/ffmpeg_convert.h"
#include "hdf_base.h"
#include "OMX_Core.h"

using namespace testing::ext;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Plugin::CodecAdapter;
using namespace OHOS::Media::Pipeline;

namespace OHOS {
namespace Media {
namespace Test {
HWTEST(TestMeta, find_unavailable_plugins, TestSize.Level1)
{
    Meta meta;
    std::string artist("abcd");
    meta.Set<Tag::MEDIA_TITLE>(artist);
    auto cap = MetaToCapability(meta);
    auto plugins = FindAvailablePlugins(*cap, Plugin::PluginType::AUDIO_DECODER);
    ASSERT_TRUE(plugins.size() == 0);
}

HWTEST(TestMeta, set_para_to_sink_plugins, TestSize.Level1)
{
    std::shared_ptr<Ffmpeg::ResamplePara> resamplePara = std::make_shared<Ffmpeg::ResamplePara>();
    resamplePara->channels = 2;
    resamplePara->sampleRate = 96000;
    resamplePara->bitsPerSample = 16;
    resamplePara->channelLayout = 3;
    resamplePara->srcFfFmt = AVSampleFormat::AV_SAMPLE_FMT_FLTP;
    resamplePara->destSamplesPerFrame = 2048;
    resamplePara->destFmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    std::shared_ptr<Ffmpeg::Resample> resample = std::make_shared<Ffmpeg::Resample>();
    auto src = new uint8_t;
    size_t srcLength = 16384;
    auto des = new uint8_t;
    size_t desLength = 16384;
    ASSERT_TRUE(resample->Init(*resamplePara) == Status::OK);
    ASSERT_TRUE(resample->Convert(src, srcLength, des, desLength) == Status::OK);
    resamplePara->destFmt = AVSampleFormat::AV_SAMPLE_FMT_U8P;
    ASSERT_TRUE(resample->Init(*resamplePara) == Status::OK);
    ASSERT_TRUE(resample->Convert(src, srcLength, des, desLength) == Status::OK);

    resamplePara->bitsPerSample = 8;
    ASSERT_TRUE(resample->Init(*resamplePara) == Status::OK);
    ASSERT_FALSE(resample->Convert(src, srcLength, des, desLength) == Status::OK);

    resamplePara->bitsPerSample = 24;
    ASSERT_TRUE(resample->Init(*resamplePara) == Status::OK);
    ASSERT_FALSE(resample->Convert(src, srcLength, des, desLength) == Status::OK);
}

HWTEST(TestMeta, hdf_status_to_string, TestSize.Level1)
{
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_SUCCESS) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_FAILURE) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_NOT_SUPPORT) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_INVALID_PARAM) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_INVALID_OBJECT) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_MALLOC_FAIL) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_TIMEOUT) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_THREAD_CREATE_FAIL) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_QUEUE_FULL) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_DEVICE_BUSY) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_IO) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_BAD_FD) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_NOPERM) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_BSP_ERR_OP) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_ERR_BSP_PLT_API_ERR) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_PAL_ERR_DEV_CREATE) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_PAL_ERR_INNER) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_NO_MEMORY) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_NO_DEVICE) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_NO_DEVICE_SERVICE) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_DEV_INIT_FAIL) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_PUBLISH_FAIL) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_ATTACHDEV_FAIL) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_NODATA) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_NORANGE) != "null");
    ASSERT_TRUE(HdfStatus2String(HDF_STATUS::HDF_DEV_ERR_OP) != "null");
    ASSERT_TRUE(HdfStatus2String(100) == "null");
}

HWTEST(TestMeta, omx_error_type_to_string, TestSize.Level1)
{
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorNone) == "OMX_ErrorNone");
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorInsufficientResources) == "OMX_ErrorInsufficientResources");
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorUndefined) == "OMX_ErrorUndefined");
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorInvalidComponentName) == "OMX_ErrorInvalidComponentName");
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorComponentNotFound) == "OMX_ErrorComponentNotFound");
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorInvalidComponent) == "OMX_ErrorInvalidComponent");
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorBadParameter) == "OMX_ErrorBadParameter");
    ASSERT_TRUE(OmxErrorType2String(OMX_ErrorNotImplemented) == "OMX_ErrorNotImplemented");
    ASSERT_TRUE(OmxErrorType2String(100) == "OMX_ErrorNone");
}

HWTEST(TestMeta, transfor_hdi_ret_value_to_status, TestSize.Level1)
{
    ASSERT_TRUE(TransHdiRetVal2Status(HDF_SUCCESS) == Status::OK);
    ASSERT_TRUE(TransHdiRetVal2Status(HDF_FAILURE) == Status::ERROR_UNKNOWN);
    ASSERT_TRUE(TransHdiRetVal2Status(HDF_ERR_NOT_SUPPORT) == Status::ERROR_INVALID_OPERATION);
    ASSERT_TRUE(TransHdiRetVal2Status(HDF_ERR_INVALID_PARAM) == Status::ERROR_INVALID_PARAMETER);
    ASSERT_TRUE(TransHdiRetVal2Status(HDF_ERR_MALLOC_FAIL) == Status::ERROR_NO_MEMORY);
    ASSERT_TRUE(TransHdiRetVal2Status(100) == Status::ERROR_UNKNOWN);
}

HWTEST(TestMeta, translate_to_omx_flag_set, TestSize.Level1)
{
    ASSERT_TRUE(Translate2omxFlagSet(BUFFER_FLAG_EOS) == OMX_BUFFERFLAG_EOS);
    ASSERT_TRUE(Translate2omxFlagSet(100) == 0);
    ASSERT_TRUE(Translate2PluginFlagSet(OMX_BUFFERFLAG_EOS) == BUFFER_FLAG_EOS);
    ASSERT_TRUE(Translate2PluginFlagSet(100) == 0);
}

HWTEST(TestMeta, coding_type_hst_to_hdi, TestSize.Level1)
{
    ASSERT_TRUE(CodingTypeHstToHdi(MEDIA_MIME_VIDEO_H264) == OMX_VIDEO_CodingAVC);
    ASSERT_TRUE(CodingTypeHstToHdi(MEDIA_MIME_VIDEO_H265)
                == static_cast<OMX_VIDEO_CODINGTYPE>(CODEC_OMX_VIDEO_CodingHEVC));
    ASSERT_TRUE(CodingTypeHstToHdi("NULL") == OMX_VIDEO_CodingUnused);
}

HWTEST(TestMeta, format_hst_to_omx, TestSize.Level1)
{
    ASSERT_TRUE(FormatHstToOmx(VideoPixelFormat::NV12) == OMX_COLOR_FormatYUV420SemiPlanar);
    ASSERT_TRUE(FormatHstToOmx(VideoPixelFormat::NV21) == OMX_COLOR_FormatYUV420SemiPlanar);
    ASSERT_TRUE(FormatHstToOmx(VideoPixelFormat::BGRA) == OMX_COLOR_Format32bitBGRA8888);
    ASSERT_TRUE(FormatHstToOmx(VideoPixelFormat::RGBA) == OMX_COLOR_Format32bitARGB8888);
    ASSERT_TRUE(FormatHstToOmx(VideoPixelFormat::YUV420P) == OMX_COLOR_FormatYUV420Planar);
    ASSERT_TRUE(FormatHstToOmx(VideoPixelFormat::RGB24) == OMX_COLOR_FormatUnused);
}

HWTEST(TestMeta, omx_state_to_string, TestSize.Level1)
{
    ASSERT_TRUE(OmxStateToString(OMX_StateInvalid) == "OMX_StateInvalid");
    ASSERT_TRUE(OmxStateToString(OMX_StateLoaded) == "OMX_StateLoaded");
    ASSERT_TRUE(OmxStateToString(OMX_StateLoaded) == "OMX_StateLoaded");
    ASSERT_TRUE(OmxStateToString(OMX_StateIdle) == "OMX_StateIdle");
    ASSERT_TRUE(OmxStateToString(OMX_StateExecuting) == "OMX_StateExecuting");
    ASSERT_TRUE(OmxStateToString(OMX_StatePause) == "OMX_StatePause");
    ASSERT_TRUE(OmxStateToString(OMX_StateWaitForResources) == "OMX_StateWaitForResources");
    ASSERT_TRUE(OmxStateToString(OMX_StateKhronosExtensions) == "OMX_StateKhronosExtensions");
    ASSERT_TRUE(OmxStateToString(OMX_StateVendorStartUnused) == "OMX_StateVendorStartUnused");
    ASSERT_TRUE(OmxStateToString(OMX_StateMax) == "OMX_StateMax");
}

HWTEST(TestMeta, get_omx_buffer_type, TestSize.Level1)
{
    ASSERT_TRUE(GetOmxBufferType(static_cast<const Plugin::MemoryType>(MemoryType::SHARE_MEMORY), true)
                == CODEC_BUFFER_TYPE_AVSHARE_MEM_FD);
    ASSERT_TRUE(GetOmxBufferType(static_cast<const Plugin::MemoryType>(MemoryType::SURFACE_BUFFER), true)
                == CODEC_BUFFER_TYPE_DYNAMIC_HANDLE);
    ASSERT_TRUE(GetOmxBufferType(static_cast<const Plugin::MemoryType>(MemoryType::SURFACE_BUFFER), false)
                == CODEC_BUFFER_TYPE_HANDLE);
    ASSERT_TRUE(GetOmxBufferType(static_cast<const Plugin::MemoryType>(MemoryType::VIRTUAL_ADDR), true)
                == CODEC_BUFFER_TYPE_INVALID);
}

HWTEST(TestMeta, codec_buffer_pool, TestSize.Level1)
{
    PluginParaAllowedMap map = PluginParameterTable::FindAllowedParameterMap(FilterType::MEDIA_SOURCE);
    ASSERT_TRUE(map.size() == 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::CAPTURE_SOURCE);
    ASSERT_TRUE(map.size() == 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::DEMUXER);
    ASSERT_TRUE(map.size() == 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::MUXER);
    ASSERT_TRUE(map.size() > 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::AUDIO_DECODER);
    ASSERT_TRUE(map.size() > 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::VIDEO_DECODER);
    ASSERT_TRUE(map.size() > 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::AUDIO_ENCODER);
    ASSERT_TRUE(map.size() > 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::VIDEO_ENCODER);
    ASSERT_TRUE(map.size() > 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::AUDIO_SINK);
    ASSERT_TRUE(map.size() > 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::VIDEO_SINK);
    ASSERT_TRUE(map.size() > 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::OUTPUT_SINK);
    ASSERT_TRUE(map.size() == 0);
    map = PluginParameterTable::FindAllowedParameterMap(FilterType::NONE);
    ASSERT_TRUE(map.size() == 0);
}

HWTEST(TestMeta, assign_parameter_if_match, TestSize.Level1)
{
    uint32_t value = 0;
    uint32_t ret;
    ASSERT_FALSE(AssignParameterIfMatch(Tag::SECTION_REGULAR_START, ret, value));
    ASSERT_FALSE(AssignParameterIfMatch(Tag::MIME, value, 0));
    ASSERT_TRUE(AssignParameterIfMatch(Tag::TRACK_ID, ret, value));
}

HWTEST(TestMeta, translate_plugin_status, TestSize.Level1)
{
    auto status = TranslatePluginStatus(Plugin::Status::END_OF_STREAM);
    ASSERT_TRUE(status == ErrorCode::END_OF_STREAM);
    status = TranslatePluginStatus(Plugin::Status::OK);
    ASSERT_TRUE(status == ErrorCode::SUCCESS);
    status = TranslatePluginStatus(Plugin::Status::NO_ERROR);
    ASSERT_TRUE(status == ErrorCode::SUCCESS);
    status = TranslatePluginStatus(Plugin::Status::ERROR_UNKNOWN);
    ASSERT_TRUE(status == ErrorCode::ERROR_UNKNOWN);
    status = TranslatePluginStatus(Plugin::Status::ERROR_CLIENT);
    ASSERT_TRUE(status == ErrorCode::ERROR_UNKNOWN);
}

HWTEST(TestMeta, translate_into_parameter, TestSize.Level1)
{
    Tag tag = Tag::SECTION_REGULAR_START;
    auto ret = TranslateIntoParameter(-1, tag);
    ASSERT_FALSE(ret);
    ASSERT_TRUE(tag == Tag::SECTION_REGULAR_START);
    ret = TranslateIntoParameter(static_cast<const int>(Tag::SECTION_REGULAR_START), tag);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(tag == Tag::SECTION_REGULAR_START);
}

HWTEST(TestMeta, capability_to_string, TestSize.Level1)
{
    std::shared_ptr<Capability> capability = std::make_shared<Capability>();
    std::string string = Capability2String(*capability);
    ASSERT_TRUE(string == "Capability{mime:}");
    capability->SetMime("video/avc");
    capability->AppendFixedKey(Capability::Key::MEDIA_BITRATE, nullptr);
    capability->AppendFixedKey(Capability::Key::VIDEO_BIT_STREAM_FORMAT, nullptr);
    string = Capability2String(*capability);
    ASSERT_TRUE(string == "Capability{mime:video/avc, ");
}

HWTEST(TestMeta, find_allowed_parameter_map, TestSize.Level1)
{
    FilterType filterType = FilterType::NONE;
    PluginParaAllowedMap map = PluginParameterTable::FindAllowedParameterMap(filterType);
    ASSERT_TRUE(map.size() == 0);
    filterType = FilterType::AUDIO_DECODER;
    map = PluginParameterTable::FindAllowedParameterMap(filterType);
    ASSERT_TRUE(map.size() != 0);
}
} // namespace Test
} // namespace Media
} // namespace OHOS
