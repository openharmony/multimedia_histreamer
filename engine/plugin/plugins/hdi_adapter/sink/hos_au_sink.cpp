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

#define LOG_TAG "HdiSink"

#include "hos_au_sink.h"

#include <dlfcn.h>
#include <memory>

#include "securec.h"

#include "audio_proxy_manager.h"
#include "audio_adapter.h"

#include "foundation/constants.h"
#include "foundation/log.h"
#include "foundation/osal/thread/scoped_lock.h"
#include "foundation/osal/utils/util.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugins/hdi_adapter/utils/hdi_au_utils.h"
#include "ring_buffer.h"

namespace {
using namespace OHOS::Media::Plugin;
constexpr int32_t MAX_RETRY_CNT = 3;
constexpr int32_t RETRY_INTERVAL = 100; // 100ms
constexpr int32_t DEFAULT_BUFFER_POOL_SIZE = 5;
constexpr int32_t HI_ERR_VI_BUF_FULL = 0xA016800F;
constexpr int32_t RANK100 = 100;
constexpr int32_t HALF = 2;
constexpr int32_t SEC_TO_MILLS = 1000;
constexpr float MAX_VOLUME = 300.f;


Status LoadAndInitAdapter(AudioManager *proxyManager, AudioAdapterDescriptor *descriptor, AudioAdapter **adapter)
{
    if (proxyManager == nullptr) {
        MEDIA_LOG_E("no audio manager when load adapter");
        return Status::ERROR_UNKNOWN;
    }
    if (adapter == nullptr) {
        MEDIA_LOG_E("**adapter null ptr");
        return Status::ERROR_INVALID_PARAMETER;
    }
    if (proxyManager->LoadAdapter(proxyManager, descriptor, adapter) < 0) {
        *adapter = nullptr;
        MEDIA_LOG_W("failed to load adapter %s", descriptor->adapterName);
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    if (*adapter == nullptr) {
        MEDIA_LOG_E("no available adapter after load adapter");
        return Status::ERROR_UNKNOWN;
    }

    int32_t retryCnt = 0;
    do {
        if ((*adapter)->InitAllPorts(*adapter) != 0) {
            OHOS::Media::OSAL::SleepFor(RETRY_INTERVAL);
        } else {
            break;
        }
        MEDIA_LOG_I("retry init port on adapter %s", descriptor->adapterName);
    } while (++retryCnt < MAX_RETRY_CNT);
    if (retryCnt >= MAX_RETRY_CNT) {
        MEDIA_LOG_W("cannot init port on adapter %s after retry %d times", descriptor->adapterName, retryCnt);
        proxyManager->UnloadAdapter(proxyManager, *adapter);
        *adapter = nullptr;
        return Status::ERROR_UNKNOWN;
    }
    return Status::OK;
}

std::shared_ptr<AudioSinkPlugin> AudioSinkPluginCreator(const std::string &name)
{
    return std::make_shared<OHOS::Media::HosLitePlugin::HdiSink>(name);
}

Status RegisterHdiSinkPlugins(const std::shared_ptr<Register>& reg)
{
    auto proxyManager = GetAudioManagerFuncs();
    if (proxyManager == nullptr) {
        MEDIA_LOG_E("cannot find audio manager funcs");
        return Status::ERROR_UNKNOWN;
    }
    int32_t adapterSize = 0;
    AudioAdapterDescriptor *descriptors = nullptr;
    int32_t ret = proxyManager->GetAllAdapters(proxyManager, &descriptors, &adapterSize);
    if (ret != 0 || adapterSize == 0) {
        MEDIA_LOG_E("cannot find available audio adapter");
        return Status::OK;
    }
    for (int32_t index = 0; index < adapterSize; index++) {
        AudioAdapter *adapter = nullptr;
        const auto &desc = descriptors[index];
        if (LoadAndInitAdapter(proxyManager, &descriptors[index], &adapter) != Status::OK) {
            continue;
        }
        CapabilitySet adapterCapabilities;
        for (uint32_t portIndex = 0; portIndex < desc.portNum; portIndex++) {
            if (desc.ports[portIndex].dir != PORT_OUT) {
                continue;
            }
            Capability capability(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
            adapterCapabilities.emplace_back(capability);
            break;
        }
        if (adapterCapabilities.empty()) {
            continue;
        }
        AudioSinkPluginDef sinkPluginDef;
        sinkPluginDef.creator = AudioSinkPluginCreator;
        sinkPluginDef.name = desc.adapterName;
        sinkPluginDef.inCaps = adapterCapabilities;
        sinkPluginDef.rank = RANK100;
        proxyManager->UnloadAdapter(proxyManager, adapter);
        if (reg->AddPlugin(sinkPluginDef) == Status::OK) {
            MEDIA_LOG_D("register plugin %s succ.", desc.adapterName);
        } else {
            MEDIA_LOG_W("register plugin %s failed", desc.adapterName);
        }
    }
    return Status::OK;
}

template<typename T>
inline Status AssignIfCastSuccess(T &lvalue, const Any& anyValue, const char* tagName)
{
    if (typeid(T) == anyValue.Type()) {
        lvalue = AnyCast<const T &>(anyValue);
        MEDIA_LOG_I("AssignIfCastSuccess found %s", tagName);
        return Status::OK;
    } else {
        MEDIA_LOG_W("tag:%s value type mismatch", tagName);
        return Status::ERROR_MISMATCHED_TYPE;
    }
}


int32_t CalculateBufferSize(const AudioSampleAttributes &attributes)
{
    return attributes.frameSize * attributes.period;
}

PLUGIN_DEFINITION(HdiAuSink, LicenseType::APACHE_V2, RegisterHdiSinkPlugins, []() {});
}
namespace OHOS {
namespace Media {
namespace HosLitePlugin {
using namespace OHOS::Media::Plugin;

HdiSink::HdiSink(std::string name) : Plugin::AudioSinkPlugin(std::move(name)), audioManager_(nullptr)
{
    // default is media
    sampleAttributes_.type = AUDIO_IN_MEDIA;
}

Status HdiSink::Init()
{
    MEDIA_LOG_D("Init entered.");
    if (pluginState_ == State::DESTROYED) {
        MEDIA_LOG_E("plugin has been already destroyed, cannot init any more");
        return Status::ERROR_WRONG_STATE;
    }
    if (pluginState_ != State::CREATED) {
        MEDIA_LOG_I("plugin has been already inited");
        return Status::OK;
    }
    audioManager_ = GetAudioManagerFuncs();
    if (audioManager_ == nullptr) {
        MEDIA_LOG_E("Init error due to audioManager nullptr");
        return Status::ERROR_UNKNOWN;
    }
    int32_t adapterSize = 0;
    AudioAdapterDescriptor *descriptors = nullptr;
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, &descriptors, &adapterSize);
    if (ret != 0 || adapterSize == 0) {
        MEDIA_LOG_E("cannot find available audio adapter");
        return Status::ERROR_UNKNOWN;
    }
    for (int32_t index = 0; index < adapterSize; index++) {
        const auto &desc = descriptors[index];
        if (pluginName_ != desc.adapterName) {
            continue;
        }

        if (LoadAndInitAdapter(audioManager_, &descriptors[index], &audioAdapter_) != Status::OK) {
            continue;
        }
        adapterDescriptor_ = descriptors[index];
    }
    if (audioAdapter_ == nullptr) {
        MEDIA_LOG_E("cannot find adapter with name %s", pluginName_.c_str());
        return Status::ERROR_UNKNOWN;
    }
    if (!renderThread_) {
        renderThread_ = std::make_shared<OHOS::Media::OSAL::Task>("auRenderThread");
        renderThread_->RegisterHandler([this] { DoRender(); });
    }
    pluginState_ = State::INITIALIZED;
    return Status::OK;
}

Media::Plugin::Status HdiSink::ReleaseRender()
{
    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        if (audioAdapter_ != nullptr && audioRender_ != nullptr) {
            audioAdapter_->DestroyRender(audioAdapter_, audioRender_);
            audioRender_ = nullptr;
        }
    }
    return Status::OK;
}

Status HdiSink::Deinit()
{
    MEDIA_LOG_E("Deinit entered.");
    if (pluginState_ == State::DESTROYED || pluginState_ == State::CREATED) {
        MEDIA_LOG_I("no need to destroy");
        return Status::OK;
    }
    if (renderThread_ != nullptr) {
        renderThread_->Stop();
    }
    // release all resources
    ReleaseRender();
    if (audioManager_ != nullptr) {
        if (audioAdapter_ != nullptr) {
            audioManager_->UnloadAdapter(audioManager_, audioAdapter_);
            audioAdapter_ = nullptr;
        }
        audioManager_ = nullptr;
    }
    pluginState_ = State::DESTROYED;
    return Status::OK;
}

Status HdiSink::SetParameter(Tag tag, const ValueType &value)
{
    if (pluginState_ == State::DESTROYED || pluginState_ == State::CREATED) {
        MEDIA_LOG_E("cannot set parameter in state %d", pluginState_.load());
        return Status::ERROR_WRONG_STATE;
    }
    switch (tag) {
        case Tag::AUDIO_CHANNELS:
            return AssignIfCastSuccess<uint32_t>(sampleAttributes_.channelCount, value, "channel");
        case Tag::AUDIO_SAMPLE_RATE:
            return AssignIfCastSuccess<uint32_t>(sampleAttributes_.sampleRate, value, "sampleRate");
        case Tag::AUDIO_SAMPLE_FORMAT: {
            AudioSampleFormat format;
            auto ret = AssignIfCastSuccess<AudioSampleFormat>(format, value, "audioSampleFormat");
            if (ret != Status::OK) {
                return ret;
            }
            if (PluginAuFormat2HdiAttrs(format, sampleAttributes_)) {
                return Status::OK;
            } else {
                MEDIA_LOG_E("audioSampleFormat mismatch");
                ret = Status::ERROR_MISMATCHED_TYPE;
                break;
            }
        }
        case Tag::AUDIO_SAMPLE_PRE_FRAME:
            return AssignIfCastSuccess<uint32_t>(sampleAttributes_.period, value, "samples per frame");
        case Tag::AUDIO_CHANNEL_LAYOUT: {
            AudioChannelLayout layout;
            auto ret = AssignIfCastSuccess<AudioChannelLayout>(layout, value, "audioChannelLayout");
            if (ret != Status::OK) {
                return ret;
            }
            if (PluginChannelLayout2HdiMask(layout, channelMask_)) {
                return Status::OK;
            } else {
                MEDIA_LOG_E("audioChannelLayout mismatch");
                return Status::ERROR_MISMATCHED_TYPE;
            }
        }
        default:
            MEDIA_LOG_I("receive one parameter with unconcern tag, ignore it");
    }
    return Status::OK;
}

Status HdiSink::GetParameter(Tag tag, ValueType &value)
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status HdiSink::Prepare()
{
    if (pluginState_ != State::PREPARED && pluginState_ != State::INITIALIZED) {
        MEDIA_LOG_E("cannot prepare in state %d", pluginState_.load());
        return Status::ERROR_WRONG_STATE;
    }
    if (pluginState_ == State::PREPARED) {
        return Status::OK;
    }

    sampleAttributes_.frameSize = GetPcmBytes(sampleAttributes_.format) * sampleAttributes_.channelCount;
    sampleAttributes_.startThreshold = sampleAttributes_.period / sampleAttributes_.frameSize;
    sampleAttributes_.stopThreshold = INT32_MAX;
    bool foundPort = false;
    for (uint32_t portIndex = 0; portIndex < adapterDescriptor_.portNum; portIndex++) {
        if (adapterDescriptor_.ports[portIndex].dir == PORT_OUT) {
            audioPort_ = adapterDescriptor_.ports[portIndex];
            foundPort = true;
            break;
        }
    }
    if (!foundPort) {
        MEDIA_LOG_E("cannot find out port");
        return Status::ERROR_UNKNOWN;
    }

    deviceDescriptor_.portId = audioPort_.portId;
    deviceDescriptor_.pins = PIN_OUT_SPEAKER;
    deviceDescriptor_.desc = nullptr;

    MEDIA_LOG_I("create render on adapter: %s,  port: %d, with parameters: category %s, channels %d, sampleRate %d,"
        " audioChannelMask %x, format %d, isSignedData %d, interleaved %d, period %u, frameSize %u",
        adapterDescriptor_.adapterName, deviceDescriptor_.portId,
        (sampleAttributes_.type == AUDIO_IN_MEDIA) ? "media" : "communication", sampleAttributes_.channelCount,
        sampleAttributes_.sampleRate, channelMask_, sampleAttributes_.format, sampleAttributes_.isSignedData,
        sampleAttributes_.interleaved, sampleAttributes_.period, sampleAttributes_.frameSize);

    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        auto ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDescriptor_, &sampleAttributes_, &audioRender_);
        if (ret != 0) {
            MEDIA_LOG_E("cannot create render with error code %lx", ret);
            audioRender_ = nullptr;
            return Status::ERROR_UNKNOWN;
        }
    }
    MEDIA_LOG_I("create audio render successfully");
    ringBuffer_ = std::make_shared<RingBuffer>(DEFAULT_BUFFER_POOL_SIZE * CalculateBufferSize(sampleAttributes_));
    if (!ringBuffer_->Init()) {
        MEDIA_LOG_E("cannot allocate enough buffer for ring buffer cache");
        return Status::ERROR_NO_MEMORY;
    }

    pluginState_ = State::PREPARED;
    return Status::OK;
}

Status HdiSink::Reset()
{
    MEDIA_LOG_D("Reset entered.");
    if (pluginState_ != State::PREPARED && pluginState_ != State::RUNNING && pluginState_ != State::PAUSED) {
        MEDIA_LOG_I("cannot reset in state %d", pluginState_.load());
        return Status::ERROR_WRONG_STATE;
    }
    ReleaseRender();
    (void)memset_s(&audioPort_, sizeof(audioPort_), 0, sizeof(audioPort_));
    (void)memset_s(&sampleAttributes_, sizeof(sampleAttributes_), 0, sizeof(sampleAttributes_));
    (void)memset_s(&deviceDescriptor_, sizeof(deviceDescriptor_), 0, sizeof(deviceDescriptor_));
    channelMask_ = AUDIO_CHANNEL_MONO;

    return Status::OK;
}

Status HdiSink::Start()
{
    MEDIA_LOG_D("Start entered.");
    if (pluginState_ != State::PREPARED && pluginState_ != State::RUNNING && pluginState_ != State::PAUSED) {
        MEDIA_LOG_E("cannot Start in state %d", pluginState_.load());
        return Status::ERROR_WRONG_STATE;
    }
    if (pluginState_ == State::RUNNING) {
        MEDIA_LOG_I("already in running state, ignore start");
        return Status::OK;
    }
    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        if (audioRender_ == nullptr) {
            MEDIA_LOG_E("no available render");
            return Status::ERROR_UNKNOWN;
        }

        if (audioRender_->control.Start(audioRender_) != 0) {
            MEDIA_LOG_E("audio render start error");
            return Status::ERROR_UNKNOWN;
        }
    }
    ringBuffer_->SetActive(true);
    renderThread_->Start();
    pluginState_ = State::RUNNING;
    return Status::OK;
}

Status HdiSink::Stop()
{
    MEDIA_LOG_D("Stop Entered");
    if (pluginState_ != State::RUNNING && pluginState_ != State::PAUSED) {
        MEDIA_LOG_W("Stop is called when not running or paused, ignore it");
        return Status::OK;
    }
    ringBuffer_->SetActive(false);
    renderThread_->Pause();
    pluginState_ = State::PREPARED;
    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        if (audioRender_ == nullptr) {
            MEDIA_LOG_E("no available render");
            return Status::OK;
        }
        if (audioRender_->control.Stop(audioRender_) != 0) {
            MEDIA_LOG_E("audio render stop error");
            return Status::ERROR_UNKNOWN;
        }
    }
    MEDIA_LOG_D("Stop Exited");
    return Status::OK;
}

bool HdiSink::IsParameterSupported(Tag tag)
{
    return false;
}

std::shared_ptr<Allocator> HdiSink::GetAllocator()
{
    return nullptr;
}

Status HdiSink::SetCallback(const std::shared_ptr<Callback> &cb)
{
    eventCallback_ = cb;
    return Status::OK;
}

Status HdiSink::GetMute(bool &mute)
{
    OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
    if (audioRender_ == nullptr) {
        MEDIA_LOG_W("no render available, get mute must be called after prepared");
        return Status::ERROR_WRONG_STATE;
    }
    if (audioRender_->volume.GetMute(audioRender_, &mute) != 0) {
        MEDIA_LOG_E("get mute failed");
        return Status::ERROR_UNKNOWN;
    }
    return Status::OK;
}

Status HdiSink::SetMute(bool mute)
{
    // todo when to set mute
    OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
    if (audioRender_ == nullptr) {
        MEDIA_LOG_W("no render available, set mute must be called after prepare");
        return Status::ERROR_WRONG_STATE;
    }
    if (audioRender_->volume.SetMute(audioRender_, mute) != 0) {
        MEDIA_LOG_E("set mute failed");
        return Status::ERROR_UNKNOWN;
    }
    return Status::OK;
}

Status HdiSink::GetVolume(float &volume)
{
    OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
    if (audioRender_ == nullptr) {
        MEDIA_LOG_W("no render available, get volume must be called after prepare");
        return Status::ERROR_WRONG_STATE;
    }
    if (audioRender_->volume.GetVolume(audioRender_, &volume) != 0) {
        MEDIA_LOG_E("get volume failed");
        return Status::ERROR_UNKNOWN;
    }
    volume /= MAX_VOLUME;
    return Status::OK;
}

Status HdiSink::SetVolume(float volume)
{
    OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
    if (audioRender_ == nullptr) {
        MEDIA_LOG_W("no render available, set volume must be called after prepare");
        return Status::ERROR_WRONG_STATE;
    }
    volume *= MAX_VOLUME;
    if (audioRender_->volume.SetVolume(audioRender_, volume) != 0) {
        MEDIA_LOG_E("set volume failed");
        return Status::ERROR_UNKNOWN;
    }
    MEDIA_LOG_W("set volume to %.3f", volume);
    return Status::OK;
}

Status HdiSink::GetSpeed(float &speed)
{
    OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
    if (audioRender_ == nullptr) {
        MEDIA_LOG_W("no render available, get speed must be called after prepare");
        return Status::ERROR_WRONG_STATE;
    }
    if (audioRender_->GetRenderSpeed(audioRender_, &speed) != 0) {
        MEDIA_LOG_E("get speed failed");
        return Status::ERROR_UNKNOWN;
    }
    return Status::OK;
}

Status HdiSink::SetSpeed(float speed)
{
    OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
    if (audioRender_ == nullptr) {
        MEDIA_LOG_W("no render available, set speed must be called after prepare");
        return Status::ERROR_WRONG_STATE;
    }
    if (audioRender_->SetRenderSpeed(audioRender_, speed) != 0) {
        MEDIA_LOG_E("set speed failed");
        return Status::ERROR_UNKNOWN;
    }
    return Status::OK;
}

Status HdiSink::Pause()
{
    MEDIA_LOG_D("Pause Entered");
    if (pluginState_ != State::RUNNING) {
        MEDIA_LOG_I("pause in status %d, ignore pause", pluginState_.load());
        return Status::OK;
    }
    renderThread_->Pause();

    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        if (audioRender_ != nullptr && audioRender_->control.Pause(audioRender_) != 0) {
            MEDIA_LOG_E("pause failed");
            return Status::ERROR_UNKNOWN;
        }
    }
    pluginState_ = State::PAUSED;
    return Status::OK;
}

Status HdiSink::Resume()
{
    MEDIA_LOG_D("Resume Entered");
    if (pluginState_ != State::PAUSED) {
        MEDIA_LOG_I("resume in status %d, ignore pause", pluginState_.load());
        return Status::OK;
    }
    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        if (audioRender_ != nullptr && audioRender_->control.Resume(audioRender_) != 0) {
            MEDIA_LOG_E("resume failed");
            return Status::ERROR_UNKNOWN;
        }
    }
    renderThread_->Start();
    pluginState_ = State::RUNNING;
    return Status::OK;
}

Status HdiSink::GetLatency(uint64_t &ms)
{
    OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
    if (audioRender_ == nullptr) {
        MEDIA_LOG_W("no render available, get latency must be called after prepare");
        return Status::ERROR_WRONG_STATE;
    }
    uint32_t tmp;
    if (audioRender_->GetLatency(audioRender_, &tmp) != 0) {
        MEDIA_LOG_E("get latency failed");
        return Status::ERROR_UNKNOWN;
    }
    ms = tmp;
    return Status::OK;
}

Status HdiSink::GetFrameSize(size_t &size)
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status HdiSink::GetFrameCount(uint32_t &count)
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status HdiSink::Write(const std::shared_ptr<Buffer> &input)
{
    MEDIA_LOG_D("Write begin.");
    if (pluginState_ != State::RUNNING) {
        MEDIA_LOG_E("cannot write buffer until enter running state");
        return Status::ERROR_WRONG_STATE;
    }
    if (input != nullptr && !input->IsEmpty()) {
        ringBuffer_->WriteBuffer(input);
        MEDIA_LOG_D("write to ring buffer");
    }
    MEDIA_LOG_D("Write finished.");
    return Status::OK;
}

Status HdiSink::Flush()
{
    MEDIA_LOG_I("Flush Entered");
    ringBuffer_->Clear();
    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        if (audioRender_ == nullptr) {
            MEDIA_LOG_E("no render available, flush must be called after prepare");
            return Status::ERROR_WRONG_STATE;
        }

        if (audioRender_->control.Flush(audioRender_) != 0) {
            MEDIA_LOG_E("audio render flush error");
            return Status::ERROR_UNKNOWN;
        }
    }
    MEDIA_LOG_I("Flush Exited.");
    return Status::OK;
}

void HdiSink::DoRender()
{
    MEDIA_LOG_D("DoRender started");
    if (pluginState_ != State::RUNNING) {
        return;
    }

    size_t outSize = 0;
    auto outFramePtr = ringBuffer_->ReadBufferWithoutAdvance(CalculateBufferSize(sampleAttributes_), outSize);
    if (outFramePtr == nullptr || outSize == 0) {
        return;
    }
    uint64_t renderSize = 0;
    auto ret = 0;
    {
        OHOS::Media::OSAL::ScopedLock lock(renderMutex_);
        if (audioRender_ != nullptr) {
            ret = audioRender_->RenderFrame(audioRender_, outFramePtr.get(), outSize, &renderSize);
        }
    }
    if (ret != 0) {
        if (ret == HI_ERR_VI_BUF_FULL) {
            MEDIA_LOG_I("renderFrame buffer full");
            uint32_t latency = sampleAttributes_.period * SEC_TO_MILLS / sampleAttributes_.sampleRate;
            MEDIA_LOG_D("latency origin %u ms", latency);
            OHOS::Media::OSAL::SleepFor(latency / HALF);
        } else {
            MEDIA_LOG_E("renderFrame error with code %lx", ret);
        }
        return;
    } else {
        MEDIA_LOG_D("render frame %d", renderSize);
    }

    if (renderSize != 0) {
        ringBuffer_->Advance(renderSize);
    }
}
}
}
}
