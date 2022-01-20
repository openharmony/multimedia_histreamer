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
#include "pipeline/factory/filter_factory.h"
#include "utils/steady_clock.h"

namespace OHOS {
namespace Media {
namespace Record {
using namespace Pipeline;

HiRecorderImpl::HiRecorderImpl() : fsm_(*this), curFsmState_(StateId::INIT)
{
    MEDIA_LOG_I("hiRecorderImpl ctor");
    FilterFactory::Instance().Init();
    muxer_ = FilterFactory::Instance().CreateFilterWithType<MuxerFilter>(
            "builtin.recorder.muxer", "muxer");
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
    ErrorCode ret = pipeline_->AddFilters({muxer_.get(), outputSink_.get()});
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkFilters({muxer_.get(), outputSink_.get()});
    }
    FALSE_LOG(ret == ErrorCode::SUCCESS);
    if (ret == ErrorCode::SUCCESS) {
        fsm_.SetStateCallback(this);
        fsm_.Start();
        initialized_ = true;
    } else {
        pipeline_->UnlinkPrevFilters();
        pipeline_->RemoveFilterChain(muxer_.get());
    }
    return ret;
}

int32_t HiRecorderImpl::SetAudioSource(AudioSourceType source, int32_t& sourceId)
{
    PROFILE_BEGIN("SetAudioSource begin");
    auto ret = ErrorCode::SUCCESS;
    audioCapture_ = FilterFactory::Instance().CreateFilterWithType<AudioCaptureFilter>(
            "builtin.recorder.audiocapture", "audiocapture");
    audioEncoder_ = FilterFactory::Instance().CreateFilterWithType<AudioEncoderFilter>(
            "builtin.recorder.audioencoder", "audioencoder");
    ret = pipeline_->AddFilters({audioCapture_.get(), audioEncoder_.get()});
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkFilters({audioCapture_.get(), audioEncoder_.get()});
    }
    std::shared_ptr<InPort> muxerInPort {nullptr};
    if (ret == ErrorCode::SUCCESS) {
        ret = muxer_->AddTrack(muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkPorts(audioEncoder_->GetOutPort(PORT_NAME_DEFAULT), muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        sourceId_++;
        sourceId = sourceId_;
        ret = fsm_.SendEvent(Intent::SET_AUDIO_SOURCE,
                             std::pair<int32_t, Plugin::SrcInputType> (sourceId, TransAudioInputType(source)));
    } else {
        sourceId = -1;
    }
    PROFILE_END("SetAudioSource end.");
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::SetVideoSource(VideoSourceType source, int32_t& sourceId)
{
#ifdef VIDEO_SUPPORT
    PROFILE_BEGIN("SetVideoSource begin");
    ErrorCode ret {ErrorCode::SUCCESS};
    videoCapture_ = FilterFactory::Instance().CreateFilterWithType<VideoCaptureFilter>(
        "builtin.recorder.videocapture", "videocapture");
    videoEncoder_ = FilterFactory::Instance().CreateFilterWithType<VideoEncoderFilter>(
        "builtin.recorder.videoencoder", "videoencoder");
    ret = pipeline_->AddFilters({videoCapture_.get(), videoEncoder_.get()});
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkFilters({videoCapture_.get(), videoEncoder_.get()});
    }
    std::shared_ptr<InPort> muxerInPort {nullptr};
    if (ret == ErrorCode::SUCCESS) {
        ret =muxer_->AddTrack(muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkPorts(videoEncoder_->GetOutPort(PORT_NAME_DEFAULT), muxerInPort);
    }
    if (ret == ErrorCode::SUCCESS) {
        sourceId_++;
        sourceId = sourceId_;
        ret = fsm_.SendEvent(Intent::SET_VIDEO_SOURCE,
                             std::pair<int32_t, Plugin::SrcInputType> (sourceId, TransVideoInputType(source)));
    } else {
        sourceId = -1;
    }
    PROFILE_END("SetVideoSource end.");
    return TransErrorCode(ret);
#else
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
#endif
}

sptr<Surface> HiRecorderImpl::GetSurface(int32_t sourceId)
{
}

int32_t HiRecorderImpl::SetOutputFormat(OutputFormatType format)
{
    containerMime_ = g_outputFormatToMimeMap.at(format);
    ErrorCode ret = fsm_.SendEvent(Intent::SET_OUTPUT_FORMAT, containerMime_);
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
    switch (recParam.type) {
        case RecorderPublicParamType::AUD_SAMPLERATE:
            any = RecordParam {sourceId, recParam.type, dynamic_cast<const AudSampleRate&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_CHANNEL:
            any = RecordParam {sourceId, recParam.type, dynamic_cast<const AudChannel&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_BITRATE:
            any = RecordParam {sourceId, recParam.type, dynamic_cast<const AudBitRate&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_ENC_FMT:
            any = RecordParam {sourceId, recParam.type, dynamic_cast<const AudEnc&>(recParam)};
            break;
        case RecorderPublicParamType::OUT_PATH:
            any = RecordParam {sourceId, recParam.type, dynamic_cast<const OutFilePath&>(recParam)};
            break;
        case RecorderPublicParamType::OUT_FD:
            any = RecordParam {sourceId, recParam.type, dynamic_cast<const OutFd&>(recParam)};
            break;
        default:
            MEDIA_LOG_E("ignore RecorderPublicParamType %d", recParam.type);
            return TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE);
    }
    auto ret = fsm_.SendEvent(Intent::CONFIGURE, any);
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
    if (curFsmState_ == StateId::PAUSE) {
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
    auto ret = TransErrorCode(fsm_.SendEvent(Intent::STOP, isDrainAll));
    PROFILE_END("Stop ret = %d", ret);
    return ret;
}

int32_t HiRecorderImpl::Reset()
{
    return Stop(false);
}

int32_t HiRecorderImpl::SetParameter(int32_t sourceId, const RecorderParam &recParam)
{
    return Configure(sourceId, recParam);
}

void HiRecorderImpl::OnEvent(Event event)
{
    MEDIA_LOG_D("[HiStreamer] OnEvent (%d)", event.type);
    switch (event.type) {
        case EVENT_ERROR: {
            fsm_.SendEventAsync(Intent::NOTIFY_ERROR, event.param);
            auto ptr = obs_.lock();
            if (ptr != nullptr) {
                ptr->OnError(IRecorderEngineObs::ErrorType::ERROR_INTERNAL,
                             TransErrorCode(Plugin::AnyCast<ErrorCode>(event.param)));
            }
            break;
        }
        case EVENT_READY: {
            fsm_.SendEventAsync(Intent::NOTIFY_READY);
            break;
        }
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
        ptr->OnInfo(IRecorderEngineObs::InfoType::INTERNEL_WARNING, to_underlying(state));
    }
}

ErrorCode HiRecorderImpl::DoSetVideoSource(const Plugin::Any& param) const
{
#ifdef VIDEO_SUPPORT
    using SrcInputPair = std::pair<int32_t, Plugin::SrcInputType>;
    if (param.Type() == typeid(SrcInputPair)) {
        auto srcType = Plugin::AnyCast<SrcInputPair>(param).second;
        return videoCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::SRC_INPUT_TYPE), srcType);
    } else {
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
#else
    return ErrorCode::ERROR_UNIMPLEMENTED;
#endif
}

ErrorCode HiRecorderImpl::DoSetAudioSource(const Plugin::Any& param) const
{
    using SrcInputPair = std::pair<int32_t, Plugin::SrcInputType>;
    if (param.Type() == typeid(SrcInputPair)) {
        auto srcType = Plugin::AnyCast<SrcInputPair>(param).second;
        return audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::SRC_INPUT_TYPE), srcType);
    } else {
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
}

ErrorCode HiRecorderImpl::DoConfigure(const Plugin::Any &param) const
{
    ErrorCode ret  = ErrorCode::SUCCESS;
    int32_t sourceId;
    RecordParam recordParam;
    Plugin::Any any;
    if (param.Type() == typeid(RecordParam)) {
        recordParam = Plugin::AnyCast<RecordParam>(param);
    } else {
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    sourceId =  recordParam.sourceId;
    any = recordParam.any;
    switch (recordParam.type) {
        case RecorderPublicParamType::AUD_SAMPLERATE:
            ret = audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_RATE),
                                              static_cast<uint32_t>(Plugin::AnyCast<AudSampleRate>(any).sampleRate));
            break;
        case RecorderPublicParamType::AUD_CHANNEL:
            ret = audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNELS),
                                              static_cast<uint32_t>(Plugin::AnyCast<AudChannel>(any).channel));
            break;
        case RecorderPublicParamType::AUD_BITRATE:
            ret = audioCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_BITRATE),
                                              static_cast<int64_t>(Plugin::AnyCast<AudBitRate>(any).bitRate));
            break;
        case RecorderPublicParamType::AUD_ENC_FMT: {
            auto encoderMeta = std::make_shared<Plugin::Meta>();
            if (!TransAudioEncoderFmt(Plugin::AnyCast<AudEnc>(any).encFmt, *encoderMeta)) {
                ret = ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
                break;
            }
            ret = audioEncoder_->SetAudioEncoder(sourceId,encoderMeta);
            break;
        }
        case RecorderPublicParamType::OUT_PATH: {
            auto filePath = Plugin::AnyCast<OutFilePath>(any).path;
            if (IsDirectory(filePath)) {
                filePath = ConvertDirPathToFilePath(filePath, containerMime_);
            }
            ret = outputSink_->SetOutputPath(filePath);
            break;
        }
        case RecorderPublicParamType::OUT_FD:
            ret = outputSink_->SetFd(Plugin::AnyCast<OutFd>(any).fd);
            break;
        default:
            break;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoSetOutputFormat(const Plugin::Any& param) const
{
    ErrorCode ret  = ErrorCode::SUCCESS;
    if (param.Type() == typeid(std::string)) {
        ret = muxer_->SetOutputFormat(Plugin::AnyCast<std::string>(param));
    } else {
        ret = ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetOutputFormat failed with error %d", static_cast<int>(ret));
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoPrepare()
{
    return pipeline_->Prepare();
}

ErrorCode HiRecorderImpl::DoStart()
{
    return pipeline_->Start();
}

ErrorCode HiRecorderImpl::DoPause()
{
    return pipeline_->Pause();
}

ErrorCode HiRecorderImpl::DoResume()
{
    return pipeline_->Resume();
}

ErrorCode HiRecorderImpl::DoStop(const Plugin::Any& param)
{
    ErrorCode ret = ErrorCode::SUCCESS;
    if (Plugin::AnyCast<bool>(param)) {
       ret = audioCapture_->SendEos();
#ifdef VIDEO_SUPPORT
       if (ret == ErrorCode::SUCCESS) {
           ret = videoCapture_->SendEos();
       }
#endif
    } else {
        ret = muxer_->SendEos();
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoOnComplete()
{
    return pipeline_->Stop();
}
} // namespace Record
} // namespace Media
} // namespace OHOS
#endif