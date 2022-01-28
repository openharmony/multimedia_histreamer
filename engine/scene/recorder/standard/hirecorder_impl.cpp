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
    mediaStatStub_.Reset();
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
    sourceId = INVALID_SOURCE_ID;
    FALSE_RETURN_V(source != AudioSourceType::AUDIO_SOURCE_INVALID,
                   TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE));
    FALSE_RETURN_V(audioCount_ < AUDIO_SOURCE_MAX_COUNT, TransErrorCode(ErrorCode::ERROR_INVALID_OPERATION));
    auto handle = HandleGenerator::GenerateAudioHandle(audioCount_);
    auto ret = SetAudioSourceInternal(source, handle);
    if (ret == ErrorCode::SUCCESS) {
        audioCount_++;
        audioSourceId_ = handle;
        sourceId = audioSourceId_;
    }
    PROFILE_END("SetAudioSource end.");
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::SetVideoSource(VideoSourceType source, int32_t& sourceId)
{
#ifdef VIDEO_SUPPORT
    PROFILE_BEGIN("SetVideoSource begin");
    sourceId = INVALID_SOURCE_ID;
    FALSE_RETURN_V(source != VideoSourceType::VIDEO_SOURCE_BUTT,
                   TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE));
    FALSE_RETURN_V(videoCount_ < VIDEO_SOURCE_MAX_COUNT, TransErrorCode(ErrorCode::ERROR_INVALID_OPERATION));
    auto handle = HandleGenerator::GenerateVideoHandle(videoCount_);
    auto ret = SetVideoSourceInternal(source, handle);
    if (ret == ErrorCode::SUCCESS) {
        videoCount_++;
        videoSourceId_ = handle;
        sourceId = videoSourceId_;
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
    FALSE_RETURN_V(format != OutputFormatType::FORMAT_BUTT, TransErrorCode(ErrorCode::ERROR_INVALID_OPERATION));
    FALSE_RETURN_V((audioCount_+ videoCount_) > 0, TransErrorCode(ErrorCode::ERROR_INVALID_OPERATION));
    outputFormatType_ = format;
    auto ret = fsm_.SendEvent(Intent::SET_OUTPUT_FORMAT, outputFormatType_);
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetOutputFormat failed with error %" PUBLIC_OUTPUT "d", static_cast<int>(ret));
    }
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::SetObs(const std::weak_ptr<IRecorderEngineObs>& obs)
{
    obs_ = obs;
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiRecorderImpl::Configure(int32_t sourceId, const RecorderParam& recParam)
{
    FALSE_RETURN_V(outputFormatType_ != OutputFormatType::FORMAT_BUTT,
                   TransErrorCode(ErrorCode::ERROR_INVALID_OPERATION));
    FALSE_RETURN_V(CheckParamType(sourceId,recParam), TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE));
    Plugin::Any recParamInternal;
    auto configureStatus{true};
    if (recParam.IsAudioParam()) {
        configureStatus = ConfigureAudio(sourceId, recParam, recParamInternal);
    } else if (recParam.IsVideoParam()) {
        configureStatus = ConfigureVideo(sourceId, recParam, recParamInternal);
    } else {
        configureStatus = ConfigureOther(sourceId, recParam, recParamInternal);
    }
    FALSE_RETURN_V(configureStatus, TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE));
    auto ret = fsm_.SendEvent(Intent::CONFIGURE, recParamInternal);
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Configure failed with error %" PUBLIC_OUTPUT "d", static_cast<int>(ret));
    }
    return TransErrorCode(ret);
}

int32_t HiRecorderImpl::Prepare()
{
    MEDIA_LOG_D("Prepare entered, current fsm state: %" PUBLIC_OUTPUT "s.", fsm_.GetCurrentState().c_str());
    PROFILE_BEGIN();
    auto ret = fsm_.SendEvent(Intent::PREPARE);
    if (ret != ErrorCode::SUCCESS) {
        PROFILE_END("Prepare failed,");
        MEDIA_LOG_E("prepare failed with error %" PUBLIC_OUTPUT "d", ret);
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
    outputFormatType_ = OutputFormatType::FORMAT_BUTT;
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

void HiRecorderImpl::OnEvent(const Event& event)
{
    MEDIA_LOG_D("[HiStreamer] OnEvent (%" PUBLIC_OUTPUT "d)", event.type);
    switch (event.type) {
        case EventType::EVENT_ERROR: {
            fsm_.SendEventAsync(Intent::NOTIFY_ERROR, event.param);
            auto ptr = obs_.lock();
            if (ptr != nullptr) {
                ptr->OnError(IRecorderEngineObs::ErrorType::ERROR_INTERNAL,
                             TransErrorCode(Plugin::AnyCast<ErrorCode>(event.param)));
            }
            break;
        }
        case EventType::EVENT_READY: {
            fsm_.SendEventAsync(Intent::NOTIFY_READY);
            break;
        }
        case EventType::EVENT_COMPLETE:
            mediaStatStub_.ReceiveEvent(EventType::EVENT_COMPLETE, 0);
            if (mediaStatStub_.IsEventCompleteAllReceived()) {
                fsm_.SendEventAsync(Intent::NOTIFY_COMPLETE);
            }
            break;
        default:
            MEDIA_LOG_E("Unknown event(%" PUBLIC_OUTPUT "d)", event.type);
    }
}

void HiRecorderImpl::OnStateChanged(StateId state)
{
    MEDIA_LOG_I("OnStateChanged from %" PUBLIC_OUTPUT "d to %" PUBLIC_OUTPUT "d", curFsmState_.load(), state);
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
    RecorderParamInternal recParamInternal;
    if (param.Type() == typeid(RecorderParamInternal)) {
        recParamInternal = Plugin::AnyCast<RecorderParamInternal>(param);
    } else {
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    switch (recParamInternal.type) {
        case RecorderPublicParamType::AUD_SAMPLERATE:
        case RecorderPublicParamType::AUD_CHANNEL:
        case RecorderPublicParamType::AUD_BITRATE:
        case RecorderPublicParamType::AUD_ENC_FMT:
            ret = DoConfigureAudio(recParamInternal);
            break;
        case RecorderPublicParamType::VID_CAPTURERATE:
        case RecorderPublicParamType::VID_RECTANGLE:
        case RecorderPublicParamType::VID_BITRATE:
        case RecorderPublicParamType::VID_FRAMERATE:
        case RecorderPublicParamType::VID_ENC_FMT:
            ret = DoConfigureVideo(recParamInternal);
            break;
        case RecorderPublicParamType::OUT_PATH:
        case RecorderPublicParamType::OUT_FD:
        case RecorderPublicParamType::VID_ORIENTATION_HINT:
        case RecorderPublicParamType::GEO_LOCATION:
            ret = DoConfigureOther(recParamInternal);
            break;
        default:
            break;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoSetOutputFormat(const Plugin::Any& param) const
{
    ErrorCode ret {ErrorCode::SUCCESS};
    if (param.Type() == typeid(OutputFormatType)) {
        auto outputFormatType = Plugin::AnyCast<OutputFormatType>(param);
        if (g_outputFormatToMimeMap.find(outputFormatType) != g_outputFormatToMimeMap.end()) {
            ret = muxer_->SetOutputFormat(g_outputFormatToMimeMap.at(outputFormatType));
        } else {
            ret = ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
        }
    } else {
        ret = ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetOutputFormat failed with error %" PUBLIC_OUTPUT "d", static_cast<int>(ret));
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
    mediaStatStub_.Reset();
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

ErrorCode HiRecorderImpl::SetAudioSourceInternal(AudioSourceType source, int handle)
{
    audioCapture_ = FilterFactory::Instance().CreateFilterWithType<AudioCaptureFilter>(
            "builtin.recorder.audiocapture", "audiocapture");
    audioEncoder_ = FilterFactory::Instance().CreateFilterWithType<AudioEncoderFilter>(
            "builtin.recorder.audioencoder", "audioencoder");
    auto ret = pipeline_->AddFilters({audioCapture_.get(), audioEncoder_.get()});
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
        ret = fsm_.SendEvent(Intent::SET_AUDIO_SOURCE, std::pair<int32_t, Plugin::SrcInputType>(
                handle,TransAudioInputType(source)));
    }
    return ret;
}

ErrorCode HiRecorderImpl::SetVideoSourceInternal(VideoSourceType source, int handle)
{
#ifdef VIDEO_SUPPORT
    videoCapture_ = FilterFactory::Instance().CreateFilterWithType<VideoCaptureFilter>(
            "builtin.recorder.videocapture", "videocapture");
    videoEncoder_ = FilterFactory::Instance().CreateFilterWithType<VideoEncoderFilter>(
            "builtin.recorder.videoencoder", "videoencoder");
    auto ret = pipeline_->AddFilters({videoCapture_.get(), videoEncoder_.get()});
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
        ret = fsm_.SendEvent(Intent::SET_AUDIO_SOURCE, std::pair<int32_t, Plugin::SrcInputType>(
                handle,TransVideoInputType(source)));
    }
    return ret;
#else
    return ErrorCode::ERROR_UNIMPLEMENTED;
#endif
}

bool HiRecorderImpl::ConfigureAudio(int32_t sourceId, const RecorderParam& recParam, Plugin::Any& recParamInternal)
{
    auto ret{true};
    switch (recParam.type) {
        case RecorderPublicParamType::AUD_SAMPLERATE:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const AudSampleRate&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_CHANNEL:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const AudChannel&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_BITRATE:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const AudBitRate&>(recParam)};
            break;
        case RecorderPublicParamType::AUD_ENC_FMT:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const AudEnc&>(recParam)};
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

bool HiRecorderImpl::ConfigureVideo(int32_t sourceId, const RecorderParam& recParam, Plugin::Any& recParamInternal)
{
    auto ret{true};
    switch (recParam.type) {
        case RecorderPublicParamType::VID_CAPTURERATE:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const CaptureRate&>(recParam)};
            break;
        case RecorderPublicParamType::VID_RECTANGLE:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const VidRectangle&>(recParam)};
            break;
        case RecorderPublicParamType::VID_BITRATE:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const VidBitRate&>(recParam)};
            break;
        case RecorderPublicParamType::VID_FRAMERATE:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const VidFrameRate&>(recParam)};
            break;
        case RecorderPublicParamType::VID_ENC_FMT:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const VidEnc&>(recParam)};
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

bool HiRecorderImpl::ConfigureOther(int32_t sourceId, const RecorderParam& recParam, Plugin::Any& recParamInternal)
{
    auto ret{true};
    switch (recParam.type) {
        case RecorderPublicParamType::OUT_PATH:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const OutFilePath&>(recParam)};
            break;
        case RecorderPublicParamType::OUT_FD:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const OutFd&>(recParam)};
            break;
        case RecorderPublicParamType::VID_ORIENTATION_HINT:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const RotationAngle&>(recParam)};
            break;
        case RecorderPublicParamType::GEO_LOCATION:
            recParamInternal = RecorderParamInternal{sourceId, recParam.type,
                                                     dynamic_cast<const GeoLocation&>(recParam)};
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoConfigureAudio(const RecorderParamInternal& recParamInternal) const
{
    ErrorCode ret  = ErrorCode::SUCCESS;
    Plugin::Any any{recParamInternal.any};
    switch (recParamInternal.type) {
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
            ret = audioEncoder_->SetAudioEncoder(recParamInternal.sourceId,encoderMeta);
            break;
        }
        default:
            break;
    }
    return ret;
}

ErrorCode HiRecorderImpl::DoConfigureVideo(const RecorderParamInternal& recParamInternal) const
{
#ifdef VIDEO_SUPPORT
    ErrorCode ret  = ErrorCode::SUCCESS;
    Plugin::Any any{recParamInternal.any};
    switch (recParamInternal.type) {
        case RecorderPublicParamType::VID_RECTANGLE: {
            auto vidRectangle = Plugin::AnyCast<VidRectangle>(any);
            ret = videoCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_WIDTH),
                                              static_cast<uint32_t>(vidRectangle.width));
            if (ret != ErrorCode::SUCCESS) {
                ret = videoCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_HEIGHT),
                                                  static_cast<uint32_t>(vidRectangle.height));
            }
            break;
        }
        case RecorderPublicParamType::VID_FRAMERATE:
            ret = videoCapture_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_FRAME_RATE),
                                              static_cast<uint64_t>(Plugin::AnyCast<VidFrameRate>(any).frameRate));
            break;
        case RecorderPublicParamType::VID_CAPTURERATE:
        case RecorderPublicParamType::VID_BITRATE:
        case RecorderPublicParamType::VID_ENC_FMT:
            MEDIA_LOG_E("ignore RecorderPublicParamType %d", recParamInternal.type);
            break;
        default:
            break;
    }
    return ret;
#else
    return ErrorCode::ERROR_UNIMPLEMENTED;
#endif
}

ErrorCode HiRecorderImpl::DoConfigureOther(const RecorderParamInternal& recParamInternal) const
{
    ErrorCode ret  = ErrorCode::SUCCESS;
    Plugin::Any any{recParamInternal.any};
    switch (recParamInternal.type) {
        case RecorderPublicParamType::OUT_PATH: {
            auto filePath = Plugin::AnyCast<OutFilePath>(any).path;
            if (IsDirectory(filePath)) {
                std::string dirPath{filePath};
                if (!ConvertDirPathToFilePath(dirPath, outputFormatType_, filePath)) {
                    ret = ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
                    break;
                }
            }
            ret = outputSink_->SetOutputPath(filePath);
            break;
        }
        case RecorderPublicParamType::OUT_FD:
            ret = outputSink_->SetFd(Plugin::AnyCast<OutFd>(any).fd);
            break;
        case RecorderPublicParamType::VID_ORIENTATION_HINT:
        case RecorderPublicParamType::GEO_LOCATION:
            MEDIA_LOG_E("ignore RecorderPublicParamType %d", recParamInternal.type);
            break;
        default:
            break;
    }
    return ret;
}

bool HiRecorderImpl::CheckParamType(int32_t sourceId, const RecorderParam& recParam) const
{
    FALSE_RETURN_V((HandleGenerator::IsAudio(sourceId) && recParam.IsAudioParam() && audioSourceId_ == sourceId) ||
        (HandleGenerator::IsVideo(sourceId) && recParam.IsVideoParam() && videoSourceId_ == sourceId) ||
        ((sourceId == DUMMY_SOURCE_ID) && !(recParam.IsAudioParam() || recParam.IsVideoParam())), false);

    return true;
}
} // namespace Record
} // namespace Media
} // namespace OHOS
#endif