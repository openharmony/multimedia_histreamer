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

#ifndef HISTREAMER_HIPLAYER_IMPL_H
#define HISTREAMER_HIPLAYER_IMPL_H

#include <memory>
#include <unordered_map>

#include "common/any.h"
#ifdef VIDEO_SUPPORT
#include "filters/sink/video_sink/video_sink_filter.h"
#include "filters/codec/video_decoder/video_decoder_filter.h"
#endif
#include "filters/demux/demuxer_filter.h"
#include "filters/source/media_source_filter.h"
#include "foundation/error_code.h"
#include "foundation/utils.h"
#include "histreamer/hiplayer.h"
#include "internal/state_machine.h"
#include "pipeline/core/filter_callback.h"
#include "pipeline/core/pipeline.h"
#include "pipeline/core/pipeline_core.h"
#include "pipeline/filters/codec/audio_decoder/audio_decoder_filter.h"
#include "pipeline/filters/sink/audio_sink/audio_sink_filter.h"
#include "play_executor.h"

namespace OHOS {
namespace Media {
class PlayerCallbackInner {
public:
    virtual void OnCompleted() = 0;
    virtual void OnError(ErrorCode errorCode) = 0;
    virtual void OnSeekCompleted(int32_t position) = 0;
    virtual void OnStateChanged(std::string state) = 0;
};

class HiPlayer::HiPlayerImpl : public Pipeline::EventReceiver,
                               public PlayExecutor,
                               public StateChangeCallback,
                               public Pipeline::FilterCallback {
    friend class StateMachine;

public:
    ~HiPlayerImpl() override;

    static std::shared_ptr<HiPlayer::HiPlayerImpl> CreateHiPlayerImpl();

    void Init();
    void OnEvent(Event event) override;

    ErrorCode Prepare();
    ErrorCode Play();
    ErrorCode Pause();
    ErrorCode Resume();
    ErrorCode Stop();

    // interface from MediaSource
    ErrorCode SetSource(std::shared_ptr<MediaSource> source);
    ErrorCode SetBufferSize(size_t size);
    ErrorCode Seek(size_t time, size_t& position);
    ErrorCode SetSingleLoop(bool loop);
    bool IsSingleLooping();

    /**
     * get duration in milliseconds
     *
     * @param time milliseconds
     * @return
     */
    ErrorCode GetDuration(size_t& time) const;
    ErrorCode GetCurrentTime(int64_t& time) const;
    ErrorCode GetSourceMeta(std::shared_ptr<const Meta>& meta) const;
    ErrorCode GetStreamCnt(size_t& cnt) const;
    ErrorCode GetStreamMeta(size_t index, std::shared_ptr<const Meta>& meta) const;

    ErrorCode SetVolume(float volume);

    ErrorCode SetCallback(const std::shared_ptr<PlayerCallback>& callback);

    ErrorCode SetCallback(const std::shared_ptr<PlayerCallbackInner>& callback);

    void OnStateChanged(std::string state) override;

    ErrorCode OnCallback(const Pipeline::FilterCallbackType& type, Pipeline::Filter* filter,
                         const Plugin::Any& parameter) override;

    // interface from PlayExecutor
    ErrorCode DoSetSource(const std::shared_ptr<MediaSource>& source) const override;
    ErrorCode PrepareFilters() override;
    ErrorCode DoPlay() override;
    ErrorCode DoPause() override;
    ErrorCode DoResume() override;
    ErrorCode DoStop() override;
    ErrorCode DoSeek(int64_t msec) override;
    ErrorCode DoOnReady() override;
    ErrorCode DoOnComplete() override;
    ErrorCode DoOnError(ErrorCode) override;

private:
    HiPlayerImpl();
    HiPlayerImpl(const HiPlayerImpl& other);
    HiPlayerImpl& operator=(const HiPlayerImpl& other);
    ErrorCode StopAsync();

    Pipeline::PFilter CreateAudioDecoder(const std::string& desc);

    ErrorCode NewAudioPortFound(Pipeline::Filter* filter, const Plugin::Any& parameter);
#ifdef VIDEO_SUPPORT
    ErrorCode NewVideoPortFound(Pipeline::Filter* filter, const Plugin::Any& parameter);
#endif

    ErrorCode RemoveFilterChains(Pipeline::Filter* filter, const Plugin::Any& parameter);

    void ActiveFilters(const std::vector<Pipeline::Filter*>& filters);

private:
    StateMachine fsm_;
    std::shared_ptr<Pipeline::PipelineCore> pipeline;

    std::shared_ptr<Pipeline::MediaSourceFilter> audioSource;
    std::shared_ptr<Pipeline::DemuxerFilter> demuxer;
    std::shared_ptr<Pipeline::AudioDecoderFilter> audioDecoder;
    std::shared_ptr<Pipeline::AudioSinkFilter> audioSink;
#ifdef VIDEO_SUPPORT
    std::shared_ptr<Pipeline::VideoDecoderFilter> videoDecoder;
    std::shared_ptr<Pipeline::VideoSinkFilter> videoSink;
#endif

    std::unordered_map<std::string, std::shared_ptr<Pipeline::AudioDecoderFilter>> audioDecoderMap;

    std::weak_ptr<Meta> sourceMeta_;
    std::vector<std::weak_ptr<Meta>> streamMeta_;

    std::atomic<bool> singleLoop {false};
    bool initialized = false;

    std::weak_ptr<PlayerCallbackInner> callback_;
};
} // namespace Media
} // namespace OHOS
#endif
