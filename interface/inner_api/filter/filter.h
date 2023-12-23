/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PIPELINE_CORE_FILTER_BASE_H
#define HISTREAMER_PIPELINE_CORE_FILTER_BASE_H
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include "meta/meta.h"
#include "buffer/avbuffer_queue_producer.h"
#include "common/event.h"

namespace OHOS {
namespace Media {
namespace Pipeline {

class Filter;

enum class FilterType {
    FILTERTYPE_SOURCE,
    FILTERTYPE_DEMUXER,
    FILTERTYPE_AENC,
    FILTERTYPE_ADEC,
    FILTERTYPE_VENC,
    FILTERTYPE_VDEC,
    FILTERTYPE_MUXER,
    FILTERTYPE_ASINK,
    FILTERTYPE_FSINK,
    AUDIO_CAPTURE,
    VIDEO_CAPTURE,
    FILTERTYPE_MAX,
};

enum class StreamType {
    STREAMTYPE_PACKED,
    STREAMTYPE_ENCODED_AUDIO,
    STREAMTYPE_ENCODED_VIDEO,
    STREAMTYPE_RAW_AUDIO,
    STREAMTYPE_RAW_VIDEO,
    STREAMTYPE_SUBTITLE,
    STREAMTYPE_MAX,
};

enum class FilterState {
    CREATED,     // Filter created
    INITIALIZED, // Init called
    PREPARING,   // Prepare called
    READY,       // Ready Event reported
    RUNNING,     // Start called
    PAUSED,      // Pause called
};

enum class FilterCallBackCommand {
    NEXT_FILTER_NEEDED,
    NEXT_FILTER_REMOVED,
    NEXT_FILTER_UPDATE,
    FILTER_CALLBACK_COMMAND_MAX,
};

class EventReceiver {
public:
    virtual ~EventReceiver() = default;
    virtual void OnEvent(const Event& event) = 0;
};

class FilterCallback {
public:
    virtual ~FilterCallback() = default;
    virtual void OnCallback(const std::shared_ptr<Filter>& filter, FilterCallBackCommand cmd, StreamType outType) = 0;
};

class FilterLinkCallback {
public:
    virtual ~FilterLinkCallback() = default;
    virtual void OnLinkedResult(const sptr<AVBufferQueueProducer>& queue, std::shared_ptr<Meta>& meta) = 0;
    virtual void OnUnlinkedResult(std::shared_ptr<Meta>& meta) = 0;
    virtual void OnUpdatedResult(std::shared_ptr<Meta>& meta) = 0;
};

class Filter {
public:
    explicit Filter(std::string name, FilterType type);
    virtual ~Filter() = default;
    virtual void Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback);

    virtual Status Prepare();

    virtual Status Start();

    virtual Status Pause();

    virtual Status Resume();

    virtual Status Stop();

    virtual Status Flush();

    virtual Status Release();

    virtual void SetParameter(const std::shared_ptr<Meta>& meta);

    virtual void GetParameter(std::shared_ptr<Meta>& meta);

    virtual Status LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType);

    virtual Status UpdateNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType);

    virtual Status UnLinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType);

    FilterType GetFilterType();

    virtual Status OnLinked(StreamType inType, const std::shared_ptr<Meta>& meta,
                            const std::shared_ptr<FilterLinkCallback>& callback);

    virtual Status OnUpdated(StreamType inType, const std::shared_ptr<Meta>& meta,
                             const std::shared_ptr<FilterLinkCallback>& callback);

    virtual Status OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback>& callback);

protected:
    std::string name_;

    std::shared_ptr<Meta> meta_;

    FilterType filterType_;

    std::vector<StreamType> supportedInStreams_;
    std::vector<StreamType> supportedOutStreams_;

    std::map<StreamType, std::vector<std::shared_ptr<Filter>>> nextFiltersMap_;

    std::shared_ptr<EventReceiver> receiver_;

    std::shared_ptr<FilterCallback> callback_;

    std::map<StreamType, std::vector<std::shared_ptr<FilterLinkCallback>>> linkCallbackMaps_;
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif
