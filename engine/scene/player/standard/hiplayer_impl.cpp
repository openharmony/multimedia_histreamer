
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

#define HST_LOG_TAG "HiPlayerImpl"

#include "hiplayer_impl.h"
#include "foundation/log.h"
#include "pipeline/core/clock_manager.h"
#include "pipeline/factory/filter_factory.h"
#include "scene/player/standard/media_utils.h"
#include "plugin/common/media_source.h"
#include "plugin/common/plugin_time.h"
#include "plugin/core/plugin_meta.h"
#include "utils/steady_clock.h"
#include "media_errors.h"

namespace {
const float MAX_MEDIA_VOLUME = 1.0f; // standard interface volume is between 0 to 1.
}

namespace OHOS {
namespace Media {
using namespace Pipeline;

HiPlayerImpl::HiPlayerImpl()
    : fsm_(*this),
      curFsmState_(StateId::INIT),
      pipelineStates_(PlayerStates::PLAYER_IDLE),
      volume_(-1.0f),
      errorCode_(ErrorCode::SUCCESS),
      mediaStats_()
{
    MEDIA_LOG_I("hiPlayerImpl ctor");
    FilterFactory::Instance().Init();

    audioSource_ =
        FilterFactory::Instance().CreateFilterWithType<MediaSourceFilter>("builtin.player.mediasource", "mediaSource");

#ifdef UNIT_TEST
    demuxer_ = FilterFactory::Instance().CreateFilterWithType<DemuxerFilter>("builtin.player.demuxer", "demuxer");
    audioDecoder_ = FilterFactory::Instance().CreateFilterWithType<AudioDecoderFilter>("builtin.player.audiodecoder",
                                                                                       "audiodecoder");
    audioSink_ =
        FilterFactory::Instance().CreateFilterWithType<AudioSinkFilter>("builtin.player.audiosink", "audiosink");
#else
    demuxer_ = FilterFactory::Instance().CreateFilterWithType<DemuxerFilter>("builtin.player.demuxer", "demuxer");
    audioSink_ =
        FilterFactory::Instance().CreateFilterWithType<AudioSinkFilter>("builtin.player.audiosink", "audioSink");
#ifdef VIDEO_SUPPORT
    videoSink_ =
        FilterFactory::Instance().CreateFilterWithType<VideoSinkFilter>("builtin.player.videosink", "videoSink");
    FALSE_RETURN(videoSink_ != nullptr);
#endif
#endif
    FALSE_RETURN(audioSource_ != nullptr);
    FALSE_RETURN(demuxer_ != nullptr);
    FALSE_RETURN(audioSink_ != nullptr);

    pipeline_ = std::make_shared<PipelineCore>();
}

HiPlayerImpl::~HiPlayerImpl()
{
    MEDIA_LOG_I("dtor called.");
    fsm_.SendEventAsync(Intent::STOP, {});
    fsm_.Stop();
    ClockManager::Instance().ClearProviders();
}

ErrorCode HiPlayerImpl::Init()
{
    MEDIA_LOG_I("Init entered.");
    errorCode_ = ErrorCode::SUCCESS;
    mediaStats_.Reset();
    if (initialized_.load()) {
        return ErrorCode::SUCCESS;
    }
    pipeline_->Init(this, this);
    ErrorCode ret = pipeline_->AddFilters({audioSource_.get(), demuxer_.get()});
    if (ret == ErrorCode::SUCCESS) {
        ret = pipeline_->LinkFilters({audioSource_.get(), demuxer_.get()});
    }
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_INITIALIZED;
        fsm_.SetStateCallback(this);
        fsm_.Start();
        initialized_ = true;
    } else {
        pipeline_->UnlinkPrevFilters();
        pipeline_->RemoveFilterChain(audioSource_.get());
        pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
    }
    ReportStateChanged();
    return ret;
}

int32_t HiPlayerImpl::SetSource(const std::string& uri)
{
    MEDIA_LOG_I("SetSource entered source uri: " PUBLIC_LOG_S, uri.c_str());
    PROFILE_BEGIN("SetSource begin");
    auto ret = Init();
    if (ret == ErrorCode::SUCCESS) {
        ret = fsm_.SendEvent(Intent::SET_SOURCE, std::make_shared<MediaSource>(uri));
    }
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetSource error: " PUBLIC_LOG_S, GetErrorName(ret));
    }
    PROFILE_END("SetSource end.");
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::SetSource(const std::shared_ptr<IMediaDataSource>& dataSrc)
{
    MEDIA_LOG_W("SetSource only support url format source!");
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}

int32_t HiPlayerImpl::Prepare()
{
    MEDIA_LOG_I("Prepare entered, current fsm state: " PUBLIC_LOG_S ".", fsm_.GetCurrentState().c_str());
    PROFILE_BEGIN();
    auto ret = fsm_.SendEvent(Intent::PREPARE);
    if (ret != ErrorCode::SUCCESS) {
        PROFILE_END("Prepare failed,");
        MEDIA_LOG_E("prepare failed with error " PUBLIC_LOG_D32, ret);
    } else {
        PROFILE_END("Prepare successfully,");
    }
    return TransErrorCode(ret);
}

int HiPlayerImpl::PrepareAsync()
{
    MEDIA_LOG_I("Prepare async entered, current fsm state: " PUBLIC_LOG_S, fsm_.GetCurrentState().c_str());
    PROFILE_BEGIN();
    auto ret = fsm_.SendEventAsync(Intent::PREPARE);
    if (ret != ErrorCode::SUCCESS) {
        PROFILE_END("Prepare async failed,");
        MEDIA_LOG_E("Prepare async failed with error " PUBLIC_LOG_D32, ret);
    } else {
        PROFILE_END("Prepare async successfully,");
    }
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::Play()
{
    MEDIA_LOG_I("Play entered.");
    PROFILE_BEGIN();
    ErrorCode ret;
    if (pipelineStates_ == PlayerStates::PLAYER_PAUSED) {
        ret = fsm_.SendEvent(Intent::RESUME);
    } else {
        ret = fsm_.SendEvent(Intent::PLAY);
    }
    PROFILE_END("Play ret = " PUBLIC_LOG_D32, TransErrorCode(ret));
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::Pause()
{
    MEDIA_LOG_I("Pause entered.");
    PROFILE_BEGIN();
    auto ret = TransErrorCode(fsm_.SendEvent(Intent::PAUSE));
    PROFILE_END("Pause ret = " PUBLIC_LOG_D32, ret);
    return ret;
}

int32_t HiPlayerImpl::Stop()
{
    MEDIA_LOG_I("Stop entered.");
    PROFILE_BEGIN();
    auto ret = TransErrorCode(fsm_.SendEvent(Intent::STOP));
    PROFILE_END("Stop ret = " PUBLIC_LOG_D32, ret);
    return ret;
}

ErrorCode HiPlayerImpl::StopAsync()
{
    MEDIA_LOG_I("StopAsync entered.");
    return fsm_.SendEventAsync(Intent::STOP);
}

int32_t HiPlayerImpl::Seek(int32_t mSeconds, PlayerSeekMode mode)
{
    MEDIA_LOG_I("Seek entered. mSeconds : " PUBLIC_LOG_D32 ", seekMode : " PUBLIC_LOG_D32,
                mSeconds, static_cast<int32_t>(mode));
    int64_t hstTime = 0;
    int32_t durationMs = 0;
    if (!Plugin::Ms2HstTime(mSeconds, hstTime)) {
        return TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE);
    }
    NZERO_RETURN(GetDuration(durationMs));
    MEDIA_LOG_I("Seek durationMs : " PUBLIC_LOG_D32, durationMs);
    FALSE_RETURN_V_MSG_E(mSeconds <= durationMs, CppExt::to_underlying(ErrorCode::ERROR_INVALID_PARAMETER_VALUE),
                         "mSeconds : " PUBLIC_LOG_D32 ", durationMs : " PUBLIC_LOG_D32, mSeconds, durationMs);
    auto smode = Transform2SeekMode(mode);
    return TransErrorCode(fsm_.SendEventAsync(Intent::SEEK, SeekInfo{hstTime, smode}));
}

int32_t HiPlayerImpl::SetVolume(float leftVolume, float rightVolume)
{
    MEDIA_LOG_I("SetVolume entered.");
    if (leftVolume < 0 || leftVolume > MAX_MEDIA_VOLUME || rightVolume < 0 || rightVolume > MAX_MEDIA_VOLUME) {
        MEDIA_LOG_E("volume not valid, should be in range [0,100]");
        return TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE);
    }
    if (leftVolume < 1e-6 && rightVolume >= 1e-6) {  // 1e-6
        volume_ = rightVolume;
    } else if (rightVolume < 1e-6 && leftVolume >= 1e-6) {  // 1e-6
        volume_ = leftVolume;
    } else {
        volume_ = (leftVolume + rightVolume) / 2;  // 2
    }
    volume_ /= MAX_MEDIA_VOLUME;  // normalize to 0~1
    PlayerStates states = pipelineStates_.load();
    if (states != Media::PlayerStates::PLAYER_STARTED) {
        return TransErrorCode(ErrorCode::SUCCESS);
    }
    return TransErrorCode(SetVolume(volume_));
}

int32_t HiPlayerImpl::SetVideoSurface(sptr<Surface> surface)
{
    MEDIA_LOG_D("SetVideoSurface entered.");
#ifdef VIDEO_SUPPORT
    return TransErrorCode(videoSink_->SetVideoSurface(surface));
#else
    return TransErrorCode(ErrorCode::SUCCESS);
#endif
}

int32_t HiPlayerImpl::GetVideoTrackInfo(std::vector<Format> &videoTrack)
{
    MEDIA_LOG_I("GetVideoTrackInfo entered.");
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}
int32_t HiPlayerImpl::GetAudioTrackInfo(std::vector<Format> &audioTrack)
{
    MEDIA_LOG_I("GetAudioTrackInfo entered.");
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}
int32_t HiPlayerImpl::GetVideoWidth()
{
    MEDIA_LOG_I("GetVideoWidth entered.");
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}
int32_t HiPlayerImpl::GetVideoHeight()
{
    MEDIA_LOG_I("GetVideoHeight entered.");
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}

void HiPlayerImpl::HandleAudioProgressEvent(const Event& event)
{
    mediaStats_.ReceiveEvent(EventType::EVENT_AUDIO_PROGRESS,
                             Plugin::AnyCast<int64_t>(event.param));
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        Format format;
        int32_t currentPositionMs;
        (void)GetCurrentTime(currentPositionMs);
        MEDIA_LOG_DD("EVENT_AUDIO_PROGRESS position updated: " PUBLIC_LOG_D32, currentPositionMs);
        ptr->OnInfo(INFO_TYPE_POSITION_UPDATE, currentPositionMs, format);
    }
}

void HiPlayerImpl::HandlePluginErrorEvent(const Event& event)
{
    Plugin::PluginEvent pluginEvent = Plugin::AnyCast<Plugin::PluginEvent>(event.param);
    MEDIA_LOG_I("Receive PLUGIN_ERROR, type:  " PUBLIC_LOG_D32, CppExt::to_underlying(pluginEvent.type));
    if (pluginEvent.type == Plugin::PluginEventType::CLIENT_ERROR &&
        pluginEvent.param.SameTypeWith(typeid(Plugin::NetworkClientErrorCode))) {
        auto netClientErrorCode = Plugin::AnyCast<Plugin::NetworkClientErrorCode>(pluginEvent.param);
        auto errorType {PlayerErrorType::PLAYER_ERROR_UNKNOWN};
        auto serviceErrCode { MSERR_UNKNOWN };
        if (netClientErrorCode == Plugin::NetworkClientErrorCode::ERROR_TIME_OUT) {
            errorType = PlayerErrorType::PLAYER_ERROR;
            serviceErrCode = MSERR_NETWORK_TIMEOUT;
        }
        auto ptr = obs_.lock();
        if (ptr != nullptr) {
            ptr->OnError(errorType, serviceErrCode);
        }
    }
}

void HiPlayerImpl::OnEvent(const Event& event)
{
    if (event.type != EventType::EVENT_AUDIO_PROGRESS) {
        MEDIA_LOG_I("[HiStreamer] OnEvent (" PUBLIC_LOG_S ")", GetEventName(event.type));
    }
    switch (event.type) {
        case EventType::EVENT_ERROR: {
            fsm_.SendEventAsync(Intent::NOTIFY_ERROR, event.param);
            break;
        }
        case EventType::EVENT_READY:
            fsm_.SendEventAsync(Intent::NOTIFY_READY);
            break;
        case EventType::EVENT_COMPLETE:
            mediaStats_.ReceiveEvent(EventType::EVENT_COMPLETE, 0);
            if (mediaStats_.IsEventCompleteAllReceived()) {
                fsm_.SendEventAsync(Intent::NOTIFY_COMPLETE);
            }
            break;
        case EventType::EVENT_AUDIO_PROGRESS: {
            HandleAudioProgressEvent(event);
            break;
        }
        case EventType::EVENT_PLUGIN_ERROR: {
            HandlePluginErrorEvent(event);
            break;
        }
        case EventType::EVENT_PLUGIN_EVENT: {
            Plugin::PluginEvent pluginEvent = Plugin::AnyCast<Plugin::PluginEvent>(event.param);
            if (pluginEvent.type == Plugin::PluginEventType::BELOW_LOW_WATERLINE ||
                pluginEvent.type == Plugin::PluginEventType::ABOVE_LOW_WATERLINE) {
                MEDIA_LOG_I("Receive PLUGIN_EVENT, type:  " PUBLIC_LOG_D32, CppExt::to_underlying(pluginEvent.type));
            }
            break;
        }
        default:
            MEDIA_LOG_E("Unknown event(" PUBLIC_LOG_U32 ")", event.type);
    }
}

ErrorCode HiPlayerImpl::DoSetSource(const std::shared_ptr<MediaSource>& source)
{
    auto ret = audioSource_->SetSource(source);
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_PREPARING;
    } else {
        pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
        ReportStateChanged();
    }
    return ret;
}

ErrorCode HiPlayerImpl::PrepareFilters()
{
    auto ret = pipeline_->Prepare();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_PREPARED;
    } else {
        pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
        ReportStateChanged();
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoPlay()
{
    (void)SetVolume(volume_);
    auto ret = pipeline_->Start();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_STARTED;
    } else {
        pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
        ReportStateChanged();
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoPause()
{
    auto ret = pipeline_->Pause();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_PAUSED;
    } else {
        pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
        ReportStateChanged();
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoResume()
{
    (void)SetVolume(volume_);
    auto ret = pipeline_->Resume();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_STARTED;
    } else {
        pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
        ReportStateChanged();
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoStop()
{
    mediaStats_.Reset();
    auto ret = pipeline_->Stop();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_STOPPED;
    } else {
        pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
        ReportStateChanged();
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoSeek(bool allowed, int64_t hstTime, Plugin::SeekMode mode)
{
    PROFILE_BEGIN();
    auto rtv = allowed && hstTime >= 0 ? ErrorCode::SUCCESS : ErrorCode::ERROR_INVALID_OPERATION;
    if (rtv == ErrorCode::SUCCESS) {
        pipeline_->FlushStart();
        PROFILE_END("Flush start");
        PROFILE_RESET();
        pipeline_->FlushEnd();
        PROFILE_END("Flush end");
        PROFILE_RESET();
        rtv = demuxer_->SeekTo(hstTime, mode);
        if (rtv == ErrorCode::SUCCESS) {
            mediaStats_.ReceiveEvent(EventType::EVENT_AUDIO_PROGRESS, hstTime);
            mediaStats_.ReceiveEvent(EventType::EVENT_VIDEO_PROGRESS, hstTime);
        }
        PROFILE_END("SeekTo");
    }
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        if (rtv != ErrorCode::SUCCESS) {
            ptr->OnError(PLAYER_ERROR, TransErrorCode(rtv));
        } else {
            Format format;
            int64_t currentPos = Plugin::HstTime2Ms(mediaStats_.GetCurrentPosition());
            MEDIA_LOG_I("Seek done, currentPos : " PUBLIC_LOG_D64, currentPos);
            ptr->OnInfo(INFO_TYPE_SEEKDONE, static_cast<int32_t>(currentPos), format);
        }
    }
    return rtv;
}

void HiPlayerImpl::ReportStateChanged()
{
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        Format format;
        MEDIA_LOG_I("State change to : " PUBLIC_LOG_D32, pipelineStates_.load());
        ptr->OnInfo(INFO_TYPE_STATE_CHANGE, pipelineStates_, format);
    }
}

ErrorCode HiPlayerImpl::DoOnReady()
{
    pipelineStates_ = PlayerStates::PLAYER_PREPARED;
    sourceMeta_ = demuxer_->GetGlobalMetaInfo();
    streamMeta_.clear();
    for (auto& streamMeta : demuxer_->GetStreamMetaInfo()) {
        streamMeta_.push_back(streamMeta);
    }
    ReportStateChanged();
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::DoOnComplete()
{
    MEDIA_LOG_I("OnComplete looping: " PUBLIC_LOG_D32 ".", singleLoop_.load());
    if (!singleLoop_) {
        StopAsync();
    } else {
        fsm_.SendEventAsync(Intent::SEEK, SeekInfo{0, Plugin::SeekMode::SEEK_PREVIOUS_SYNC});
    }
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        Format format;
        ptr->OnInfo(INFO_TYPE_EOS, static_cast<int32_t>(singleLoop_.load()), format);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::DoOnError(ErrorCode errorCode)
{
    errorCode_ = errorCode;
    pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
    ReportStateChanged();
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::SetVolume(float volume)
{
    MEDIA_LOG_I("SetVolume entered.");
    if (audioSink_ == nullptr) {
        MEDIA_LOG_W("cannot set volume while audio sink filter is null");
        return ErrorCode::ERROR_AGAIN;
    }
    ErrorCode ret = ErrorCode::SUCCESS;
    if (volume > 0) {
        MEDIA_LOG_I("set volume " PUBLIC_LOG_F, volume);
        ret = audioSink_->SetVolume(volume);
    }
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        if (ret != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("SetVolume failed with error " PUBLIC_LOG_D32, static_cast<int>(ret));
            ptr->OnError(PLAYER_ERROR, TransErrorCode(ret));
        } else {
            Format format;
            ptr->OnInfo(INFO_TYPE_VOLUME_CHANGE, volume, format);
        }
    }
    return ret;
}

PFilter HiPlayerImpl::CreateAudioDecoder(const std::string& desc)
{
    if (!audioDecoderMap_[desc]) {
        audioDecoderMap_[desc] = FilterFactory::Instance().CreateFilterWithType<AudioDecoderFilter>(
            "builtin.player.audiodecoder", "audiodecoder-" + desc);
        // set parameters to decoder.
    }
    return audioDecoderMap_[desc];
}

int32_t HiPlayerImpl::SetLooping(bool loop)
{
    MEDIA_LOG_I("SetLooping entered.");
    singleLoop_ = loop;
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::SetParameter(const Format& params)
{
    MEDIA_LOG_I("SetParameter entered.");
    return CppExt::to_underlying(ErrorCode::ERROR_UNIMPLEMENTED);
}

int32_t HiPlayerImpl::SetObs(const std::weak_ptr<IPlayerEngineObs>& obs)
{
    MEDIA_LOG_I("SetObs entered.");
    obs_ = obs;
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::Reset()
{
    MEDIA_LOG_I("Reset entered.");
    Stop();
    pipelineStates_ = PlayerStates::PLAYER_IDLE;
    singleLoop_ = false;
    mediaStats_.Reset();
    ReportStateChanged();
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::GetCurrentTime(int32_t& currentPositionMs)
{
    currentPositionMs = Plugin::HstTime2Ms(mediaStats_.GetCurrentPosition());
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::GetDuration(int32_t& durationMs)
{
    if (audioSource_ && !audioSource_->IsSeekable()) {
        durationMs = -1;
        return TransErrorCode(ErrorCode::SUCCESS);
    }
    uint64_t duration = 0;
    auto sourceMeta = sourceMeta_.lock();
    if (sourceMeta == nullptr) {
        durationMs = 0;
        return TransErrorCode(ErrorCode::ERROR_AGAIN);
    }
    if (sourceMeta->GetUint64(Media::Plugin::MetaID::MEDIA_DURATION, duration)) {
        durationMs = Plugin::HstTime2Ms(duration);
        return TransErrorCode(ErrorCode::SUCCESS);
    }
    // use max stream duration as whole source duration if source meta does not contains the duration meta
    uint64_t tmp = 0;
    bool found = false;
    for (const auto& streamMeta : streamMeta_) {
        auto ptr = streamMeta.lock();
        if (ptr != nullptr && ptr->GetUint64(Media::Plugin::MetaID::MEDIA_DURATION, tmp)) {
            found = true;
            duration = std::max(duration, tmp);
        }
    }
    if (found) {
        durationMs = Plugin::HstTime2Ms(duration);
        return TransErrorCode(ErrorCode::SUCCESS);
    }
    return TransErrorCode(ErrorCode::ERROR_AGAIN);
}

int32_t HiPlayerImpl::SetPlaybackSpeed(PlaybackRateMode mode)
{
    MEDIA_LOG_I("SetPlaybackSpeed entered.");
    (void)mode;
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}
int32_t HiPlayerImpl::GetPlaybackSpeed(PlaybackRateMode& mode)
{
    MEDIA_LOG_I("GetPlaybackSpeed entered.");
    (void)mode;
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}

void HiPlayerImpl::OnStateChanged(StateId state)
{
    MEDIA_LOG_I("OnStateChanged from " PUBLIC_LOG_D32 " to " PUBLIC_LOG_D32, curFsmState_.load(), state);
    {
        OSAL::ScopedLock lock(stateMutex_);
        curFsmState_ = state;
        cond_.NotifyOne();
    }
    auto ptr = obs_.lock();
    if (ptr != nullptr) {
        Format format;
        ptr->OnInfo(INFO_TYPE_STATE_CHANGE, TransStateId2PlayerState(state), format);
    }
}

ErrorCode HiPlayerImpl::OnCallback(const FilterCallbackType& type, Filter* filter, const Plugin::Any& parameter)
{
    ErrorCode ret = ErrorCode::SUCCESS;
    switch (type) {
        case FilterCallbackType::PORT_ADDED:
            ret = NewAudioPortFound(filter, parameter);
#ifdef VIDEO_SUPPORT
            ret = NewVideoPortFound(filter, parameter);
#endif
            break;
        case FilterCallbackType::PORT_REMOVE:
            ret = RemoveFilterChains(filter, parameter);
            break;
        default:
            break;
    }
    return ret;
}

ErrorCode HiPlayerImpl::NewAudioPortFound(Filter* filter, const Plugin::Any& parameter)
{
    if (!parameter.SameTypeWith(typeid(PortInfo))) {
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    ErrorCode rtv = ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter == demuxer_.get() && param.type == PortType::OUT) {
        MEDIA_LOG_I("new port found on demuxer " PUBLIC_LOG_ZU, param.ports.size());
        for (const auto& portDesc : param.ports) {
            if (portDesc.name.compare(0, 5, "audio") != 0) { // 5 is length of "audio"
                continue;
            }
            MEDIA_LOG_I("port name " PUBLIC_LOG_S, portDesc.name.c_str());
            auto fromPort = filter->GetOutPort(portDesc.name);
            if (portDesc.isPcm) {
                pipeline_->AddFilters({audioSink_.get()});
                FAIL_LOG(pipeline_->LinkPorts(fromPort, audioSink_->GetInPort(PORT_NAME_DEFAULT)));
                ActiveFilters({audioSink_.get()});
            } else {
                auto newAudioDecoder = CreateAudioDecoder(portDesc.name);
                pipeline_->AddFilters({newAudioDecoder.get(), audioSink_.get()});
                FAIL_LOG(pipeline_->LinkPorts(fromPort, newAudioDecoder->GetInPort(PORT_NAME_DEFAULT)));
                FAIL_LOG(pipeline_->LinkPorts(newAudioDecoder->GetOutPort(PORT_NAME_DEFAULT),
                                              audioSink_->GetInPort(PORT_NAME_DEFAULT)));
                ActiveFilters({newAudioDecoder.get(), audioSink_.get()});
            }
            mediaStats_.Append(MediaStatStub::MediaType::AUDIO);
            rtv = ErrorCode::SUCCESS;
            break;
        }
    }
    return rtv;
}

#ifdef VIDEO_SUPPORT
ErrorCode HiPlayerImpl::NewVideoPortFound(Filter* filter, const Plugin::Any& parameter)
{
    if (!parameter.SameTypeWith(typeid(PortInfo))) {
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter != demuxer_.get() || param.type != PortType::OUT) {
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    std::vector<Filter*> newFilters;
    for (const auto& portDesc : param.ports) {
        if (portDesc.name.compare(0, 5, "video") == 0) { // 5 is length of "video"
            MEDIA_LOG_I("port name " PUBLIC_LOG_S, portDesc.name.c_str());
            videoDecoder_ = FilterFactory::Instance().CreateFilterWithType<VideoDecoderFilter>(
                "builtin.player.videodecoder", "videodecoder-" + portDesc.name);
            if (pipeline_->AddFilters({videoDecoder_.get()}) == ErrorCode::SUCCESS) {
                // link demuxer and video decoder
                auto fromPort = filter->GetOutPort(portDesc.name);
                auto toPort = videoDecoder_->GetInPort(PORT_NAME_DEFAULT);
                FAIL_LOG(pipeline_->LinkPorts(fromPort, toPort));  // link ports
                newFilters.emplace_back(videoDecoder_.get());

                // link video decoder and video sink
                if (pipeline_->AddFilters({videoSink_.get()}) == ErrorCode::SUCCESS) {
                    fromPort = videoDecoder_->GetOutPort(PORT_NAME_DEFAULT);
                    toPort = videoSink_->GetInPort(PORT_NAME_DEFAULT);
                    FAIL_LOG(pipeline_->LinkPorts(fromPort, toPort));  // link ports
                    newFilters.push_back(videoSink_.get());
                }
            }
        }
        break;
    }
    if (!newFilters.empty()) {
        ActiveFilters(newFilters);
    }
    return ErrorCode::SUCCESS;
}
#endif

ErrorCode HiPlayerImpl::RemoveFilterChains(Filter* filter, const Plugin::Any& parameter)
{
    ErrorCode ret = ErrorCode::SUCCESS;
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter != demuxer_.get() || param.type != PortType::OUT) {
        return ret;
    }
    for (const auto& portDesc : param.ports) {
        MEDIA_LOG_I("remove filter chain for port: " PUBLIC_LOG_S, portDesc.name.c_str());
        auto peerPort = filter->GetOutPort(portDesc.name)->GetPeerPort();
        if (peerPort) {
            auto nextFilter = const_cast<Filter*>(dynamic_cast<const Filter*>(peerPort->GetOwnerFilter()));
            if (nextFilter) {
                pipeline_->RemoveFilterChain(nextFilter);
            }
        }
    }
    return ret;
}

void HiPlayerImpl::ActiveFilters(const std::vector<Filter*>& filters)
{
    for (auto it = filters.rbegin(); it != filters.rend(); ++it) {
        (*it)->Prepare();
    }
}
}  // namespace Media
}  // namespace OHOS
