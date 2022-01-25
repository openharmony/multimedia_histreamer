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

#ifndef HISTREAMER_MEDIA_UTILS_H
#define HISTREAMER_MEDIA_UTILS_H

#include "i_recorder_engine.h"
#include "foundation/error_code.h"
#include "plugin/common/plugin_source_tags.h"
#include "plugin/core/plugin_meta.h"
#include "utils/constants.h"

namespace OHOS {
namespace Media {
namespace Record {
int TransErrorCode(ErrorCode errorCode);
Plugin::SrcInputType TransAudioInputType(OHOS::Media::AudioSourceType sourceType);
Plugin::SrcInputType TransVideoInputType(OHOS::Media::VideoSourceType sourceType);
bool TransAudioEncoderFmt(OHOS::Media::AudioCodecFormat aFormat, Plugin::Meta& encoderMeta);
bool IsDirectory(const std::string& path);
bool ConvertDirPathToFilePath(const std::string& dirPath, OutputFormatType outputFormatType, std::string& filePath);

struct RecordParam {
    int32_t sourceId;
    uint32_t type;
    Plugin::Any any;
};

const std::map<OutputFormatType, std::string> g_outputFormatToMimeMap = {
    {OutputFormatType::FORMAT_DEFAULT, MEDIA_MIME_AUDIO_MPEG},
    {OutputFormatType::FORMAT_MPEG_4, MEDIA_MIME_AUDIO_MPEG},
    {OutputFormatType::FORMAT_M4A, MEDIA_MIME_CONTAINER_MP4},
};

#define CreateFileName(prefix, suffix)  std::string{prefix}.append("_%Y%m%d%H%M%S").append(suffix)

const std::map<OutputFormatType, std::string> g_outputFormatToFormat = {
    {OutputFormatType::FORMAT_DEFAULT, CreateFileName("audio", ".m4a")},
    {OutputFormatType::FORMAT_MPEG_4, CreateFileName("video", ".mp4")},
    {OutputFormatType::FORMAT_M4A, CreateFileName("audio", ".m4a")},
};
}  // namespace Record
}  // namespace Media
}  // namespace OHOS

#endif  // HISTREAMER_MEDIA_UTILS_H
