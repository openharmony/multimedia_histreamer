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

#define LOG_TAG "DemuxerFilter"

#include "demuxer_filter.h"
#include <algorithm>
#include "compatible_check.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"
#include "utils/constants.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<DemuxerFilter> g_registerFilterHelper("builtin.player.demuxer");

class DemuxerFilter::DataSourceImpl : public Plugin::DataSourceHelper {
public:
    explicit DataSourceImpl(const DemuxerFilter& filter);
    ~DataSourceImpl() override = default;
    Plugin::Status ReadAt(int64_t offset, std::shared_ptr<Plugin::Buffer>& buffer, size_t expectedLen) override;
    Plugin::Status GetSize(size_t& size) override;

private:
    const DemuxerFilter& filter;
};

DemuxerFilter::DataSourceImpl::DataSourceImpl(const DemuxerFilter& filter) : filter(filter)
{
}

/**
 * ReadAt Plugin::DataSource::ReadAt implementation.
 * @param offset offset in media stream.
 * @param buffer caller allocate real buffer.
 * @param expectedLen buffer size wanted to read.
 * @return read result.
 */
Plugin::Status DemuxerFilter::DataSourceImpl::ReadAt(int64_t offset, std::shared_ptr<Plugin::Buffer>& buffer,
                                                     size_t expectedLen)
{
    if (!buffer || buffer->IsEmpty() || expectedLen == 0 || !filter.IsOffsetValid(offset)) {
        MEDIA_LOG_E("ReadAt failed, buffer empty: %d, expectedLen: %d, offset: %lld", !buffer,
                    static_cast<int>(expectedLen), offset);
        return Plugin::Status::ERROR_UNKNOWN;
    }
    Plugin::Status rtv = Plugin::Status::OK;
    switch (filter.pluginState_.load()) {
        case DemuxerState::DEMUXER_STATE_NULL:
            rtv = Plugin::Status::ERROR_WRONG_STATE;
            MEDIA_LOG_E("ReadAt error due to DEMUXER_STATE_NULL");
            break;
        case DemuxerState::DEMUXER_STATE_PARSE_HEADER: {
            if (!filter.peekRange_(static_cast<uint64_t>(offset), expectedLen, buffer)) {
                rtv = Plugin::Status::ERROR_NOT_ENOUGH_DATA;
            }
            break;
        }
        case DemuxerState::DEMUXER_STATE_PARSE_FRAME: {
            if (!filter.getRange_(static_cast<uint64_t>(offset), expectedLen, buffer)) {
                rtv = Plugin::Status::END_OF_STREAM;
            }
            break;
        }
        default:
            break;
    }
    return rtv;
}

Plugin::Status DemuxerFilter::DataSourceImpl::GetSize(size_t& size)
{
    size = filter.mediaDataSize_;
    return (filter.mediaDataSize_ > 0) ? Plugin::Status::OK : Plugin::Status::ERROR_WRONG_STATE;
}

DemuxerFilter::DemuxerFilter(std::string name)
    : FilterBase(std::move(name)),
      uriSuffix_(),
      mediaDataSize_(0),
      task_(nullptr),
      typeFinder_(nullptr),
      dataPacker_(nullptr),
      pluginName_(),
      plugin_(nullptr),
      pluginState_(DemuxerState::DEMUXER_STATE_NULL),
      pluginAllocator_(nullptr),
      dataSource_(std::make_shared<DataSourceImpl>(*this)),
      mediaMetaData_(),
      curTimeUs_(0)
{
    filterType_ = FilterType::DEMUXER;
    MEDIA_LOG_D("ctor called");
}

DemuxerFilter::~DemuxerFilter()
{
    MEDIA_LOG_D("dtor called");
    if (task_) {
        task_->Stop();
    }
    if (plugin_) {
        plugin_->Deinit();
    }
}

void DemuxerFilter::Init(EventReceiver* receiver, FilterCallback* callback)
{
    this->eventReceiver_ = receiver;
    this->callback_ = callback;
    inPorts_.clear();
    outPorts_.clear();
    inPorts_.push_back(std::make_shared<Pipeline::InPort>(this, PORT_NAME_DEFAULT, true));
    state_ = FilterState::INITIALIZED;
}

ErrorCode DemuxerFilter::Start()
{
    if (task_) {
        task_->Start();
    }
    return FilterBase::Start();
}

ErrorCode DemuxerFilter::Stop()
{
    MEDIA_LOG_I("Stop called.");
    if (task_) {
        task_->Pause();
    }
    Reset();
    if (!outPorts_.empty()) {
        PortInfo portInfo;
        portInfo.type = PortType::OUT;
        portInfo.ports.reserve(outPorts_.size());
        for (const auto& outPort : outPorts_) {
            portInfo.ports.push_back({outPort->GetName(), false});
        }
        if (callback_) {
            callback_->OnCallback(FilterCallbackType::PORT_REMOVE, static_cast<Filter*>(this), portInfo);
        }
    }
    return FilterBase::Stop();
}

ErrorCode DemuxerFilter::Pause()
{
    MEDIA_LOG_D("Pause called");
    return FilterBase::Pause();
}

void DemuxerFilter::FlushStart()
{
    MEDIA_LOG_D("FlushStart entered");
    if (dataPacker_) {
        dataPacker_->Flush();
    }
    if (task_) {
        task_->Pause();
    }
}

void DemuxerFilter::FlushEnd()
{
    MEDIA_LOG_D("FlushEnd entered");
}

ErrorCode DemuxerFilter::Prepare()
{
    MEDIA_LOG_D("Prepare called");
    pluginState_ = DemuxerState::DEMUXER_STATE_NULL;
    Pipeline::WorkMode mode;
    GetInPort(PORT_NAME_DEFAULT)->Activate({Pipeline::WorkMode::PULL, Pipeline::WorkMode::PUSH}, mode);
    if (mode == Pipeline::WorkMode::PULL) {
        ActivatePullMode();
    } else {
        ActivatePushMode();
    }
    SetCurrentTime(0);
    state_ = FilterState::PREPARING;
    return ErrorCode::SUCCESS;
}

ErrorCode DemuxerFilter::PushData(const std::string& inPort, AVBufferPtr buffer)
{
    MEDIA_LOG_D("PushData for port: %s", inPort.c_str());
    if (dataPacker_) {
        dataPacker_->PushData(std::move(buffer));
    }
    return ErrorCode::SUCCESS;
}

bool DemuxerFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                              Capability& upstreamNegotiatedCap)
{
    (void)inPort;
    (void)upstreamCap;
    (void)upstreamNegotiatedCap;
    return true;
}

bool DemuxerFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta)
{
    (void)upstreamMeta->GetUint64(Plugin::MetaID::MEDIA_FILE_SIZE, mediaDataSize_);
    return upstreamMeta->GetString(Plugin::MetaID::MEDIA_FILE_EXTENSION, uriSuffix_);
}

ErrorCode DemuxerFilter::SeekTo(int64_t msec)
{
    if (!plugin_) {
        MEDIA_LOG_E("SeekTo failed due to no valid plugin");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    ErrorCode rtv = ErrorCode::SUCCESS;
    auto ret = plugin_->SeekTo(-1, msec * 1000, Plugin::SeekMode::BACKWARD); // 1000
    if (ret == Plugin::Status::OK) {
        if (task_) {
            task_->Start();
        }
    } else {
        MEDIA_LOG_E("SeekTo failed with return value: %d", static_cast<int>(ret));
        rtv = ErrorCode::ERROR_SEEK_FAILURE;
    }
    return rtv;
}

std::vector<std::shared_ptr<Plugin::Meta>> DemuxerFilter::GetStreamMetaInfo() const
{
    return mediaMetaData_.trackMetas;
}

std::shared_ptr<Plugin::Meta> DemuxerFilter::GetGlobalMetaInfo() const
{
    return mediaMetaData_.globalMeta;
}

ErrorCode DemuxerFilter::GetCurrentTime(int64_t& time) const
{
    OSAL::ScopedLock lock(timeMutex_);
    time = curTimeUs_ / 1000; // 1000, time in milliseconds
    return ErrorCode::SUCCESS;
}

void DemuxerFilter::Reset()
{
    SetCurrentTime(0);
    mediaMetaData_.globalMeta.reset();
    mediaMetaData_.trackMetas.clear();
    mediaMetaData_.trackInfos.clear();
}

void DemuxerFilter::InitTypeFinder()
{
    if (!typeFinder_) {
        typeFinder_ = std::make_shared<TypeFinder>();
    }
}

bool DemuxerFilter::InitPlugin(std::string pluginName)
{
    if (pluginName.empty()) {
        return false;
    }
    if (!(pluginName_ == pluginName)) {
        if (plugin_) {
            plugin_->Deinit();
        }
        plugin_ = Plugin::PluginManager::Instance().CreateDemuxerPlugin(pluginName);
        if (!plugin_ || plugin_->Init() != Plugin::Status::OK) {
            MEDIA_LOG_E("InitPlugin for %s failed.", pluginName.c_str());
            return false;
        }
        pluginAllocator_ = plugin_->GetAllocator();
        pluginName_.swap(pluginName);
    } else {
        if (plugin_->Reset() != Plugin::Status::OK) {
            MEDIA_LOG_E("plugin %s failed to reset.", pluginName.c_str());
            return false;
        }
    }
    MEDIA_LOG_W("InitPlugin, %s used.", pluginName_.c_str());
    plugin_->SetDataSource(std::dynamic_pointer_cast<Plugin::DataSourceHelper>(dataSource_));
    pluginState_ = DemuxerState::DEMUXER_STATE_PARSE_HEADER;
    plugin_->Prepare();
    return true;
}

void DemuxerFilter::ActivatePullMode()
{
    MEDIA_LOG_D("ActivatePullMode called");
    InitTypeFinder();
    if (!task_) {
        task_ = std::make_shared<OSAL::Task>("DemuxerFilter");
    }
    task_->RegisterHandler([this] { DemuxerLoop(); });
    checkRange_ = [this](uint64_t offset, uint32_t size) {
        return (offset < mediaDataSize_) && (size <= mediaDataSize_) && (offset <= (mediaDataSize_ - size));
    };
    peekRange_ = [this](uint64_t offset, size_t size, AVBufferPtr& bufferPtr) -> bool {
        return inPorts_.front()->PullData(offset, size, bufferPtr) == ErrorCode::SUCCESS;
    };
    getRange_ = peekRange_;
    typeFinder_->Init(uriSuffix_, mediaDataSize_, checkRange_, peekRange_);
    MediaTypeFound(typeFinder_->FindMediaType());
}

void DemuxerFilter::ActivatePushMode()
{
    MEDIA_LOG_D("ActivatePushMode called");
    InitTypeFinder();
    if (!dataPacker_) {
        dataPacker_ = std::make_shared<DataPacker>();
    }
    checkRange_ = [this](uint64_t offset, uint32_t size) { return dataPacker_->IsDataAvailable(offset, size); };
    peekRange_ = [this](uint64_t offset, size_t size, AVBufferPtr& bufferPtr) -> bool {
        return dataPacker_->PeekRange(offset, size, bufferPtr);
    };
    getRange_ = [this](uint64_t offset, size_t size, AVBufferPtr& bufferPtr) -> bool {
        return dataPacker_->GetRange(offset, size, bufferPtr);
    };
    typeFinder_->Init(uriSuffix_, mediaDataSize_, checkRange_, peekRange_);
    typeFinder_->FindMediaTypeAsync([this](std::string pluginName) { MediaTypeFound(std::move(pluginName)); });
}

void DemuxerFilter::MediaTypeFound(std::string pluginName)
{
    if (InitPlugin(std::move(pluginName))) {
        task_->Start();
    } else {
        OnEvent({EVENT_ERROR, ErrorCode::ERROR_PLUGIN_NOT_FOUND});
    }
}

void DemuxerFilter::InitMediaMetaData(const Plugin::MediaInfoHelper& mediaInfo)
{
    mediaMetaData_.globalMeta = std::make_shared<Plugin::Meta>(mediaInfo.globalMeta);
    mediaMetaData_.trackMetas.clear();
    int trackCnt = 0;
    for (auto& trackMeta : mediaInfo.streamMeta) {
        mediaMetaData_.trackMetas.push_back(std::make_shared<Plugin::Meta>(trackMeta));
        if (!trackMeta.Empty()) {
            ++trackCnt;
        }
    }
    mediaMetaData_.trackInfos.reserve(trackCnt);
}

bool DemuxerFilter::IsOffsetValid(int64_t offset) const
{
    return mediaDataSize_ == 0 || offset <= static_cast<int64_t>(mediaDataSize_);
}

bool DemuxerFilter::PrepareStreams(const Plugin::MediaInfoHelper& mediaInfo)
{
    MEDIA_LOG_D("PrepareStreams called");
    InitMediaMetaData(mediaInfo);
    outPorts_.clear();
    int streamCnt = mediaInfo.streamMeta.size();
    PortInfo portInfo;
    portInfo.type = PortType::OUT;
    portInfo.ports.reserve(streamCnt);
    int audioTrackCnt = 0;
    for (int i = 0; i < streamCnt; ++i) {
        if (mediaInfo.streamMeta[i].Empty()) {
            MEDIA_LOG_E("PrepareStreams, unsupported stream with streamIdx = %d", i);
            continue;
        }
        std::string mime;
        uint32_t streamIdx = 0;
        if (!mediaInfo.streamMeta[i].GetString(Plugin::MetaID::MIME, mime) ||
            !mediaInfo.streamMeta[i].GetUint32(Plugin::MetaID::STREAM_INDEX, streamIdx)) {
            MEDIA_LOG_E("PrepareStreams failed to extract mime or streamIdx.");
            continue;
        }
        if (IsAudioMime(mime)) {
            MEDIA_LOG_D("PrepareStreams, audio stream with streamIdx = %u.", streamIdx);
            if (audioTrackCnt == 1) {
                MEDIA_LOG_E("PrepareStreams, discard audio stream: %d.", streamIdx);
                continue;
            }
            ++audioTrackCnt;
        }
        auto port = std::make_shared<OutPort>(this, NamePort(mime));
        MEDIA_LOG_I("PrepareStreams, streamIdx: %d, portName: %s", i, port->GetName().c_str());
        outPorts_.push_back(port);
        portInfo.ports.push_back({port->GetName(), IsRawAudio(mime)});
        mediaMetaData_.trackInfos.emplace_back(streamIdx, std::move(port), true);
    }
    if (portInfo.ports.empty()) {
        MEDIA_LOG_E("PrepareStreams failed due to no valid port.");
        return false;
    }
    ErrorCode ret = ErrorCode::SUCCESS;
    if (callback_) {
        ret = callback_->OnCallback(FilterCallbackType::PORT_ADDED, static_cast<Filter*>(this), portInfo);
    }
    return ret == ErrorCode::SUCCESS;
}

ErrorCode DemuxerFilter::ReadFrame(AVBuffer& buffer, uint32_t& streamIndex)
{
    MEDIA_LOG_D("ReadFrame called");
    ErrorCode result = ErrorCode::ERROR_UNKNOWN;
    auto rtv = plugin_->ReadFrame(buffer, 0);
    if (rtv == Plugin::Status::OK) {
        streamIndex = buffer.streamID;
        SetCurrentTime(buffer.pts);
        result = ErrorCode::SUCCESS;
    }
    MEDIA_LOG_D("ReadFrame return with rtv = %d", static_cast<int32_t>(rtv));
    return (rtv != Plugin::Status::END_OF_STREAM) ? result : ErrorCode::END_OF_STREAM;
}

std::shared_ptr<Plugin::Meta> DemuxerFilter::GetStreamMeta(uint32_t streamIndex)
{
    return (streamIndex < mediaMetaData_.trackMetas.size()) ? mediaMetaData_.trackMetas[streamIndex] : nullptr;
}

void DemuxerFilter::SendEventEos()
{
    MEDIA_LOG_D("SendEventEos called");
    AVBufferPtr bufferPtr = std::make_shared<AVBuffer>();
    bufferPtr->flag = BUFFER_FLAG_EOS;
    for (const auto& stream : mediaMetaData_.trackInfos) {
        stream.port->PushData(bufferPtr);
    }
}

void DemuxerFilter::HandleFrame(const AVBufferPtr& bufferPtr, uint32_t streamIndex)
{
    for (auto& stream : mediaMetaData_.trackInfos) {
        if (stream.streamIdx != streamIndex) {
            continue;
        }
        stream.port->PushData(bufferPtr);
        break;
    }
}

void DemuxerFilter::NegotiateDownstream()
{
    for (auto& stream : mediaMetaData_.trackInfos) {
        if (stream.needNegoCaps) {
            Capability caps;
            MEDIA_LOG_I("demuxer negotiate with streamIdx: %u", stream.streamIdx);
            auto streamMeta = GetStreamMeta(stream.streamIdx);
            auto tmpCap = MetaToCapability(*streamMeta);
            if (stream.port->Negotiate(tmpCap, caps) && stream.port->Configure(streamMeta)) {
                stream.needNegoCaps = false;
            } else {
                task_->PauseAsync();
                OnEvent({EVENT_ERROR, ErrorCode::ERROR_PLUGIN_NOT_FOUND});
            }
        }
    }
}

void DemuxerFilter::DemuxerLoop()
{
    if (pluginState_.load() == DemuxerState::DEMUXER_STATE_PARSE_FRAME) {
        AVBufferPtr bufferPtr = std::make_shared<AVBuffer>();
        uint32_t streamIndex = 0;
        auto rtv = ReadFrame(*bufferPtr, streamIndex);
        if (rtv == ErrorCode::SUCCESS) {
            HandleFrame(bufferPtr, streamIndex);
        } else {
            SendEventEos();
            task_->PauseAsync();
            if (rtv != ErrorCode::END_OF_STREAM) {
                MEDIA_LOG_E("ReadFrame failed with rtv = %d", rtv);
            }
        }
    } else {
        Plugin::MediaInfoHelper mediaInfo;
        if (plugin_->GetMediaInfo(mediaInfo) == Plugin::Status::OK && PrepareStreams(mediaInfo)) {
            NegotiateDownstream();
            pluginState_ = DemuxerState::DEMUXER_STATE_PARSE_FRAME;
            state_ = FilterState::READY;
            OnEvent({EVENT_READY, {}});
        } else {
            task_->PauseAsync();
            OnEvent({EVENT_ERROR, ErrorCode::ERROR_PARSE_META_FAILED});
        }
    }
}

void DemuxerFilter::SetCurrentTime(int64_t timestampUsec)
{
    OSAL::ScopedLock lock(timeMutex_);
    curTimeUs_ = timestampUsec;
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
