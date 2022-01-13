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
#ifdef RECORDER_SUPPORT
#define HST_LOG_TAG "HiRecorderImpl"

#include "hirecorder_impl.h"
#include "foundation/log.h"
#include "pipeline/factory/filter_factory.h"
#include "recorder/standard/media_utils.h"
#include "utils/steady_clock.h"

namespace OHOS {
namespace Media {
namespace Record {
using namespace Pipeline;

HiRecorderImpl::HiRecorderImpl() : fsm_(*this), curFsmState_(StateId::INIT),
    pipelineStates_(RecorderState::RECORDER_IDLE)
{
    MEDIA_LOG_I("hiRecorderImpl ctor");

    FilterFactory::Instance().Init();
    muxer_ = FilterFactory::Instance().CreateFilterWithType<MuxerFilter>(
        "builtin.recorder.muxer","muxer");
    outputSink_ = FilterFactory::Instance().CreateFilterWithType<OutputSinkFilter>(
            "builtin.recorder.output_sink", "output_sink");
    FALSE_RETURN(muxer_ != nullptr);
    FALSE_RETURN(outputSink_ != nullptr);
    pipeline_ = std::make_shared<PipelineCore>();
}

HiRecorderImpl::~HiRecorderImpl()
{
    pipeline_.reset();
    audioCapture_.reset();
    audioEncoder_.reset();
#ifdef VIDEO_SUPPORT
    videoCapture_.reset();
    videoEncoder_.reset();
#endif
    muxer_.reset();
    outputSink_.reset();
    fsm_.Stop();
    MEDIA_LOG_D("dtor called.");
}

ErrorCode HiRecorderImpl::Init()
{
    if (initialized_.load()) {
        return ErrorCode::SUCCESS;
    }
    pipeline_->Init(this, nullptr);
    ErrorCode ret = pipeline_->AddFilters( {muxer_.get(), outputSink_.get()} );
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkFilters( {muxer_.get(), outputSink_.get()} );
    }
    FALSE_LOG(ret == ErrorCode::SUCCESS);
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = RecorderState::RECORDER_INITIALIZED;
        fsm_.SetStateCallback(this);
        fsm_.Start();
        initialized_ = true;
    } else {
        pipeline_->UnlinkPrevFilters();
        pipeline_->RemoveFilterChain(muxer_.get());
        pipelineStates_ = RecorderState::RECORDER_STATE_ERROR;
    }
    return ret;
}

int32_t HiRecorderImpl::SetAudioSource(AudioSourceType source, int32_t& sourceId)
{
    PROFILE_BEGIN("SetAudioSource begin");
    auto ret  = ErrorCode::SUCCESS;
    audioCapture_ = FilterFactory::Instance().CreateFilterWithType<AudioCaptureFilter>(
            "builtin.recorder.audiocapture", "audiocapture");
    audioEncoder_ = FilterFactory::Instance().CreateFilterWithType<AudioEncoderFilter>(
            "builtin.recorder.audioencoder", "audioencoder");
    ret = pipeline_->AddFilters( {audioCapture_.get(), audioEncoder_.get()});
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkFilters( {audioCapture_.get(), audioEncoder_.get()});
    }
    std::shared_ptr<InPort> muxerInPort {nullptr};
    if (ret == ErrorCode::SUCCESS) {
        ret = muxer_->AddTrack(muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkPorts(audioEncoder_->GetOutPort(PORT_NAME_DEFAULT), muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        ret = fsm_.SendEvent(Intent::SET_AUDIO_SOURCE, RecorderSource {source, sourceId});
    }
    if (ret != ErrorCode::SUCCESS) {
        sourceId =-1;
    }
    sourceId_++;
    sourceId = sourceId_;
    PROFILE_END("SetAudioSource end.");
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::SetVideoSource(VideoSourceType source, int32_t& sourceId)
{
    PROFILE_BEGIN("SetVideoSource begin");
    ErrorCode ret{ErrorCode::SUCCESS};
    videoCapture_ = FilterFactory::Instance().CreateFilterWithType<VideoCaptureFilter>(
        "builtin.recorder.videocapture", "videocapture");
    videoEncoder_ = FilterFactory::Instance().CreateFilterWithType<VideoEncoderFilter>(
        "builtin.recorder.videoencoder", "videoencoder");
    ret = pipeline_->AddFilters({videoCapture_.get(), videoEncoder_.get()});
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkFilters( {videoCapture_.get(), videoEncoder_.get()});
    }
    std::shared_ptr<InPort> muxerInPort{ nullptr };
    if (ret == ErrorCode::SUCCESS) {
        ret =muxer_->AddTrack(muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkPorts(videoEncoder_->GetOutPort(PORT_NAME_DEFAULT), muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        ret = fsm_.SendEvent(Intent::SET_VIDEO_SOURCE, RecorderSource {source, sourceId});
    }
    if (ret != ErrorCode::SUCCESS) {
        sourceId =-1;
    }
    sourceId_++;
    sourceId = sourceId_;
    sourceIdToSurfaceMap_.emplace(sourceId_, Surface::CreateSurfaceAsConsumer());
    PROFILE_END("SetVideoSource end.");
    return TransErrorCode(ret);
}

sptr<Surface> HiRecorderImpl::GetSurface(int32_t sourceId)
{
    if(sourceIdToSurfaceMap_.find(sourceId) != sourceIdToSurfaceMap_.end()) {
        return sourceIdToSurfaceMap_[sourceId];
    }
    return nullptr;
}

int32_t HiRecorderImpl::SetOutputFormat(OutputFormatType format)
{
    if (muxer_ == nullptr) {
        MEDIA_LOG_W("cannot set Param while muxer filter is null");
        return static_cast<int>(ErrorCode::ERROR_AGAIN);
    }
    ErrorCode ret = muxer_->SetOutputFormat(g_outputFormatToMimeMap.at(format));
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetOutputFormat failed with error %d", static_cast<int>(ret));
    }
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::SetObs(const std::weak_ptr<IRecorderEngineObs>& obs)
{
    obs_ = obs;
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiRecorderImpl::Configure(int32_t sourceId,  const RecorderParam &recParam)
{
    Plugin::Any any;
    switch(recParam.type) {
        case RecorderPublicParamType::AUD_SAMPLERATE:
            any = RecordParam {sourceId, RecorderParameterType::AUD_SAMPLE_RATE,
                               dynamic_cast<const AudSampleRate&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_CHANNEL:
            any = RecordParam {sourceId, RecorderParameterType::AUD_CHANNEL,
                               dynamic_cast<const AudChannel&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_BITRATE:
            any = RecordParam {sourceId,RecorderParameterType::AUD_BIT_RATE,
                               dynamic_cast<const AudBitRate&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_ENC_FMT:
            any = RecordParam {sourceId, RecorderParameterType::AUD_ENC_FMT,
                               dynamic_cast<const AudEnc&>(recParam)};
            break;
        case RecorderPublicParamType::OUT_PATH:
            any = RecordParam {sourceId, RecorderParameterType::OUT_PATH,
                               dynamic_cast<const OutFilePath&>(recParam)};
            break;
        case RecorderPublicParamType::OUT_FD:
            any = RecordParam {sourceId, RecorderParameterType::OUT_FD,
                               dynamic_cast<const OutFd&>(recParam)};
            break;
        default:
            MEDIA_LOG_E("ignore RecorderPublicParamType %d", recParam.type);
            return TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE);
    }
    auto ret = fsm_.SendEvent(Intent::SET_PARAMETER, any);
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Configure failed with error %d", static_cast<int>(ret));
    }
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::Prepare()
{
    MEDIA_LOG_D("Prepare entered, current fsm state: %s.", fsm_.GetCurrentState().c_str());
    PROFILE_BEGIN();
    auto ret = fsm_.SendEvent(Intent::PREPARE);
    if (ret != ErrorCode::SUCCESS) {
        PROFILE_END("Prepare failed,");
        MEDIA_LOG_E("prepare failed with error %d", ret);
    } else {
        PROFILE_END("Prepare successfully,");
    }
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::Start()
{
    PROFILE_BEGIN();
    ErrorCode ret;
    if (pipelineStates_ == RecorderState::RECORDER_PAUSED) {
        ret = fsm_.SendEvent(Intent::RESUME);
    } else {
        ret = fsm_.SendEvent(Intent::START);
    }
    PROFILE_END("Start ret = %d", TransErrorCode(ret));
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::Pause()
{
    PROFILE_BEGIN();
    auto ret = TransErrorCode(fsm_.SendEvent(Intent::PAUSE));
    PROFILE_END("Pause ret = %d", ret);
    return ret;
}

int32_t HiRecorderImpl::Resume()
{
    PROFILE_BEGIN();
    auto ret = TransErrorCode(fsm_.SendEvent(Intent::RESUME));
    PROFILE_END("Resume ret = %d", ret);
    return ret;
}

int32_t HiRecorderImpl::Stop(bool isDrainAll)
{
    PROFILE_BEGIN();
    auto ret = TransErrorCode(fsm_.SendEvent(Intent::STOP));
    PROFILE_END("Stop ret = %d", ret);
    return ret;
}

int32_t HiRecorderImpl::Reset()
{
    pipelineStates_ = RecorderState::RECORDER_IDLE;
    return Stop(false);
}

int32_t HiRecorderImpl::SetParameter(int32_t sourceId, const RecorderParam &recParam)
{
    return TransErrorCode(ErrorCode::SUCCESS);
}

void HiRecorderImpl::OnEvent(Event event)
{
    MEDIA_LOG_D("[HiStreamer] OnEvent (%d)", event.type);
    switch (event.type) {
        case EVENT_ERROR:
            fsm_.SendEventAsync(Intent::NOTIFY_ERROR, event.param);
            break;
        case EVENT_READY:
            fsm_.SendEventAsync(Intent::NOTIFY_READY);
            break;
        default:
            MEDIA_LOG_E("Unknown event(%d)", event.type);
    }
}

void HiRecorderImpl::OnStateChanged(StateId state)
{
    MEDIA_LOG_I("OnStateChanged from %d to %d", curFsmState_.load(), state);
    {
        OSAL::ScopedLock lock(stateMutex_);
        curFsmState_ = state;
        cond_.NotifyOne();
    }
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        ptr->OnInfo(IRecorderEngineObs::InfoType::INTERNEL_WARNING,
                    to_underlying(stateIdToRecorderStateMap_.at(state)));
    }
}

ErrorCode HiRecorderImpl::DoSetVideoSource(const Plugin::Any& param) const
{
    return videoCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_SOURCE_TYPE),
                                       Plugin::AnyCast<RecorderSource>(param).sourceType);
}

ErrorCode HiRecorderImpl::DoSetAudioSource(const Plugin::Any& param) const
{
    return audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SOURCE_TYPE),
                                       Plugin::AnyCast<RecorderSource>(param).sourceType);
}

ErrorCode HiRecorderImpl::DoSetParameter(const Plugin::Any& param) const
{
    ErrorCode ret  = ErrorCode::SUCCESS;
    int32_t sourceId;
    RecorderParameterType recorderParameterType;
    Plugin::Any any;
    if (param.Type() == typeid(RecordParam)) {
        sourceId =  Plugin::AnyCast<RecordParam>(param).sourceId;
        recorderParameterType = Plugin::AnyCast<RecordParam>(param).type;
        any = Plugin::AnyCast<RecordParam>(param).any;
    } else {
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    switch (recorderParameterType) {
        case RecorderParameterType::AUD_SAMPLE_RATE:
            ret = audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_RATE),
                                              static_cast<uint32_t>(Plugin::AnyCast<AudSampleRate>(any).sampleRate));
            break;
        case RecorderParameterType::AUD_CHANNEL:
            ret = audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNELS),
                                              static_cast<uint32_t>(Plugin::AnyCast<AudChannel>(any).channel));
            break;
        case RecorderParameterType::AUD_BIT_RATE:
            ret = audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_BITRATE),
                                              static_cast<int64_t>(Plugin::AnyCast<AudBitRate>(any).bitRate));
            break;
        case RecorderParameterType::AUD_ENC_FMT:
            ret = audioEncoder_->SetAudioEncoder(sourceId,
                                                 static_cast<Plugin::AudioFormat>(Plugin::AnyCast<AudEnc>(any).encFmt));
            break;
        case RecorderParameterType::OUT_PATH:
            ret = outputSink_->SetOutputPath(Plugin::AnyCast<OutFilePath>(any).path);
            break;
        case RecorderParameterType::OUT_FD:
            ret = outputSink_->SetFd(Plugin::AnyCast<OutFd>(any).fd);
            break;
        default:
            break;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoPrepare()
{
    auto ret = pipeline_->Prepare();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = RecorderState::RECORDER_PREPARED;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoStart()
{
    auto ret = pipeline_->Start();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = RecorderState::RECORDER_STARTED;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoPause()
{
    auto ret = pipeline_->Pause();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = RecorderState::RECORDER_PAUSED;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoResume()
{
    auto ret = pipeline_->Resume();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = RecorderState::RECORDER_STARTED;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoStop()
{
    auto ret = pipeline_->Stop();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = RecorderState::RECORDER_STOPPED;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoOnComplete()
{
    return ErrorCode::SUCCESS;
}

ErrorCode HiRecorderImpl::DoOnError(const Plugin::Any& param)
{
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        ptr->OnError(static_cast<IRecorderEngineObs::ErrorType>(RecorderErrorType::RECORDER_ERROR_INTERNAL),
                     TransErrorCode(Plugin::AnyCast<ErrorCode>(param)));
    }
    return ErrorCode::SUCCESS;
}
} // namespace Record
}  // namespace Media
}  // namespace OHOS
#endif