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
#include <utility>
#include "foundation/log.h"
#include "foundation/meta.h"
#include "foundation/utils.h"
#include "pipeline/factory/filter_factory.h"

namespace OHOS {
namespace Media {
using namespace Pipeline;

HiPlayer::HiPlayerImpl::HiPlayerImpl() : fsm_(*this)
{
    MEDIA_LOG_I("hiPlayerImpl ctor");
    FilterFactory::Instance().Init();

    audioSource =
        FilterFactory::Instance().CreateFilterWithType<MediaSourceFilter>("builtin.player.mediasource", "mediaSource");

#ifdef UNIT_TEST
    demuxer = FilterFactory::Instance().CreateFilterWithType<DemuxerFilter>("builtin.player.demuxer", "demuxer");
    audioDecoder = FilterFactory::Instance().CreateFilterWithType<AudioDecoderFilter>("builtin.player.audiodecoder",
                                                                                      "audiodecoder");
    audioSink =
        FilterFactory::Instance().CreateFilterWithType<AudioSinkFilter>("builtin.player.audiosink", "audiosink");
#else
    demuxer = FilterFactory::Instance().CreateFilterWithType<DemuxerFilter>("builtin.player.demuxer", "demuxer");
    audioSink =
        FilterFactory::Instance().CreateFilterWithType<AudioSinkFilter>("builtin.player.audiosink", "audioSink");
#ifdef VIDEO_SUPPORT
    videoSink =
        FilterFactory::Instance().CreateFilterWithType<VideoSinkFilter>("builtin.player.videosink", "videoSink");
    FALSE_RETURN(videoSink != nullptr);
#endif
#endif
    FALSE_RETURN(audioSource != nullptr);
    FALSE_RETURN(demuxer != nullptr);
    FALSE_RETURN(audioSink != nullptr);

    pipeline = std::make_shared<PipelineCore>();
}

HiPlayer::HiPlayerImpl::~HiPlayerImpl()
{
    fsm_.Stop();
    MEDIA_LOG_W("dtor called.");
}

std::shared_ptr<HiPlayer::HiPlayerImpl> HiPlayer::HiPlayerImpl::CreateHiPlayerImpl()
{
    return std::shared_ptr<HiPlayer::HiPlayerImpl>(new (std::nothrow) HiPlayer::HiPlayerImpl());
}

void HiPlayer::HiPlayerImpl::Init()
{
    if (initialized) {
        return;
    }
    initialized = true;
    fsm_.SetStateCallback(this);
    fsm_.Start();
    pipeline->Init(this, this);

#ifdef UNIT_TEST
    pipeline->AddFilters({audioSource.get(), demuxer.get(), audioSink.get()});
    pipeline->LinkFilters({audioSource.get(), demuxer.get(), audioSink.get()});
#else
    pipeline->AddFilters({audioSource.get(), demuxer.get()});
    pipeline->LinkFilters({audioSource.get(), demuxer.get()});
#endif
}

PFilter HiPlayer::HiPlayerImpl::CreateAudioDecoder(const std::string& desc)
{
    if (!audioDecoderMap[desc]) {
        audioDecoderMap[desc] = FilterFactory::Instance().CreateFilterWithType<AudioDecoderFilter>(
            "builtin.player.audiodecoder", "audiodecoder-" + desc);
        // set parameters to decoder.
    }
    return audioDecoderMap[desc];
}

ErrorCode HiPlayer::HiPlayerImpl::Prepare()
{
    // Do nothing, because after SetSource, it enters PreparingState, will call filters.Prepare()
    return SUCCESS;
}

ErrorCode HiPlayer::HiPlayerImpl::Play()
{
    return fsm_.SendEvent(PLAY);
}

ErrorCode HiPlayer::HiPlayerImpl::Pause()
{
    return fsm_.SendEvent(PAUSE);
}

ErrorCode HiPlayer::HiPlayerImpl::Resume()
{
    return fsm_.SendEvent(RESUME);
}

ErrorCode HiPlayer::HiPlayerImpl::Stop()
{
    return fsm_.SendEvent(STOP);
}

ErrorCode HiPlayer::HiPlayerImpl::StopAsync()
{
    return fsm_.SendEventAsync(STOP);
}

ErrorCode HiPlayer::HiPlayerImpl::SetSource(std::shared_ptr<MediaSource> source)
{
    return fsm_.SendEvent(SET_SOURCE, source);
}

ErrorCode HiPlayer::HiPlayerImpl::SetBufferSize(size_t size)
{
    return audioSource->SetBufferSize(size);
}

void HiPlayer::HiPlayerImpl::OnEvent(Event event)
{
    MEDIA_LOG_D("[HiStreamer] OnEvent (%d)", event.type);
    switch (event.type) {
        case EVENT_ERROR: {
            fsm_.SendEventAsync(NOTIFY_ERROR, event.param);
            break;
        }
        case EVENT_READY:
            fsm_.SendEventAsync(NOTIFY_READY);
            break;
        case EVENT_COMPLETE:
            fsm_.SendEventAsync(NOTIFY_COMPLETE);
            break;
        default:
            MEDIA_LOG_E("Unknown event(%d)", event.type);
    }
}

ErrorCode HiPlayer::HiPlayerImpl::SetSingleLoop(bool loop)
{
    singleLoop = loop;
    return SUCCESS;
}

ErrorCode HiPlayer::HiPlayerImpl::Seek(size_t time, size_t& position)
{
    auto rtv = fsm_.SendEvent(SEEK, static_cast<int64_t>(time));
    if (rtv == SUCCESS) {
        int64_t pos = 0;
        rtv = GetCurrentTime(pos);
        position = static_cast<size_t>(pos);
    }
    return rtv;
}

ErrorCode HiPlayer::HiPlayerImpl::DoSetSource(const std::shared_ptr<MediaSource>& source) const
{
    return audioSource->SetSource(source);
}

ErrorCode HiPlayer::HiPlayerImpl::PrepareFilters()
{
    return pipeline->Prepare();
}

ErrorCode HiPlayer::HiPlayerImpl::DoPlay()
{
    return pipeline->Start();
}

ErrorCode HiPlayer::HiPlayerImpl::DoPause()
{
    return pipeline->Pause();
}

ErrorCode HiPlayer::HiPlayerImpl::DoResume()
{
    return pipeline->Resume();
}

ErrorCode HiPlayer::HiPlayerImpl::DoStop()
{
    return pipeline->Stop();
}

ErrorCode HiPlayer::HiPlayerImpl::DoSeek(int64_t msec)
{
    pipeline->FlushStart();
    pipeline->FlushEnd();
    return demuxer->SeekTo(msec);
}

ErrorCode HiPlayer::HiPlayerImpl::DoOnReady()
{
    sourceMeta_ = demuxer->GetGlobalMetaInfo();
    streamMeta_.clear();
    for (auto& streamMeta : demuxer->GetStreamMetaInfo()) {
        streamMeta_.push_back(streamMeta);
    }
    return SUCCESS;
}

ErrorCode HiPlayer::HiPlayerImpl::DoOnComplete()
{
    MEDIA_LOG_W("OnComplete looping: %d.", singleLoop.load());
    if (!singleLoop) {
        StopAsync();
    } else {
        fsm_.SendEventAsync(SEEK, static_cast<int64_t>(0));
    }
    auto ptr = callback_.lock();
    if (ptr != nullptr) {
        ptr->OnCompleted();
    }
    return SUCCESS;
}

ErrorCode HiPlayer::HiPlayerImpl::DoOnError(ErrorCode errorCode)
{
    // fixme do we need to callback here to notify registered callback
    auto ptr = callback_.lock();
    if (ptr != nullptr) {
        ptr->OnError(errorCode);
    }
    return SUCCESS;
}

bool HiPlayer::HiPlayerImpl::IsSingleLooping()
{
    return singleLoop;
}

ErrorCode HiPlayer::HiPlayerImpl::GetCurrentTime(int64_t& time) const
{
    return demuxer->GetCurrentTime(time);
}

ErrorCode HiPlayer::HiPlayerImpl::GetDuration(size_t& time) const
{
    uint64_t duration = 0;
    auto sourceMeta = sourceMeta_.lock();
    if (sourceMeta == nullptr) {
        time = 0;
        return NOT_FOUND;
    }
    if (sourceMeta->GetUint64(Media::Plugin::MetaID::MEDIA_DURATION, duration)) {
        time = duration;
        return SUCCESS;
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
        time = duration;
        return SUCCESS;
    }
    return NOT_FOUND;
}

ErrorCode HiPlayer::HiPlayerImpl::SetVolume(float volume)
{
    if (audioSink != nullptr) {
        return audioSink->SetVolume(volume);
    }
    MEDIA_LOG_W("cannot set volume while audio sink filter is null");
    return NULL_POINTER_ERROR;
}

ErrorCode HiPlayer::HiPlayerImpl::SetCallback(const std::shared_ptr<PlayerCallback>& callback)
{
    return SUCCESS;
}

ErrorCode HiPlayer::HiPlayerImpl::SetCallback(const std::shared_ptr<PlayerCallbackInner>& callback)
{
    // todo this interface should be protected by mutex in case of multi-thread runtime
    callback_ = callback;
    return NULL_POINTER_ERROR;
}

void HiPlayer::HiPlayerImpl::OnStateChanged(std::string state)
{
    if (auto callback = callback_.lock()) {
        callback->OnStateChanged(state);
    }
}

ErrorCode HiPlayer::HiPlayerImpl::OnCallback(const FilterCallbackType& type, Filter* filter,
                                             const Plugin::Any& parameter)
{
    ErrorCode ret = ErrorCode::SUCCESS;
    switch (type) {
        case FilterCallbackType::PORT_ADDED:
#ifndef VIDEO_SUPPORT
            ret = NewAudioPortFound(filter, parameter);
#else
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

ErrorCode HiPlayer::HiPlayerImpl::GetStreamCnt(size_t& cnt) const
{
    cnt = streamMeta_.size();
    return SUCCESS;
}

ErrorCode HiPlayer::HiPlayerImpl::GetSourceMeta(shared_ptr<const Meta>& meta) const
{
    meta = sourceMeta_.lock();
    return meta ? SUCCESS : NOT_FOUND;
}

ErrorCode HiPlayer::HiPlayerImpl::GetStreamMeta(size_t index, shared_ptr<const Meta>& meta) const
{
    if (index > streamMeta_.size() || index < 0) {
        return INVALID_PARAM_VALUE;
    }
    meta = streamMeta_[index].lock();
    if (meta == nullptr) {
        return NOT_FOUND;
    }
    return SUCCESS;
}

ErrorCode HiPlayer::HiPlayerImpl::NewAudioPortFound(Filter* filter, const Plugin::Any& parameter)
{
    ErrorCode rtv = PORT_UNEXPECTED;
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter == demuxer.get() && param.type == PortType::OUT) {
        MEDIA_LOG_I("new port found on demuxer %lu", param.ports.size());
        for (const auto& portDesc : param.ports) {
            if (!StringStartsWith(portDesc.name, "audio")) {
                MEDIA_LOG_W("NewAudioPortFound, discard non-audio port: %s", portDesc.name.c_str());
                continue;
            }
            MEDIA_LOG_I("port name %s", portDesc.name.c_str());
            auto fromPort = filter->GetOutPort(portDesc.name);
            if (portDesc.isPcm) {
                pipeline->AddFilters({audioSink.get()});
                FAIL_LOG(pipeline->LinkPorts(fromPort, audioSink->GetInPort(PORT_NAME_DEFAULT)));
                ActiveFilters({audioSink.get()});
            } else {
                auto newAudioDecoder = CreateAudioDecoder(portDesc.name);
                pipeline->AddFilters({newAudioDecoder.get(), audioSink.get()});
                FAIL_LOG(pipeline->LinkPorts(fromPort, newAudioDecoder->GetInPort(PORT_NAME_DEFAULT)));
                FAIL_LOG(pipeline->LinkPorts(newAudioDecoder->GetOutPort(PORT_NAME_DEFAULT),
                                             audioSink->GetInPort(PORT_NAME_DEFAULT)));
                ActiveFilters({newAudioDecoder.get(), audioSink.get()});
            }
            rtv = SUCCESS;
            break;
        }
    }
    return rtv;
}

#ifdef VIDEO_SUPPORT
ErrorCode HiPlayer::HiPlayerImpl::NewVideoPortFound(Filter* filter, const Plugin::Any& parameter)
{
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter != demuxer.get() || param.type != PortType::OUT) {
        return PORT_UNEXPECTED;
    }
    MEDIA_LOG_I("new port found on demuxer %lu", param.ports.size());
    std::vector<Filter*> newFilters;
    for (const auto& portDesc : param.ports) {
        MEDIA_LOG_I("port name %s", portDesc.name.c_str());
        if (StringStartsWith(portDesc.name, "video")) {
            videoDecoder = FilterFactory::Instance().CreateFilterWithType<VideoDecoderFilter>(
                "builtin.player.videodecoder", "videodecoder-" + portDesc.name);
            if (pipeline->AddFilters({videoDecoder.get()}) != ALREADY_EXISTS) {
                // link demuxer and video decoder
                auto fromPort = filter->GetOutPort(portDesc.name);
                auto toPort = videoDecoder->GetInPort(PORT_NAME_DEFAULT);
                FAIL_LOG(pipeline->LinkPorts(fromPort, toPort)); // link ports
                newFilters.emplace_back(videoDecoder.get());

                // link video decoder and video sink
                if (pipeline->AddFilters({videoSink.get()}) != ALREADY_EXISTS) {
                    fromPort = videoDecoder->GetOutPort(PORT_NAME_DEFAULT);
                    toPort = videoSink->GetInPort(PORT_NAME_DEFAULT);
                    FAIL_LOG(pipeline->LinkPorts(fromPort, toPort)); // link ports
                    newFilters.push_back(videoSink.get());
                }
            }
            break;
        }
    }
    if (!newFilters.empty()) {
        ActiveFilters(newFilters);
    }
    return SUCCESS;
}
#endif

ErrorCode HiPlayer::HiPlayerImpl::RemoveFilterChains(Filter* filter, const Plugin::Any& parameter)
{
    ErrorCode ret = SUCCESS;
    auto param = Plugin::AnyCast<PortInfo>(parameter);
    if (filter != demuxer.get() || param.type != PortType::OUT) {
        return ret;
    }
    for (const auto& portDesc : param.ports) {
        MEDIA_LOG_I("remove filter chain for port: %s", portDesc.name.c_str());
        auto peerPort = filter->GetOutPort(portDesc.name)->GetPeerPort();
        if (peerPort) {
            auto nextFilter = const_cast<Filter*>(dynamic_cast<const Filter*>(peerPort->GetOwnerFilter()));
            if (nextFilter) {
                pipeline->RemoveFilterChain(nextFilter);
            }
        }
    }
    return ret;
}

void HiPlayer::HiPlayerImpl::ActiveFilters(const std::vector<Filter*>& filters)
{
    for (auto it = filters.rbegin(); it != filters.rend(); ++it) {
        (*it)->Prepare();
    }
}
} // namespace Media
} // namespace OHOS
