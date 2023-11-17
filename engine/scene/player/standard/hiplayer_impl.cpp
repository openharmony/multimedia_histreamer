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
#include <audio_info.h>
#include <av_common.h>
#include <media_errors.h>
#include "foundation/cpp_ext/type_traits_ext.h"
#include "foundation/log.h"
#include "foundation/utils/dump_buffer.h"
#include "foundation/utils/hitrace_utils.h"
#include "foundation/utils/steady_clock.h"
#include "media_utils.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/common/media_source.h"
#include "plugin/common/plugin_time.h"

namespace {
const float MAX_MEDIA_VOLUME = 1.0f; // standard interface volume is between 0 to 1.
}

namespace OHOS {
namespace Media {
using namespace Pipeline;
constexpr double EPSINON = 0.0001;
constexpr double SPEED_0_75_X = 0.75;
constexpr double SPEED_1_00_X = 1.00;
constexpr double SPEED_1_25_X = 1.25;
constexpr double SPEED_1_75_X = 1.75;
constexpr double SPEED_2_00_X = 2.00;

HiPlayerImpl::HiPlayerImpl(int32_t appUid, int32_t appPid)
    : appUid_(appUid),
      appPid_(appPid),
      volume_(-1.0f), // default negative, if app not set, will not set it.
      mediaStats_()
{
    SYNC_TRACER();
    MEDIA_LOG_I("hiPlayerImpl ctor");
    FilterFactory::Instance().Init();
    syncManager_ = std::make_shared<MediaSyncManager>();

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
    audioSink_->SetParameter(static_cast<int32_t>(Plugin::Tag::APP_PID), appPid_);
    audioSink_->SetParameter(static_cast<int32_t>(Plugin::Tag::APP_UID), appUid_);
#ifdef VIDEO_SUPPORT
    videoSink_ =
        FilterFactory::Instance().CreateFilterWithType<VideoSinkFilter>("builtin.player.videosink", "videoSink");
    FALSE_RETURN(videoSink_ != nullptr);
    videoSink_->SetSyncCenter(syncManager_);
#endif
#endif
    FALSE_RETURN(audioSource_ != nullptr);
    FALSE_RETURN(demuxer_ != nullptr);
    FALSE_RETURN(audioSink_ != nullptr);
    audioSink_->SetSyncCenter(syncManager_);
    pipeline_ = std::make_shared<PipelineCore>();
    callbackLooper_.SetPlayEngine(this);
}

HiPlayerImpl::~HiPlayerImpl()
{
    MEDIA_LOG_I("dtor called.");
    if (pipelineStates_ != PLAYER_STOPPED) {
        DoStop();
        HiPlayerImpl::OnStateChanged(StateId::STOPPED);
    }
    callbackLooper_.Stop();
    audioSink_.reset();
#ifdef VIDEO_SUPPORT
    videoSink_.reset();
#endif
    syncManager_.reset();
}
void HiPlayerImpl::UpdateStateNoLock(PlayerStates newState, bool notifyUpward)
{
    if (pipelineStates_ == newState) {
        return;
    }
    pipelineStates_ = newState;
    if (pipelineStates_ == PlayerStates::PLAYER_IDLE || pipelineStates_ == PlayerStates::PLAYER_PREPARING) {
        MEDIA_LOG_W("do not report idle and preparing since av player doesn't need report idle and preparing");
        return;
    }
    if (notifyUpward) {
        if (callbackLooper_.IsStarted()) {
            Format format;
            while (!pendingStates_.empty()) {
                auto pendingState = pendingStates_.front();
                pendingStates_.pop();
                MEDIA_LOG_I("sending pending state change: " PUBLIC_LOG_S, StringnessPlayerState(pendingState).c_str());
                callbackLooper_.OnInfo(INFO_TYPE_STATE_CHANGE, pendingState, format);
            }
            MEDIA_LOG_I("sending newest state change: " PUBLIC_LOG_S,
                        StringnessPlayerState(pipelineStates_.load()).c_str());
            callbackLooper_.OnInfo(INFO_TYPE_STATE_CHANGE, pipelineStates_, format);
        } else {
            pendingStates_.push(newState);
        }
    }
}

ErrorCode HiPlayerImpl::Init()
{
    MEDIA_LOG_I("Init entered.");
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
        initialized_ = true;
    } else {
        pipeline_->RemoveFilterChain(audioSource_.get());
        UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    }
    return ret;
}

int32_t HiPlayerImpl::SetSource(const std::string& uri)
{
    SYNC_TRACER();
    MEDIA_LOG_I("SetSource entered source uri: " PUBLIC_LOG_S, uri.c_str());
    PROFILE_BEGIN("SetSource begin");
    auto ret = Init();
    if (ret == ErrorCode::SUCCESS) {
        ret = DoSetSource(std::make_shared<MediaSource>(uri));
        url_ = uri;
    }
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetSource error: " PUBLIC_LOG_S, GetErrorName(ret));
    } else {
        OnStateChanged(StateId::INIT);
    }
    PROFILE_END("SetSource end.");
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::SetSource(const std::shared_ptr<IMediaDataSource>& dataSrc)
{
    MEDIA_LOG_I("SetSource entered source stream");
    PROFILE_BEGIN("SetSource begin");
    auto ret = Init();
    if (ret == ErrorCode::SUCCESS) {
        ret = DoSetSource(std::make_shared<MediaSource>(dataSrc));
    }
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetSource error: " PUBLIC_LOG_S, GetErrorName(ret));
    } else {
        OnStateChanged(StateId::INIT);
    }
    PROFILE_END("SetSource end.");
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::Prepare()
{
    DUMP_BUFFER2FILE_PREPARE();
    if (!(pipelineStates_ == PlayerStates::PLAYER_INITIALIZED || pipelineStates_ == PlayerStates::PLAYER_STOPPED)) {
        return MSERR_INVALID_OPERATION;
    }
    if (pipelineStates_ == PlayerStates::PLAYER_STOPPED) {
        SetSource(url_);
    }
    SYNC_TRACER();
    NotifyBufferingUpdate(PlayerKeys::PLAYER_BUFFERING_START, 0);
    MEDIA_LOG_I("Prepare entered, current pipeline state: " PUBLIC_LOG_S ".",
        StringnessPlayerState(pipelineStates_).c_str());
    PROFILE_BEGIN();
    auto ret = PrepareFilters();
    if (ret != ErrorCode::SUCCESS) {
        PROFILE_END("Prepare failed,");
        MEDIA_LOG_E("Prepare failed with error " PUBLIC_LOG_D32, ret);
        return TransErrorCode(ret);
    }
    OnStateChanged(StateId::PREPARING);
    OSAL::ScopedLock lock(stateMutex_);
    if (pipelineStates_ == PlayerStates::PLAYER_PREPARING) { // Wait state change to ready
        cond_.Wait(lock, [this] { return pipelineStates_ != PlayerStates::PLAYER_PREPARING; });
    }
    MEDIA_LOG_D("Prepare finished, current pipeline state: " PUBLIC_LOG "s.",
        StringnessPlayerState(pipelineStates_).c_str());
    PROFILE_END("Prepare finished, current pipeline state: " PUBLIC_LOG "s.",
        StringnessPlayerState(pipelineStates_).c_str());
    if (pipelineStates_ == PlayerStates::PLAYER_PREPARED) {
        NotifyBufferingUpdate(PlayerKeys::PLAYER_BUFFERING_END, 0);
        Format format;
        callbackLooper_.OnInfo(INFO_TYPE_POSITION_UPDATE, 0, format);
        return TransErrorCode(ErrorCode::SUCCESS);
    }

    return TransErrorCode(ErrorCode::ERROR_UNKNOWN);
}

int HiPlayerImpl::PrepareAsync()
{
    DUMP_BUFFER2FILE_PREPARE();
    if (!(pipelineStates_ == PlayerStates::PLAYER_INITIALIZED || pipelineStates_ == PlayerStates::PLAYER_STOPPED)) {
        return MSERR_INVALID_OPERATION;
    }
    if (pipelineStates_ == PlayerStates::PLAYER_STOPPED) {
        SetSource(url_);
    }
    ASYNC_TRACER();
    NotifyBufferingUpdate(PlayerKeys::PLAYER_BUFFERING_START, 0);
    MEDIA_LOG_I("Prepare async entered, current pipeline state: " PUBLIC_LOG_S,
        StringnessPlayerState(pipelineStates_).c_str());
    PROFILE_BEGIN();
    auto ret = PrepareFilters();
    if (ret != ErrorCode::SUCCESS) {
        PROFILE_END("Prepare async failed,");
        MEDIA_LOG_E("Prepare async failed with error " PUBLIC_LOG_D32, ret);
    } else {
        PROFILE_END("Prepare async successfully,");
    }
    OnStateChanged(StateId::PREPARING);
    NotifyBufferingUpdate(PlayerKeys::PLAYER_BUFFERING_END, 0);
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::Play()
{
    SYNC_TRACER();
    MEDIA_LOG_I("Play entered.");
    PROFILE_BEGIN();
    auto ret {ErrorCode::SUCCESS};
    callbackLooper_.StartReportMediaProgress(100); // 100 MS
    if (pipelineStates_ == PlayerStates::PLAYER_PLAYBACK_COMPLETE) {
        ret = DoSeek(0, Plugin::SeekMode::SEEK_PREVIOUS_SYNC);
    } else if (pipelineStates_ == PlayerStates::PLAYER_PAUSED) {
        ret = DoResume();
    } else {
        ret = DoPlay();
    }
    if (ret == ErrorCode::SUCCESS) {
        OnStateChanged(StateId::PLAYING);
    }
    PROFILE_END("Play ret = " PUBLIC_LOG_D32, TransErrorCode(ret));
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::Pause()
{
    SYNC_TRACER();
    MEDIA_LOG_I("Pause entered.");
    PROFILE_BEGIN();
    auto ret = TransErrorCode(DoPause());
    callbackLooper_.StopReportMediaProgress();
    callbackLooper_.ManualReportMediaProgressOnce();
    OnStateChanged(StateId::PAUSE);
    PROFILE_END("Pause ret = " PUBLIC_LOG_D32, ret);
    return ret;
}

int32_t HiPlayerImpl::Stop()
{
    SYNC_TRACER();
    MEDIA_LOG_I("Stop entered.");
    PROFILE_BEGIN();
    if (pipelineStates_ == PlayerStates::PLAYER_STOPPED) {
        return TransErrorCode(ErrorCode::SUCCESS);
    }
    auto ret = TransErrorCode(DoStop());
    OnStateChanged(StateId::STOPPED);
    callbackLooper_.StopReportMediaProgress();
    callbackLooper_.ManualReportMediaProgressOnce();
    PROFILE_END("Stop ret = " PUBLIC_LOG_D32, ret);
    return ret;
}

ErrorCode HiPlayerImpl::StopAsync()
{
    MEDIA_LOG_I("StopAsync entered.");
    if (pipelineStates_ == PlayerStates::PLAYER_STOPPED) {
        return ErrorCode::SUCCESS;
    }
    ErrorCode ret = DoStop();
    OnStateChanged(StateId::STOPPED);
    return ret;
}

int32_t HiPlayerImpl::Seek(int32_t mSeconds, PlayerSeekMode mode)
{
    SYNC_TRACER();
    MEDIA_LOG_I("Seek entered. mSeconds : " PUBLIC_LOG_D32 ", seekMode : " PUBLIC_LOG_D32,
                mSeconds, static_cast<int32_t>(mode));
    int64_t hstTime = 0;
    int32_t durationMs = 0;
    NZERO_RETURN(GetDuration(durationMs));
    MEDIA_LOG_D("Seek durationMs : " PUBLIC_LOG_D32, durationMs);
    if (mSeconds >= durationMs) { // if exceeds change to duration
        mSeconds = durationMs;
    }
    mSeconds = mSeconds < 0 ? 0 : mSeconds;
    if (audioSource_->GetSeekable() != Plugin::Seekable::SEEKABLE) {
        MEDIA_LOG_E("Seek, invalid operation, audio source is unseekable or invalid");
        return MSERR_INVALID_OPERATION;
    }
    if (!Plugin::Ms2HstTime(mSeconds, hstTime)) {
        return TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE);
    }
    auto smode = Transform2SeekMode(mode);
    auto ret = DoSeek(hstTime, smode);
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::SetVolume(float leftVolume, float rightVolume)
{
    MEDIA_LOG_I("SetVolume entered.");
    if (leftVolume < 0 || leftVolume > MAX_MEDIA_VOLUME || rightVolume < 0 || rightVolume > MAX_MEDIA_VOLUME) {
        MEDIA_LOG_E("volume not valid, should be in range [0,100]");
        return TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE);
    }
    float volume = 0.0f;
    if (leftVolume < 1e-6 && rightVolume >= 1e-6) {  // 1e-6
        volume = rightVolume;
    } else if (rightVolume < 1e-6 && leftVolume >= 1e-6) {  // 1e-6
        volume = leftVolume;
    } else {
        volume = (leftVolume + rightVolume) / 2;  // 2
    }
    volume /= MAX_MEDIA_VOLUME;  // normalize to 0~1
    volume_ = volume;
    if (pipelineStates_ == PlayerStates::PLAYER_IDLE || pipelineStates_ == PlayerStates::PLAYER_INITIALIZED ||
        pipelineStates_ == PlayerStates::PLAYER_PREPARING || pipelineStates_ == PlayerStates::PLAYER_STOPPED ||
        audioSink_ == nullptr) {
        MEDIA_LOG_W("cannot set volume, will do this onReady");
        return TransErrorCode(ErrorCode::SUCCESS);
    }
    return TransErrorCode(SetVolumeToSink(volume));
}

int32_t HiPlayerImpl::SetVideoSurface(sptr<Surface> surface)
{
    MEDIA_LOG_D("SetVideoSurface entered.");
#ifdef VIDEO_SUPPORT
    FALSE_RETURN_V_MSG_E(surface != nullptr, TransErrorCode(ErrorCode::ERROR_INVALID_PARAMETER_VALUE),
                         "Set video surface failed, surface == nullptr");
    return TransErrorCode(videoSink_->SetVideoSurface(surface));
#else
    return TransErrorCode(ErrorCode::SUCCESS);
#endif
}

int32_t HiPlayerImpl::GetVideoTrackInfo(std::vector<Format>& videoTrack)
{
    MEDIA_LOG_I("GetVideoTrackInfo entered.");
    std::string mime;
    std::vector<std::shared_ptr<Plugin::Meta>> metaInfo = demuxer_->GetStreamMetaInfo();
    for (const auto& trackInfo : metaInfo) {
        if (trackInfo->Get<Plugin::Tag::MIME>(mime)) {
            if (IsVideoMime(mime)) {
                int64_t bitRate;
                uint32_t frameRate;
                uint32_t height;
                uint32_t width;
                uint32_t trackIndex;
                Format videoTrackInfo {};
                (void)videoTrackInfo.PutStringValue("codec_mime", mime);
                (void)videoTrackInfo.PutIntValue("track_type", MediaType::MEDIA_TYPE_VID);
                if (trackInfo->Get<Plugin::Tag::TRACK_ID>(trackIndex)) {
                    (void)videoTrackInfo.PutIntValue("track_index", static_cast<int32_t>(trackIndex));
                } else {
                    MEDIA_LOG_W("Get TRACK_ID failed.");
                }
                if (trackInfo->Get<Plugin::Tag::MEDIA_BITRATE>(bitRate)) {
                    (void)videoTrackInfo.PutIntValue("bitrate", static_cast<int32_t>(bitRate));
                } else {
                    MEDIA_LOG_W("Get MEDIA_BITRATE fail");
                }
                if (trackInfo->Get<Plugin::Tag::VIDEO_FRAME_RATE>(frameRate)) {
                    (void)videoTrackInfo.PutIntValue("frame_rate", static_cast<int32_t>(frameRate));
                } else {
                    MEDIA_LOG_W("Get VIDEO_FRAME_RATE fail");
                }
                if (trackInfo->Get<Plugin::Tag::VIDEO_HEIGHT>(height)) {
                    (void)videoTrackInfo.PutIntValue("height", static_cast<int32_t>(height));
                } else {
                    MEDIA_LOG_W("Get VIDEO_HEIGHT fail");
                }
                if (trackInfo->Get<Plugin::Tag::VIDEO_WIDTH>(width)) {
                    (void)videoTrackInfo.PutIntValue("width", static_cast<int32_t>(width));
                } else {
                    MEDIA_LOG_W("Get VIDEO_WIDTH failed.");
                }
                videoTrack.push_back(videoTrackInfo);
            }
        } else {
            MEDIA_LOG_W("Get MIME fail");
        }
    }
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::GetAudioTrackInfo(std::vector<Format>& audioTrack)
{
    MEDIA_LOG_I("GetAudioTrackInfo entered.");
    std::string mime;
    std::vector<std::shared_ptr<Plugin::Meta>> metaInfo = demuxer_->GetStreamMetaInfo();
    for (const auto& trackInfo : metaInfo) {
        if (trackInfo->Get<Plugin::Tag::MIME>(mime)) {
            if (IsAudioMime(mime)) {
                int64_t bitRate;
                uint32_t audioChannels;
                uint32_t audioSampleRate;
                uint32_t trackIndex;
                Format audioTrackInfo {};
                (void)audioTrackInfo.PutStringValue("codec_mime", mime);
                (void)audioTrackInfo.PutIntValue("track_type", MediaType::MEDIA_TYPE_AUD);
                if (trackInfo->Get<Plugin::Tag::TRACK_ID>(trackIndex)) {
                    (void)audioTrackInfo.PutIntValue("track_index", static_cast<int32_t>(trackIndex));
                } else {
                    MEDIA_LOG_I("Get TRACK_ID failed.");
                }
                if (trackInfo->Get<Plugin::Tag::MEDIA_BITRATE>(bitRate)) {
                    (void)audioTrackInfo.PutIntValue("bitrate", static_cast<int32_t>(bitRate));
                } else {
                    MEDIA_LOG_I("Get MEDIA_BITRATE fail");
                }
                if (trackInfo->Get<Plugin::Tag::AUDIO_CHANNELS>(audioChannels)) {
                    (void)audioTrackInfo.PutIntValue("channel_count", static_cast<int32_t>(audioChannels));
                } else {
                    MEDIA_LOG_I("Get AUDIO_CHANNELS fail");
                }
                if (trackInfo->Get<Plugin::Tag::AUDIO_SAMPLE_RATE>(audioSampleRate)) {
                    (void)audioTrackInfo.PutIntValue("sample_rate", static_cast<int32_t>(audioSampleRate));
                } else {
                    MEDIA_LOG_I("Get AUDIO_SAMPLE_RATE fail");
                }
                audioTrack.push_back(audioTrackInfo);
            }
        } else {
            MEDIA_LOG_W("Get MIME fail");
        }
    }
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::GetVideoWidth()
{
    MEDIA_LOG_I("GetVideoWidth entered. video width: " PUBLIC_LOG_D32, videoWidth_);
    return videoWidth_;
}

int32_t HiPlayerImpl::GetVideoHeight()
{
    MEDIA_LOG_I("GetVideoHeight entered. video height: " PUBLIC_LOG_D32, videoHeight_);
    return videoHeight_;
}

void HiPlayerImpl::HandleErrorEvent(const Event& event)
{
    ErrorCode errorCode = ErrorCode::ERROR_UNKNOWN;
    if (Plugin::Any::IsSameTypeWith<ErrorCode>(event.param)) {
        errorCode = Plugin::AnyCast<ErrorCode>(event.param);
    }
    DoOnError(errorCode);
}

void HiPlayerImpl::HandleReadyEvent()
{
    ErrorCode errorCode = DoOnReady();
    if (errorCode == ErrorCode::SUCCESS) {
        OnStateChanged(StateId::READY);
        Format format;
        callbackLooper_.OnInfo(INFO_TYPE_POSITION_UPDATE, 0, format);
    } else {
        OnStateChanged(StateId::INIT);
    }
}

void HiPlayerImpl::HandleCompleteEvent(const Event& event)
{
    mediaStats_.ReceiveEvent(event);
    if (mediaStats_.IsEventCompleteAllReceived()) {
        DoOnComplete();
    }
}

void HiPlayerImpl::HandlePluginErrorEvent(const Event& event)
{
    Plugin::PluginEvent pluginEvent = Plugin::AnyCast<Plugin::PluginEvent>(event.param);
    MEDIA_LOG_I("Receive PLUGIN_ERROR, type:  " PUBLIC_LOG_D32, CppExt::to_underlying(pluginEvent.type));
    if (pluginEvent.type == Plugin::PluginEventType::CLIENT_ERROR &&
            Plugin::Any::IsSameTypeWith<Plugin::NetworkClientErrorCode>(pluginEvent.param)) {
        auto netClientErrorCode = Plugin::AnyCast<Plugin::NetworkClientErrorCode>(pluginEvent.param);
        auto errorType {PlayerErrorType::PLAYER_ERROR_UNKNOWN};
        auto serviceErrCode { MSERR_UNKNOWN };
        if (netClientErrorCode == Plugin::NetworkClientErrorCode::ERROR_TIME_OUT) {
            errorType = PlayerErrorType::PLAYER_ERROR;
            serviceErrCode = MSERR_NETWORK_TIMEOUT;
        }
        callbackLooper_.OnError(errorType, serviceErrCode);
    }
}

void HiPlayerImpl::OnEvent(const Event& event)
{
    if (event.type != EventType::EVENT_AUDIO_PROGRESS) {
        MEDIA_LOG_I("[HiStreamer] OnEvent (" PUBLIC_LOG_S ")", GetEventName(event.type));
    }
    switch (event.type) {
        case EventType::EVENT_ERROR: {
            HandleErrorEvent(event);
            break;
        }
        case EventType::EVENT_READY: {
            HandleReadyEvent();
            break;
        }
        case EventType::EVENT_COMPLETE: {
            HandleCompleteEvent(event);
            break;
        }
        case EventType::EVENT_PLUGIN_ERROR: {
            HandlePluginErrorEvent(event);
            break;
        }
        case EventType::EVENT_RESOLUTION_CHANGE: {
            HandleResolutionChangeEvent(event);
            break;
        }
        case EventType::EVENT_PLUGIN_EVENT: {
            HandlePluginEvent(event);
            break;
        }
        case EventType::EVENT_VIDEO_RENDERING_START: {
            Format format;
            callbackLooper_.OnInfo(INFO_TYPE_MESSAGE, PlayerMessageType::PLAYER_INFO_VIDEO_RENDERING_START, format);
            break;
        }
        case EventType::EVENT_IS_LIVE_STREAM: {
            Format format;
            callbackLooper_.OnInfo(INFO_TYPE_IS_LIVE_STREAM, 0, format);
            break;
        }
        default:
            MEDIA_LOG_E("Unknown event(" PUBLIC_LOG_U32 ")", event.type);
    }
}

ErrorCode HiPlayerImpl::DoSetSource(const std::shared_ptr<MediaSource>& source)
{
    auto ret = audioSource_->SetSource(source);
    if (ret != ErrorCode::SUCCESS) {
        UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    }
    return ret;
}

ErrorCode HiPlayerImpl::PrepareFilters()
{
    auto ret = pipeline_->Prepare();
    if (ret != ErrorCode::SUCCESS) {
        UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoPlay()
{
    syncManager_->Resume();
    auto ret = pipeline_->Start();
    if (ret != ErrorCode::SUCCESS) {
        UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoPause()
{
    auto ret = pipeline_->Pause();
    syncManager_->Pause();
    if (ret != ErrorCode::SUCCESS) {
        UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoResume()
{
    syncManager_->Resume();
    auto ret = pipeline_->Resume();
    if (ret != ErrorCode::SUCCESS) {
        UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoStop()
{
    DUMP_BUFFER2FILE_END();
    mediaStats_.Reset();
    // 先关闭demuxer线程，防止元数据解析prepare过程中出现并发问题
    if (demuxer_) {
        demuxer_->StopTask(false);
    }
    auto ret = pipeline_->Stop();
    syncManager_->Reset();
    if (ret != ErrorCode::SUCCESS) {
        UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    }
    return ret;
}

ErrorCode HiPlayerImpl::DoReset()
{
    return DoStop();
}

ErrorCode HiPlayerImpl::DoSeek(int64_t hstTime, Plugin::SeekMode mode)
{
    SYNC_TRACER();
    PROFILE_BEGIN();
    int64_t seekTime = hstTime;
    Plugin::SeekMode seekMode = mode;
    auto rtv = seekTime >= 0 ? ErrorCode::SUCCESS : ErrorCode::ERROR_INVALID_OPERATION;
    if (rtv == ErrorCode::SUCCESS) {
        pipeline_->FlushStart();
        PROFILE_END("Flush start");
        PROFILE_RESET();

        MEDIA_LOG_I("Do seek ...");
        int64_t realSeekTime = seekTime;
        rtv = audioSource_->SeekToTime(seekTime);
        if (rtv != ErrorCode::SUCCESS) {
            MEDIA_LOG_I("SeekToTime failed\n");
            rtv = demuxer_->SeekTo(seekTime, seekMode, realSeekTime);
        }
        if (rtv == ErrorCode::SUCCESS) {
            syncManager_->Seek(realSeekTime);
        }
        PROFILE_END("SeekTo");

        pipeline_->FlushEnd();
        PROFILE_END("Flush end");
        PROFILE_RESET();
    }
    if (rtv != ErrorCode::SUCCESS) {
        callbackLooper_.OnError(PLAYER_ERROR, MSERR_SEEK_FAILED);
        MEDIA_LOG_E("Seek done, seek error.");
    } else {
        Format format;
        int64_t currentPos = Plugin::HstTime2Ms(seekTime);
        MEDIA_LOG_I("Seek done, currentPos : " PUBLIC_LOG_D64, currentPos);
        callbackLooper_.OnInfo(INFO_TYPE_SEEKDONE, static_cast<int32_t>(currentPos), format);
        callbackLooper_.OnInfo(INFO_TYPE_POSITION_UPDATE, static_cast<int32_t>(currentPos), format);
    }

    return rtv;
}

ErrorCode HiPlayerImpl::DoOnReady()
{
    SetVolumeToSink(volume_, false); // do not report
    auto tmpMeta = demuxer_->GetGlobalMetaInfo();
    sourceMeta_ = tmpMeta;
    int64_t duration = 0;
    bool found = false;
    if (tmpMeta->Get<Media::Plugin::Tag::MEDIA_DURATION>(duration)) {
        found = true;
    } else {
        MEDIA_LOG_W("Get media duration failed.");
    }
    streamMeta_.clear();
    int64_t tmp = 0;
    for (auto& streamMeta : demuxer_->GetStreamMetaInfo()) {
        streamMeta_.push_back(streamMeta);
        if (streamMeta->Get<Media::Plugin::Tag::MEDIA_DURATION>(tmp)) {
            duration = std::max(duration, tmp);
            found = true;
        } else {
            MEDIA_LOG_W("Get media duration failed.");
        }
    }
    if (found) {
        duration_ = duration;
        Format format;
        callbackLooper_.OnInfo(INFO_TYPE_DURATION_UPDATE, Plugin::HstTime2Ms(duration_), format);
    } else {
        MEDIA_LOG_E("INFO_TYPE_DURATION_UPDATE failed");
    }
    std::vector<uint32_t> vBitRates;
    auto ret = audioSource_->GetBitRates(vBitRates);
    if ((ret == ErrorCode::SUCCESS) && (vBitRates.size() > 0)) {   
        int msize = vBitRates.size();
        const int size_ = msize;
        uint32_t* bitrates = vBitRates.data();
        Format bitRateFormat;
        (void)bitRateFormat.PutBuffer(std::string(PlayerKeys::PLAYER_BITRATE),
        static_cast<uint8_t *>(static_cast<void *>(bitrates)), size_ * sizeof(uint32_t));
        callbackLooper_.OnInfo(INFO_TYPE_BITRATE_COLLECT, 0, bitRateFormat);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::DoOnComplete()
{
    MEDIA_LOG_I("OnComplete looping: " PUBLIC_LOG_D32 ".", singleLoop_.load());
    Format format;
    if (singleLoop_.load()) {
        callbackLooper_.OnInfo(INFO_TYPE_EOS, static_cast<int32_t>(singleLoop_.load()), format);
    } else {
        OnStateChanged(StateId::EOS);
        callbackLooper_.StopReportMediaProgress();
        callbackLooper_.ManualReportMediaProgressOnce();
    }
    mediaStats_.ResetEventCompleteAllReceived();
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::DoOnError(ErrorCode errorCode)
{
    UpdateStateNoLock(PlayerStates::PLAYER_STATE_ERROR);
    callbackLooper_.OnError(PLAYER_ERROR, TransErrorCode(errorCode));
    return ErrorCode::SUCCESS;
}

ErrorCode HiPlayerImpl::SetVolumeToSink(float volume, bool reportUpward)
{
    MEDIA_LOG_I("SetVolumeToSink entered.");
    ErrorCode ret = ErrorCode::SUCCESS;
    if (volume_ >= 0) {
        MEDIA_LOG_I("set volume " PUBLIC_LOG_F, volume);
        ret = audioSink_->SetVolume(volume);
    }

    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("SetVolume failed with error " PUBLIC_LOG_D32, static_cast<int>(ret));
        callbackLooper_.OnError(PLAYER_ERROR, TransErrorCode(ret));
    } else if (reportUpward) {
        Format format;
        callbackLooper_.OnInfo(INFO_TYPE_VOLUME_CHANGE, volume, format);
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
    MEDIA_LOG_I("SetLooping entered, loop: " PUBLIC_LOG_D32, loop);
    singleLoop_ = loop;
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::SetParameter(const Format& params)
{
    MEDIA_LOG_I("SetParameter entered.");
    if (params.ContainKey(PlayerKeys::VIDEO_SCALE_TYPE)) {
        int32_t videoScaleType = 0;
        params.GetIntValue(PlayerKeys::VIDEO_SCALE_TYPE, videoScaleType);
        return SetVideoScaleType(VideoScaleType(videoScaleType));
    }
    if (params.ContainKey(PlayerKeys::CONTENT_TYPE) && params.ContainKey(PlayerKeys::STREAM_USAGE)) {
        int32_t contentType;
        int32_t streamUsage;
        int32_t rendererFlag;
        params.GetIntValue(PlayerKeys::CONTENT_TYPE, contentType);
        params.GetIntValue(PlayerKeys::STREAM_USAGE, streamUsage);
        params.GetIntValue(PlayerKeys::RENDERER_FLAG, rendererFlag);
        return SetAudioRendererInfo(contentType, streamUsage, rendererFlag);
    }
    if (params.ContainKey(PlayerKeys::AUDIO_INTERRUPT_MODE)) {
        int32_t interruptMode = 0;
        params.GetIntValue(PlayerKeys::AUDIO_INTERRUPT_MODE, interruptMode);
        return SetAudioInterruptMode(interruptMode);
    }
    return TransErrorCode(ErrorCode::ERROR_UNIMPLEMENTED);
}

int32_t HiPlayerImpl::SetObs(const std::weak_ptr<IPlayerEngineObs>& obs)
{
    MEDIA_LOG_I("SetObs entered.");
    callbackLooper_.StartWithPlayerEngineObs(obs);
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::Reset()
{
    MEDIA_LOG_I("Reset entered.");
    if (pipelineStates_ == PlayerStates::PLAYER_STOPPED) {
        return TransErrorCode(ErrorCode::SUCCESS);
    }
    singleLoop_ = false;
    mediaStats_.Reset();
    auto ret = DoReset();
    OnStateChanged(StateId::STOPPED);
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::GetCurrentTime(int32_t& currentPositionMs)
{
    currentPositionMs = Plugin::HstTime2Ms(syncManager_->GetMediaTimeNow());
    return TransErrorCode(ErrorCode::SUCCESS);
}

int32_t HiPlayerImpl::GetDuration(int32_t& durationMs)
{
    durationMs = 0;
    if (pipelineStates_ == PlayerStates::PLAYER_IDLE || pipelineStates_ == PlayerStates::PLAYER_PREPARING ||
        audioSource_ == nullptr) {
        MEDIA_LOG_E("GetDuration, invalid state or audioSource_ is null. state: " PUBLIC_LOG_S,
                    StringnessPlayerState(pipelineStates_).c_str());
        return MSERR_INVALID_STATE;
    }
    if (duration_ < 0) {
        durationMs = -1;
        MEDIA_LOG_W("no valid duration");
        return MSERR_UNKNOWN;
    }
    durationMs = Plugin::HstTime2Ms(duration_);
    MEDIA_LOG_DD("GetDuration returned " PUBLIC_LOG_D32, durationMs);
    return MSERR_OK;
}

int32_t HiPlayerImpl::SetPlaybackSpeed(PlaybackRateMode mode)
{
    MEDIA_LOG_I("SetPlaybackSpeed entered.");
    double playbackSpeed = ChangeModeToSpeed(mode);
    demuxer_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_PLAYBACK_SPEED), playbackSpeed);
    Format format;
    callbackLooper_.OnInfo(INFO_TYPE_SPEEDDONE, 0, format);

    int32_t currentPosMs = 0;
    int32_t durationMs = 0;
    NZERO_RETURN(GetDuration(durationMs));
    NZERO_RETURN(GetCurrentTime(currentPosMs));
    currentPosMs = std::min(currentPosMs, durationMs);
    currentPosMs = currentPosMs < 0 ? 0 : currentPosMs;
    callbackLooper_.OnInfo(INFO_TYPE_POSITION_UPDATE, currentPosMs, format);
    MEDIA_LOG_D("SetPlaybackSpeed entered end.");
    return MSERR_OK;
}
int32_t HiPlayerImpl::GetPlaybackSpeed(PlaybackRateMode& mode)
{
    MEDIA_LOG_I("GetPlaybackSpeed entered.");
    Plugin::Any any;
    demuxer_->GetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_PLAYBACK_SPEED), any);
    auto playbackSpeed = Plugin::AnyCast<double>(any);
    mode = ChangeSpeedToMode(playbackSpeed);
    return MSERR_OK;
}

void HiPlayerImpl::OnStateChanged(StateId state)
{
    MEDIA_LOG_I("OnStateChanged from " PUBLIC_LOG_D32 " to " PUBLIC_LOG_D32, pipelineStates_.load(),
        TransStateId2PlayerState(state));
    UpdateStateNoLock(TransStateId2PlayerState(state));
    {
        OSAL::ScopedLock lock(stateMutex_);
        cond_.NotifyOne();
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
    if (!Plugin::Any::IsSameTypeWith<PortInfo>(parameter)) {
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
            mediaStats_.Append(audioSink_->GetName());
            rtv = ErrorCode::SUCCESS;
            break;
        }
    }
    return rtv;
}

#ifdef VIDEO_SUPPORT
ErrorCode HiPlayerImpl::NewVideoPortFound(Filter* filter, const Plugin::Any& parameter)
{
    if (!Plugin::Any::IsSameTypeWith<PortInfo>(parameter)) {
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
                    mediaStats_.Append(videoSink_->GetName());
                }
            }
            break;
        }
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
            auto nextFilter = const_cast<Filter*>(reinterpret_cast<const Filter*>(peerPort->GetOwnerFilter()));
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

double HiPlayerImpl::ChangeModeToSpeed(const PlaybackRateMode& mode) const
{
    switch (mode) {
        case SPEED_FORWARD_0_75_X:
            return SPEED_0_75_X;
        case SPEED_FORWARD_1_00_X:
            return SPEED_1_00_X;
        case SPEED_FORWARD_1_25_X:
            return SPEED_1_25_X;
        case SPEED_FORWARD_1_75_X:
            return SPEED_1_75_X;
        case SPEED_FORWARD_2_00_X:
            return SPEED_2_00_X;
        default:
            MEDIA_LOG_I("unknown mode:" PUBLIC_LOG_D32 ", return default speed(SPEED_1_00_X)", mode);
    }
    return SPEED_1_00_X;
}

PlaybackRateMode HiPlayerImpl::ChangeSpeedToMode(double rate) const
{
    if (abs(rate - SPEED_0_75_X) < EPSINON) {
        return SPEED_FORWARD_0_75_X;
    }
    if (abs(rate - SPEED_1_00_X) < EPSINON) {
        return SPEED_FORWARD_1_00_X;
    }
    if (abs(rate - SPEED_1_25_X) < EPSINON) {
        return SPEED_FORWARD_1_25_X;
    }
    if (abs(rate - SPEED_1_75_X) < EPSINON) {
        return SPEED_FORWARD_1_75_X;
    }
    if (abs(rate - SPEED_2_00_X) < EPSINON) {
        return SPEED_FORWARD_2_00_X;
    }
    MEDIA_LOG_I("unknown rate:" PUBLIC_LOG_F ", return default speed(SPEED_FORWARD_1_00_X)", rate);
    return SPEED_FORWARD_1_00_X;
}

int32_t HiPlayerImpl::SetVideoScaleType(VideoScaleType videoScaleType)
{
    MEDIA_LOG_I("SetVideoScaleType entered.");
#ifdef VIDEO_SUPPORT
    auto ret = videoSink_->SetParameter(static_cast<int32_t>(Tag::VIDEO_SCALE_TYPE),
        static_cast<Plugin::VideoScaleType>(static_cast<uint32_t>(videoScaleType)));
    return TransErrorCode(ret);
#else
    return TransErrorCode(ErrorCode::SUCCESS);
#endif
}

int32_t HiPlayerImpl::SetAudioRendererInfo(const int32_t contentType, const int32_t streamUsage,
                                           const int32_t rendererFlag)
{
    MEDIA_LOG_I("SetAudioRendererInfo entered.");
    Plugin::AudioRenderInfo audioRenderInfo {contentType, streamUsage, rendererFlag};
    auto ret = audioSink_->SetParameter(static_cast<int32_t>(Tag::AUDIO_RENDER_INFO), audioRenderInfo);
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::SetAudioInterruptMode(const int32_t interruptMode)
{
    MEDIA_LOG_I("SetAudioInterruptMode entered.");
    auto ret = audioSink_->SetParameter(static_cast<int32_t>(Tag::AUDIO_INTERRUPT_MODE), interruptMode);
    return TransErrorCode(ret);
}

int32_t HiPlayerImpl::SelectBitRate(uint32_t bitRate) {
    int64_t mBitRate = static_cast<int64_t>(bitRate);
    pipeline_->FlushStart();
    auto ret = audioSource_->SelectBitRate(mBitRate);
    pipeline_->FlushEnd();
    return TransErrorCode(ret);
}

void HiPlayerImpl::NotifyBufferingUpdate(const std::string_view& type, int32_t param)
{
    Format format;
    format.PutIntValue(std::string(type), param);
    callbackLooper_.OnInfo(INFO_TYPE_BUFFERING_UPDATE, 0, format);
}

void HiPlayerImpl::HandleResolutionChangeEvent(const Event& event)
{
    auto resolution = Plugin::AnyCast<std::pair<int32_t, int32_t>>(event.param);
    Format format;
    (void)format.PutIntValue(PlayerKeys::PLAYER_WIDTH, resolution.first);
    (void)format.PutIntValue(PlayerKeys::PLAYER_HEIGHT, resolution.second);
    callbackLooper_.OnInfo(INFO_TYPE_RESOLUTION_CHANGE, 0, format);
    MEDIA_LOG_I("Receive plugin RESOLUTION_CHANGE, video_width: " PUBLIC_LOG_U32
                ", video_height: " PUBLIC_LOG_U32, resolution.first, resolution.second);
    videoWidth_ = resolution.first;
    videoHeight_ = resolution.second;
}

void HiPlayerImpl::HandlePluginEvent(const Event& event)
{
    auto pluginEvent = Plugin::AnyCast<Plugin::PluginEvent>(event.param);
    switch (pluginEvent.type) {
        case Plugin::PluginEventType::AUDIO_INTERRUPT: {
            auto interruptEvent = Plugin::AnyCast<AudioStandard::InterruptEvent>(pluginEvent.param);
            MEDIA_LOG_I("Receive Audio AUDIO_INTERRUPT EVENT, eventType: " PUBLIC_LOG_U32
                ", forceType: " PUBLIC_LOG_U32 ", hintType: " PUBLIC_LOG_U32,
                interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType);
            Format format;
            (void)format.PutIntValue(PlayerKeys::AUDIO_INTERRUPT_TYPE, interruptEvent.eventType);
            (void)format.PutIntValue(PlayerKeys::AUDIO_INTERRUPT_FORCE, interruptEvent.forceType);
            (void)format.PutIntValue(PlayerKeys::AUDIO_INTERRUPT_HINT, interruptEvent.hintType);
            callbackLooper_.OnInfo(INFO_TYPE_INTERRUPT_EVENT, 0, format);
            break;
        }
        case Plugin::PluginEventType::AUDIO_STATE_CHANGE: {
            auto renderState = Plugin::AnyCast<AudioStandard::RendererState>(pluginEvent.param);
            MEDIA_LOG_I("Receive Audio STATE_CHANGE EVENT, renderState: " PUBLIC_LOG_U32,
                static_cast<uint32_t>(renderState));
            if (renderState == AudioStandard::RendererState::RENDERER_PAUSED) {
                Format format;
                callbackLooper_.OnInfo(INFO_TYPE_STATE_CHANGE_BY_AUDIO, PlayerStates::PLAYER_PAUSED, format);
            }
            break;
        }
        case Plugin::PluginEventType::BELOW_LOW_WATERLINE:
        case Plugin::PluginEventType::ABOVE_LOW_WATERLINE:
        default:
            MEDIA_LOG_I("Receive PLUGIN_EVENT, type:  " PUBLIC_LOG_D32,
                        CppExt::to_underlying(pluginEvent.type));
            break;
    }
}
}  // namespace Media
}  // namespace OHOS
