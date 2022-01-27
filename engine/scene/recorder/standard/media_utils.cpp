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

#include "media_utils.h"
#include <regex>
#include <sys/stat.h>
#include "media_errors.h"
#include "plugin/common/plugin_audio_tags.h"
namespace OHOS {
namespace Media {
namespace {
const std::pair<ErrorCode, int32_t> g_errorCodePair[] = {
    {ErrorCode::SUCCESS, MSERR_OK},
    {ErrorCode::ERROR_UNKNOWN, MSERR_UNKNOWN},
    {ErrorCode::ERROR_AGAIN, MSERR_UNKNOWN},
    {ErrorCode::ERROR_UNIMPLEMENTED, MSERR_UNSUPPORT},
    {ErrorCode::ERROR_INVALID_PARAMETER_VALUE, MSERR_INVALID_VAL},
    {ErrorCode::ERROR_INVALID_PARAMETER_TYPE, MSERR_INVALID_VAL},
    {ErrorCode::ERROR_INVALID_OPERATION, MSERR_INVALID_OPERATION},
    {ErrorCode::ERROR_UNSUPPORTED_FORMAT, MSERR_UNSUPPORT_CONTAINER_TYPE},
    {ErrorCode::ERROR_NOT_EXISTED, MSERR_OPEN_FILE_FAILED},
    {ErrorCode::ERROR_TIMED_OUT, MSERR_EXT_TIMEOUT},
    {ErrorCode::ERROR_NO_MEMORY, MSERR_EXT_NO_MEMORY},
    {ErrorCode::ERROR_INVALID_STATE, MSERR_INVALID_STATE},
};
}  // namespace
namespace Record {
int TransErrorCode(ErrorCode errorCode)
{
    for (const auto& errPair : g_errorCodePair) {
        if (errPair.first == errorCode) {
            return errPair.second;
        }
    }
    return MSERR_UNKNOWN;
}
Plugin::SrcInputType TransAudioInputType(OHOS::Media::AudioSourceType sourceType)
{
    const static std::pair<OHOS::Media::AudioSourceType, Plugin::SrcInputType> mapArray[] = {
            {OHOS::Media::AudioSourceType::AUDIO_MIC, Plugin::SrcInputType::AUD_MIC},
            {OHOS::Media::AudioSourceType::AUDIO_SOURCE_DEFAULT, Plugin::SrcInputType::AUD_MIC},
    };
    for (const auto& item : mapArray) {
        if (item.first == sourceType) {
            return item.second;
        }
    }
    return Plugin::SrcInputType::UNKNOWN;
}
Plugin::SrcInputType TransVideoInputType(OHOS::Media::VideoSourceType sourceType)
{
    const static std::pair<OHOS::Media::VideoSourceType, Plugin::SrcInputType> mapArray[] = {
            {OHOS::Media::VideoSourceType::VIDEO_SOURCE_SURFACE_YUV, Plugin::SrcInputType::VID_SURFACE_YUV},
            {OHOS::Media::VideoSourceType::VIDEO_SOURCE_SURFACE_ES, Plugin::SrcInputType::VID_SURFACE_ES},
    };
    for (const auto& item : mapArray) {
        if (item.first == sourceType) {
            return item.second;
        }
    }
    return Plugin::SrcInputType::UNKNOWN;
}
bool TransAudioEncoderFmt(OHOS::Media::AudioCodecFormat aFormat, Plugin::Meta& encoderMeta)
{
    switch (aFormat) {
        case OHOS::Media::AudioCodecFormat::AUDIO_DEFAULT:
        case OHOS::Media::AudioCodecFormat::AAC_LC:
            encoderMeta.SetString(Plugin::MetaID::MIME, MEDIA_MIME_AUDIO_AAC);
            encoderMeta.SetData(Plugin::MetaID::AUDIO_AAC_PROFILE, Plugin::AudioAacProfile::LC);
            return true;
        default:
            break;
    }
    return false;
}

bool IsDirectory(const std::string& path)
{
    struct stat s {};
    return ((stat(path.c_str(), &s) == 0) && S_ISDIR(s.st_mode));
}
bool ConvertDirPathToFilePath(const std::string& dirPath, OutputFormatType outputFormatType, std::string& filePath)
{
    std::regex reg("\\\\");
    filePath = std::regex_replace(dirPath, reg, "/") ;
    if (filePath[filePath.size() - 1] != '/') {
        filePath += "/";
    }
    if (g_outputFormatToFormat.find(outputFormatType) != g_outputFormatToFormat.end()) {
        char fileName[32] = { 0 }; /// magic number 32
        auto tm = time(nullptr);
        strftime(fileName, sizeof(fileName), g_outputFormatToFormat.at(outputFormatType).c_str(), localtime(&tm));
        filePath += fileName;
        return true;
    } else {
        return false;
    }
}
}  // namespace Record
}  // namespace Media
}  // namespace OHOS