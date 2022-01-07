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

#ifndef HISTREAMER_HIRECORDER_IMPL_H
#define HISTREAMER_HIRECORDER_IMPL_H
#ifdef RECORDER_SUPPORT
#include <memory>
#include <unordered_map>

#include "common/any.h"
#include "engine_intf/i_recorder_engine.h"
#include "foundation/error_code.h"
#include "media_utils.h"
#include "recorder/internal/state_machine.h"
#include "osal/thread/condition_variable.h"
#include "osal/thread/mutex.h"
#include "pipeline/core/pipeline.h"
#include "pipeline/core/pipeline_core.h"
#include "pipeline/filters/source/audio_capture/audio_capture_filter.h"
#include "pipeline/filters/codec/audio_encoder/audio_encoder_filter.h"
#ifdef VIDEO_SUPPORT
#include "pipeline/filters/source/video_capture/video_capture_filter.h"
#include "pipeline/filters/codec/video_encoder/video_encoder_filter.h"
#endif
#include "pipeline/filters/muxer/muxer_filter.h"
#include "pipeline/filters/sink/output_sink/output_sink_filter.h"
#include "recorder_executor.h"

namespace OHOS {
namespace Media {
namespace Record {
class HiRecorderImpl : public Pipeline::EventReceiver,
                       public RecorderExecutor,
                       public StateChangeCallback,
                       public IRecorderEngine {
    friend class StateMachine;
public:
    HiRecorderImpl();
    ~HiRecorderImpl() override;
    HiRecorderImpl(const HiRecorderImpl& other) = delete;
    HiRecorderImpl& operator=(const HiRecorderImpl& other) = delete;
    ErrorCode Init();

    // interface from IRecorderEngine
    int32_t SetAudioSource(AudioSourceType source, int32_t& sourceId) override;
#ifdef VIDEO_SUPPORT
    int32_t SetVideoSource(VideoSourceType source, int32_t &sourceId) override;
#endif
    int32_t SetOutputFormat(OutputFormatType format) override;
    int32_t SetObs(const std::weak_ptr<IRecorderEngineObs>& obs) override;
    int32_t Configure(int32_t sourceId, const RecorderParam& recParam) override;
    int32_t Prepare() override;
    int32_t Start() override;
    int32_t Pause() override;
    int32_t Resume() override;
    int32_t Stop(bool isDrainAll = false) override;
    int32_t Reset() override;
    int32_t SetParameter(int32_t sourceId, const RecorderParam& recParam) override;
    // internal interfaces from Pipeline::EventReceiver
    void OnEvent(Event event) override;
    // internal interfaces from StateChangeCallback
    void OnStateChanged(StateId state) override;

    // interface from RecorderExecutor
    ErrorCode DoSetVideoSource(VideoSourceType sourceType, int32_t sourceId) const override;
    ErrorCode DoSetAudioSource(AudioSourceType sourceType, int32_t sourceId) const override;
    ErrorCode DoSetParameter(const Plugin::Any& param) const override;
    ErrorCode DoPrepare() override;
    ErrorCode DoStart() override;
    ErrorCode DoPause() override;
    ErrorCode DoResume() override;
    ErrorCode DoStop() override;
    ErrorCode DoOnComplete() override;
    ErrorCode DoOnError(RecorderErrorType infoType, ErrorCode errorCode) override;
private:
    OSAL::Mutex stateMutex_{};
    OSAL::ConditionVariable cond_{};
    StateMachine fsm_;
    std::atomic<StateId> curFsmState_;
    std::shared_ptr<Pipeline::PipelineCore> pipeline_;
    std::atomic<RecorderState> pipelineStates_;
    std::atomic<bool> initialized_{false};
    const std::map<StateId, RecorderState> stateIdToRecorderStateMap_ = {
        { StateId::INIT, RECORDER_INITIALIZED},
        { StateId::RECORDING_SETTING, RECORDER_PREPARING},
        { StateId::READY, RECORDER_PREPARED},
        { StateId::PAUSE, RECORDER_PAUSED},
        { StateId::RECORDING, RECORDER_STARTED}};
    std::weak_ptr<IRecorderEngineObs> obs_{};
    int32_t sourceId_ {};
    std::shared_ptr<Pipeline::MuxerFilter> muxer_;
    std::shared_ptr<Pipeline::OutputSinkFilter> outputSink_;
    std::shared_ptr<Pipeline::AudioCaptureFilter> audioCapture_;
    std::shared_ptr<Pipeline::AudioEncoderFilter> audioEncoder_;
#ifdef VIDEO_SUPPORT
    std::shared_ptr<Pipeline::VideoCaptureFilter> videoCapture_;
    std::shared_ptr<Pipeline::VideoEncoderFilter> videoEncoder_;
#endif
};
}  // namespace Record
}  // namespace Media
}  // namespace OHOS
#endif
#endif //HISTREAMER_HIRECORDER_IMPL_H
