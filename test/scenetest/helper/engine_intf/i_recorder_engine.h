/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef IRECORDER_ENGINE_H
#define IRECORDER_ENGINE_H

#include <cstdint>
#include <string>
#include <memory>
#include <refbase.h>
#include "nocopyable.h"
#include "recorder.h"
#include "recorder_param.h"

namespace OHOS {
class Surface;

namespace Media {
/**
 * Dummy source id, used to configure recorder parameter for source-independent parameter. For
 * example, when specify the maximum duration of the output file, a "sourceId" parameter is still
 * required by the "Configure" interface of RecorderEngine. Then, internally use the DUMMY_SOURCE_ID
 * to represent the parameter is source-independent.
 */
constexpr int32_t DUMMY_SOURCE_ID = 0;

/**
 * Recorder Engine Observer. This is a abstract class, engine's user need to implement it and register
 * its instance into engine. The  recorder engine will report itself's information or error asynchronously
 * to observer.
 */
class IRecorderEngineObs : public std::enable_shared_from_this<IRecorderEngineObs> {
public:
    enum InfoType : int32_t {
        MAX_DURATION_APPROACHING = 0,
        MAX_FILESIZE_APPROACHING,
        MAX_DURATION_REACHED,
        MAX_FILESIZE_REACHED,
        NEXT_OUTPUT_FILE_STARTED,
        FILE_SPLIT_FINISHED,  // reserved
        FILE_START_TIME_MS,   // reserved
        NEXT_FILE_FD_NOT_SET,
        INTERNEL_WARNING,
        INFO_EXTEND_START = 0x10000,
    };

    enum ErrorType : int32_t {
        ERROR_CREATE_FILE_FAIL = 0,
        ERROR_WRITE_FILE_FAIL,
        ERROR_INTERNAL,
        ERROR_EXTEND_START = 0X10000,
    };

    virtual ~IRecorderEngineObs() = default;
    virtual void OnError(ErrorType errorType, int32_t errorCode) = 0;
    virtual void OnInfo(InfoType type, int32_t extra) = 0;
};

/**
 * Recorder Engine Interface.
 */
class IRecorderEngine {
public:
    virtual ~IRecorderEngine() = default;

    /**
     * Sets the video source for recording. The sourceId can be used to identify the video source when configure
     * the video track's any properties. When the setting is failed, the sourceId is -1.
     * This interface must be called before SetOutputFormat.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t SetVideoSource(VideoSourceType source, int32_t &sourceId) = 0;

    /**
     * Sets the audio source for recording. The sourceId can be used to identify the audio source when configure
     * the audio track's any properties. When the setting is failed, the sourceId is -1.
     * This interface must be called before SetOutputFormat.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t SetAudioSource(AudioSourceType source, int32_t &sourceId) = 0;

    /**
     * Sets the output file format. The function must be called after SetVideoSource or SetAudioSource, and before
     * Prepare. After this interface called, the engine will not accept any source setting interface call.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t SetOutputFormat(OutputFormatType format) = 0;

    /**
     * Register a recording observer.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t SetObs(const std::weak_ptr<IRecorderEngineObs> &obs) = 0;

    /**
     * Configure static record parameters before calling the Prepare interface. The interface must be called after
     * SetOutputFormat. The sourceId indicates the source ID, which can be obtained from SetVideoSource
     * or SetAudioSource. Use the DUMMY_SOURCE_ID  to configure the source-independent parameters.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t Configure(int32_t sourceId, const RecorderParam &recParam) = 0;

    /**
     * Obtains the surface of the video source. The sourceId indicates the video source ID, which can be obtained
     * from SetVideoSource.
     */
    virtual sptr<Surface> GetSurface(int32_t sourceId) = 0;

    /**
     * Prepares for recording. This function must be called before Start. Ensure all required recorder parameter
     * have already been set, or this call will be failed.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t Prepare() = 0;

    /**
     * Starts recording. This function must be called after Prepare.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t Start() = 0;

    /**
     * Pause recording. This function must be called during recording.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t Pause() = 0;

    /**
     * Resume recording. This function must be called during recording. After called, the paused recording will
     * be resumed.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t Resume() = 0;

    /**
     * Stop recording. This function must be called during recording. The isDrainAll indicates whether all caches
     * need to be discarded. If true, wait all caches to be processed, or discard all caches.
     * Currently, this interface behaves like the Reset, so after it called, anything need to be reconfigured.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t Stop(bool isDrainAll = false) = 0;

    /**
     * Resets the recording. After this interface called, anything need to be reconfigured.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t Reset() = 0;

    /**
     * Sets an extended parameter for recording. It must be called after Prepare. The sourceId indicates the
     * source ID, which can be obtained from SetVideoSource or SetAudioSource. Use the DUMMY_SOURCE_ID  to
     * configure the source-independent parameters.
     * Return MSERR_OK indicates success, or others indicate failed.
     */
    virtual int32_t SetParameter(int32_t sourceId, const RecorderParam &recParam) = 0;
};
} // namespace Media
} // namespace OHOS
#endif
