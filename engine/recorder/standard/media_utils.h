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
#include "recorder.h"
#include "foundation/error_code.h"
#include "recorder/internal/state.h"
#include "utils/constants.h"

namespace OHOS {
namespace Media {
namespace Record {
int TransErrorCode(ErrorCode errorCode);
std::string TransOutputFormatType(OutputFormatType outputFormatType);

struct RecorderSource {
    int32_t sourceType;
    int32_t sourceId;
};

/**
* @brief Enumerates recorder states.
*
* @since 1.0
* @version 1.0
*/
enum  RecorderState : uint32_t {
    /** Error */
    RECORDER_STATE_ERROR = 0,
    /** Idle */
    RECORDER_IDLE = 1 << 0,
    /** Initialized */
    RECORDER_INITIALIZED = 1 << 1,
    /** Preparing */
    RECORDER_PREPARING = 1 << 2,
    /** Prepared */
    RECORDER_PREPARED = 1 << 3,
    /** Recorder started */
    RECORDER_STARTED = 1 << 4,
    /** Recorder paused */
    RECORDER_PAUSED = 1 << 5,
    /** Recorder stopped */
    RECORDER_STOPPED = 1 << 6,
};

enum class RecorderParameterType : uint32_t {
    AUD_SAMPLE_RATE,
    AUD_SAMPLE_FORMAT,
    AUD_CHANNEL,
    MEDIA_BITRATE,
    AUD_BIT_RATE,
    AUD_ENC_FMT,
    OUT_PATH,
    OUT_FD,
};

struct RecordParam {
    int32_t sourceId;
    RecorderParameterType type;
    Plugin::Any any;
};

const std::map<OutputFormatType, std::string> g_outputFormatToMimeMap = {
    {OutputFormatType::FORMAT_DEFAULT, MEDIA_MIME_AUDIO_MPEG},
    {OutputFormatType::FORMAT_MPEG_4, MEDIA_MIME_AUDIO_MPEG},
    {OutputFormatType::FORMAT_M4A,  MEDIA_MIME_CONTAINER_MP4},
};
}  // namespace Record
}  // namespace Media
}  // namespace OHOS

#endif  // HISTREAMER_MEDIA_UTILS_H
