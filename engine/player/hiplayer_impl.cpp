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

#define LOG_TAG "HiPlayerImpl"

#include "hiplayer_impl.h"
#include "foundation/log.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/core/plugin_meta.h"
#include "utils/utils.h"

namespace {
    const float MAX_MEDIA_VOLUME = 300.0f;
}

namespace OHOS {
namespace Media {
using namespace Pipeline;

HiPlayerImpl::HiPlayerImpl() : fsm_(*this), pipelineStates_(PlayerStates::PLAYER_IDLE)
{
    MEDIA_LOG_I("hiPlayerImpl ctor");
    FilterFactory::Instance().Init();

    audioSource_ =
        FilterFactory::Instance().CreateFilterWithType<MediaSourceFilter>("builtin.player.mediasource", "mediaSource");

#ifdef UNIT_TEST
    demuxer = FilterFactory::Instance().CreateFilterWithType<DemuxerFilter>("builtin.player.demuxer", "demuxer");
    audioDecoder_ = FilterFactory::Instance().CreateFilterWithType<AudioDecoderFilter>("builtin.player.audiodecoder",
                                                                                      "audiodecoder");
    audioSink_ =
        FilterFactory::Instance().CreateFilterWithType<AudioSinkFilter>("builtin.player.audiosink", "audiosink");
#else
    demuxer_ = FilterFactory::Instance().CreateFilterWithType<DemuxerFilter>("builtin.player.demuxer", "demuxer");
    audioSink_ =
        FilterFactory::Instance().CreateFilterWithType<AudioSinkFilter>("builtin.player.audiosink", "audioSink");
#ifdef VIDEO_SUPPORT
    videoSink =
        FilterFactory::Instance().CreateFilterWithType<VideoSinkFilter>("builtin.player.videosink", "videoSink");
    FALSE_RETURN(videoSink != nullptr);
#endif
#endif
    FALSE_RETURN(audioSource_ != nullptr);
    FALSE_RETURN(demuxer_ != nullptr);
    FALSE_RETURN(audioSink_ != nullptr);

    pipeline_ = std::make_shared<PipelineCore>();
}

HiPlayerImpl::~HiPlayerImpl()
{
    fsm_.Stop();
    MEDIA_LOG_D("dtor called.");
}

std::shared_ptr<HiPlayerImpl> HiPlayerImpl::CreateHiPlayerImpl()
{
    return std::shared_ptr<HiPlayerImpl>(new (std::nothrow) HiPlayerImpl());
}

int32_t HiPlayerImpl::Init()
{
    if (initialized_.load()) {
        return to_underlying(ErrorCode::SUCCESS);
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
        pipelineStates_ = PLAYER_STATE_ERROR;
    }
    return to_underlying(ret);
}
int32_t HiPlayerImpl::SetSource(const Source &source)
{
    auto ret = Init();
    if (ret != to_underlying(ErrorCode::SUCCESS)) {
        return ret;
    }
    return to_underlying(fsm_.SendEvent(Intent::SET_SOURCE, std::make_shared<Media::Source>(source)));
}

int32_t HiPlayerImpl::Prepare()
{
    MEDIA_LOG_D("Prepare entered, current fsm state: %s.", fsm_.GetCurrentState().c_str());
    OSAL::ScopedLock lock(prepareBlockMutex_);
    cond_.Wait(lock, [this] {
        auto state = fsm_.GetCurrentStateId();
        return state == StateId::READY || state == StateId::INIT;
    });
    MEDIA_LOG_D("Prepare finished, current fsm state: %s.", fsm_.GetCurrentState().c_str());
    if (fsm_.GetCurrentStateId() == StateId::READY) {
        return to_underlying(ErrorCode::SUCCESS);
    } else {
        return to_underlying(ErrorCode::ERROR_UNKNOWN);
    }
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

int32_t HiPlayerImpl::Play()
{
    ErrorCode ret;
    if (pipelineStates_ == PlayerStates::PLAYER_PAUSED) {
        ret = fsm_.SendEvent(Intent::RESUME);
    } else {
        ret = fsm_.SendEvent(Intent::PLAY);
    }
    return to_underlying(ret);
}

bool HiPlayerImpl::IsPlaying()
{
    return pipelineStates_ == PlayerStates::PLAYER_STARTED;
}

int32_t HiPlayerImpl::Pause()
{
    return to_underlying(fsm_.SendEvent(Intent::PAUSE));
}

ErrorCode HiPlayerImpl::Resume()
{
    return fsm_.SendEvent(Intent::RESUME);
}

int32_t HiPlayerImpl::Stop()
{
    return to_underlying(fsm_.SendEvent(Intent::STOP));
}

ErrorCode HiPlayerImpl::StopAsync()
{
    return fsm_.SendEventAsync(Intent::STOP);
}

int32_t HiPlayerImpl::Rewind(int64_t mSeconds, int32_t mode)
{
    return to_underlying(fsm_.SendEventAsync(Intent::SEEK, mSeconds));
}

int32_t HiPlayerImpl::SetVolume(float leftVolume, float rightVolume)
{
    PlayerStates states = pipelineStates_.load();
    if ((states != Media::PlayerStates::PLAYER_STARTED) && (states != Media::PlayerStates::PLAYER_PAUSED) &&
        (states != Media::PlayerStates::PLAYER_PREPARED)) {
        MEDIA_LOG_E("cannot set volume in state %d", states);
        return to_underlying(ErrorCode::ERROR_STATE);
    }
    if (leftVolume < 0 || leftVolume > MAX_MEDIA_VOLUME || rightVolume < 0 || rightVolume > MAX_MEDIA_VOLUME) {
        MEDIA_LOG_E("volume not valid, should be in range [0,300]");
        return to_underlying(ErrorCode::ERROR_INVALID_PARAM_VALUE);
    }
    float volume = 0.f;
    if (leftVolume < 1e-6 && rightVolume >= 1e-6) { // 1e-6
        volume = rightVolume;
    } else if (rightVolume < 1e-6 && leftVolume >= 1e-6) { // 1e-6
        volume = leftVolume;
    } else {
        volume = (leftVolume + rightVolume) / 2; // 2
    }
    volume /= MAX_MEDIA_VOLUME; // normalize to 0~1
    MEDIA_LOG_I("set volume %.3f", volume);
    if (audioSink_ != nullptr) {
        return to_underlying(audioSink_->SetVolume(volume));
    }
    return to_underlying(ErrorCode::ERROR_UNKNOWN);
}
int32_t HiPlayerImpl::SetSurface(Surface* surface)
{
    return to_underlying(ErrorCode::ERROR_UNIMPLEMENTED);
}

ErrorCode HiPlayerImpl::SetBufferSize(size_t size)
{
    return audioSource_->SetBufferSize(size);
}

void HiPlayerImpl::OnEvent(Event event)
{
    MEDIA_LOG_D("[HiStreamer] OnEvent (%d)", event.type);
    switch (event.type) {
        case EVENT_ERROR: {
            fsm_.SendEventAsync(Intent::NOTIFY_ERROR, event.param);
            break;
        }
        case EVENT_READY:
            fsm_.SendEventAsync(Intent::NOTIFY_READY);
            break;
        case EVENT_COMPLETE:
            fsm_.SendEventAsync(Intent::NOTIFY_COMPLETE);
            break;
        default:
            MEDIA_LOG_E("Unknown event(%d)", event.type);
    }
}

ErrorCode HiPlayerImpl::DoSetSource(const std::shared_ptr<MediaSource>& source) const
{
    return audioSource_->SetSource(source);
}

ErrorCode HiPlayerImpl::PrepareFilters()
{
    auto ret = pipeline_->Prepare();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_PREPARED;
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoPlay()
{
    auto ret = pipeline_->Start();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_STARTED;
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoPause()
{
    auto ret = pipeline_->Pause();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_PAUSED;
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoResume()
{
    auto ret = pipeline_->Resume();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_STARTED;
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoStop()
{
    auto ret = pipeline_->Stop();
    if (ret == ErrorCode::SUCCESS) {
        pipelineStates_ = PlayerStates::PLAYER_STOPPED;
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoSeek(int64_t msec)
{
    {
        pipeline_->FlushStart();
        pipeline_->FlushEnd();
    }
    auto rtv = demuxer_->SeekTo(msec);
    auto ptr = callback_.lock();
    if (ptr != nullptr) {
        ptr->OnRewindToComplete();
    }
    return rtv;
}

ErrorCode HiPlayerImpl::DoOnReady()
{
    pipelineStates_ = PlayerStates::PLAYER_PREPARED;
    sourceMeta_ = demuxer_->GetGlobalMetaInfo();
    streamMeta_.clear();
    for (auto& streamMeta : demuxer_->GetStreamMetaInfo()) {
        streamMeta_.push_back(streamMeta);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::DoOnComplete()
{
    MEDIA_LOG_W("OnComplete looping: %d.", singleLoop_.load());
    if (!singleLoop_) {
        StopAsync();
    } else {
        fsm_.SendEventAsync(Intent::SEEK, static_cast<int64_t>(0));
    }
    auto ptr = callback_.lock();
    if (ptr != nullptr) {
        ptr->OnPlaybackComplete();
    }
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::DoOnError(ErrorCode errorCode)
{
    // fixme do we need to callback here to notify registered callback
    auto ptr = callback_.lock();
    if (ptr != nullptr) {
        ptr->OnError(PlayerCallback::PLAYER_ERROR_UNKNOWN, static_cast<int32_t>(errorCode));
    }
    pipelineStates_ = PlayerStates::PLAYER_STATE_ERROR;
    return ErrorCode::SUCCESS;
}

bool HiPlayerImpl::IsSingleLooping()
{
    return singleLoop_.load();
}

int32_t HiPlayerImpl::SetLoop(bool loop)
{
    singleLoop_ = loop;
    return to_underlying(ErrorCode::SUCCESS);
}

void HiPlayerImpl::SetPlayerCallback(const std::shared_ptr<PlayerCallback> &cb)
{
    callback_ = cb;
}

int32_t HiPlayerImpl::Reset()
{
    Stop();
    pipelineStates_ = PlayerStates::PLAYER_IDLE;
    singleLoop_ = false;
    return to_underlying(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::Release()
{
    return Reset();
}

int32_t HiPlayerImpl::DeInit()
{
    return Reset();
}

int32_t HiPlayerImpl::GetPlayerState(int32_t& state)
{
    state = static_cast<int32_t>(pipelineStates_.load());
    return to_underlying(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::GetCurrentPosition(int64_t& currentPositionMs)
{
    return to_underlying(demuxer_->GetCurrentTime(currentPositionMs));
}

int32_t HiPlayerImpl::GetDuration(int64_t& outDurationMs)
{
    uint64_t duration = 0;
    auto sourceMeta = sourceMeta_.lock();
    if (sourceMeta == nullptr) {
        outDurationMs = 0;
        return to_underlying(ErrorCode::ERROR_NOT_FOUND);
    }
    if (sourceMeta->GetUint64(Media::Plugin::MetaID::MEDIA_DURATION, duration)) {
        outDurationMs = duration;
        return to_underlying(ErrorCode::SUCCESS);
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
        outDurationMs = duration;
        return to_underlying(ErrorCode::SUCCESS);
    }
    return to_underlying(ErrorCode::ERROR_NOT_FOUND);
}

ErrorCode HiPlayerImpl::SetVolume(float volume)
{
    if (audioSink_ != nullptr) {
        return audioSink_->SetVolume(volume);
    }
    MEDIA_LOG_W("cannot set volume while audio sink filter is null");
    return ErrorCode::ERROR_NULL_POINTER;
}

void HiPlayerImpl::OnStateChanged(StateId state)
{
    cond_.NotifyOne();
}

ErrorCode HiPlayerImpl::OnCallback(const FilterCallbackType& type, Filter* filter,
                                             const Plugin::Any& parameter)
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

ErrorCode HiPlayerImpl::GetStreamCnt(size_t& cnt) const
{
    cnt = streamMeta_.size();
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::GetSourceMeta(shared_ptr<const Plugin::Meta>& meta) const
{
    meta = sourceMeta_.lock();
    return meta ? ErrorCode::SUCCESS : ErrorCode::ERROR_NOT_FOUND;
}

ErrorCode HiPlayerImpl::GetStreamMeta(size_t index, shared_ptr<const Plugin::Meta>& meta) const
{
    if (index > streamMeta_.size() || index < 0) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    meta = streamMeta_[index].lock();
    if (meta == nullptr) {
        return ErrorCode::ERROR_NOT_FOUND;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::NewAudioPortFound(Filter* filter, const Plugin::Any& parameter)
{
    ErrorCode rtv = ErrorCode::ERROR_PORT_UNEXPECTED;
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter == demuxer_.get() && param.type == PortType::OUT) {
        MEDIA_LOG_I("new port found on demuxer %zu", param.ports.size());
        for (const auto& portDesc : param.ports) {
            if (!StringStartsWith(portDesc.name, "audio")) {
                continue;
            }
            MEDIA_LOG_I("port name %s", portDesc.name.c_str());
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
            rtv = ErrorCode::SUCCESS;
            break;
        }
    }
    return rtv;
}

#ifdef VIDEO_SUPPORT
ErrorCode HiPlayerImpl::NewVideoPortFound(Filter* filter, const Plugin::Any& parameter)
{
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter != demuxer_.get() || param.type != PortType::OUT) {
        return ErrorCode::ERROR_PORT_UNEXPECTED;
    }
    std::vector<Filter*> newFilters;
    for (const auto& portDesc : param.ports) {
        if (StringStartsWith(portDesc.name, "video")) {
            MEDIA_LOG_I("port name %s", portDesc.name.c_str());
            videoDecoder = FilterFactory::Instance().CreateFilterWithType<VideoDecoderFilter>(
                "builtin.player.videodecoder", "videodecoder-" + portDesc.name);
            if (pipeline_->AddFilters({videoDecoder.get()}) != ErrorCode::ERROR_ALREADY_EXISTS) {
                // link demuxer and video decoder
                auto fromPort = filter->GetOutPort(portDesc.name);
                auto toPort = videoDecoder->GetInPort(PORT_NAME_DEFAULT);
                FAIL_LOG(pipeline_->LinkPorts(fromPort, toPort)); // link ports
                newFilters.emplace_back(videoDecoder.get());

                // link video decoder and video sink
                if (pipeline_->AddFilters({videoSink.get()}) != ErrorCode::ERROR_ALREADY_EXISTS) {
                    fromPort = videoDecoder->GetOutPort(PORT_NAME_DEFAULT);
                    toPort = videoSink->GetInPort(PORT_NAME_DEFAULT);
                    FAIL_LOG(pipeline_->LinkPorts(fromPort, toPort)); // link ports
                    newFilters.push_back(videoSink.get());
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
    for (const auto &portDesc: param.ports) {
        MEDIA_LOG_I("remove filter chain for port: %s", portDesc.name.c_str());
        auto peerPort = filter->GetOutPort(portDesc.name)->GetPeerPort();
        if (peerPort) {
            auto nextFilter = const_cast<Filter *>(dynamic_cast<const Filter *>(peerPort->GetOwnerFilter()));
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
} // namespace Media
} // namespace OHOS
