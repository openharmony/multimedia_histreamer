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

#include "plugin_manager.h"
#include "interface/audio_sink_plugin.h"
#include "interface/codec_plugin.h"
#include "interface/demuxer_plugin.h"
#include "interface/source_plugin.h"
#include "interface/video_sink_plugin.h"
#include "plugin_register.h"
#include "plugin_wrapper.h"

#include <utility>

using namespace OHOS::Media::Plugin;

PluginManager::PluginManager()
{
    Init();
}

PluginManager::~PluginManager()
{
}

std::set<std::string> PluginManager::ListPlugins(PluginType type)
{
    return pluginRegister->ListPlugins(type);
}

std::shared_ptr<PluginInfo> PluginManager::GetPluginInfo(PluginType type, const std::string& name)
{
    std::shared_ptr<PluginRegInfo> regInfo = pluginRegister->GetPluginRegInfo(type, name);
    if (regInfo && regInfo->info && regInfo->info->pluginType == type) {
        return regInfo->info;
    }
    return std::shared_ptr<PluginInfo>();
}

int32_t PluginManager::Sniffer(const std::string& name, std::shared_ptr<DataSourceHelper> source)
{
    std::shared_ptr<PluginRegInfo> regInfo = pluginRegister->GetPluginRegInfo(PluginType::DEMUXER, name);
    if (!regInfo) {
        return 0;
    }
    if (regInfo->info->pluginType == PluginType::DEMUXER) {
        return regInfo->sniffer(name, std::make_shared<DataSourceWrapper>(regInfo->packageDef->pkgVersion, source));
    }
    return 0;
}

void PluginManager::Init()
{
    pluginRegister = std::make_shared<PluginRegister>();
    pluginRegister->RegisterPlugins();
}

std::shared_ptr<Demuxer> PluginManager::CreateDemuxerPlugin(const std::string& name)
{
    std::shared_ptr<PluginRegInfo> regInfo = pluginRegister->GetPluginRegInfo(PluginType::DEMUXER, name);
    if (!regInfo) {
        return std::shared_ptr<Demuxer>();
    }
    std::shared_ptr<DemuxerPlugin> demuxer = std::dynamic_pointer_cast<DemuxerPlugin>(regInfo->creator(name));
    if (!demuxer) {
        return std::shared_ptr<Demuxer>();
    }
    return std::shared_ptr<Demuxer>(new Demuxer(regInfo->packageDef->pkgVersion, regInfo->info->apiVersion, demuxer));
}

std::shared_ptr<Source> PluginManager::CreateSourcePlugin(const std::string& name)
{
    std::shared_ptr<PluginRegInfo> regInfo = pluginRegister->GetPluginRegInfo(PluginType::SOURCE, name);
    if (!regInfo) {
        return std::shared_ptr<Source>();
    }
    std::shared_ptr<SourcePlugin> source = std::dynamic_pointer_cast<SourcePlugin>(regInfo->creator(name));
    if (!source) {
        return std::shared_ptr<Source>();
    }
    return std::shared_ptr<Source>(new Source(regInfo->packageDef->pkgVersion, regInfo->info->apiVersion, source));
}

std::shared_ptr<Codec> PluginManager::CreateCodecPlugin(const std::string& name)
{
    std::shared_ptr<PluginRegInfo> regInfo = pluginRegister->GetPluginRegInfo(PluginType::CODEC, name);
    if (!regInfo) {
        return std::shared_ptr<Codec>();
    }
    std::shared_ptr<CodecPlugin> codec = std::dynamic_pointer_cast<CodecPlugin>(regInfo->creator(name));
    if (!codec) {
        return std::shared_ptr<Codec>();
    }
    return std::shared_ptr<Codec>(new Codec(regInfo->packageDef->pkgVersion, regInfo->info->apiVersion, codec));
}

std::shared_ptr<AudioSink> PluginManager::CreateAudioSinkPlugin(const std::string& name)
{
    std::shared_ptr<PluginRegInfo> regInfo = pluginRegister->GetPluginRegInfo(PluginType::AUDIO_SINK, name);
    if (!regInfo) {
        return std::shared_ptr<AudioSink>();
    }
    std::shared_ptr<AudioSinkPlugin> audioSink = std::dynamic_pointer_cast<AudioSinkPlugin>(regInfo->creator(name));
    if (!audioSink) {
        return std::shared_ptr<AudioSink>();
    }
    return std::shared_ptr<AudioSink>(
        new AudioSink(regInfo->packageDef->pkgVersion, regInfo->info->apiVersion, audioSink));
}

std::shared_ptr<VideoSink> PluginManager::CreateVideoSinkPlugin(const std::string& name)
{
    std::shared_ptr<PluginRegInfo> regInfo = pluginRegister->GetPluginRegInfo(PluginType::VIDEO_SINK, name);
    if (!regInfo) {
        return std::shared_ptr<VideoSink>();
    }
    std::shared_ptr<VideoSinkPlugin> videoSink = std::dynamic_pointer_cast<VideoSinkPlugin>(regInfo->creator(name));
    if (!videoSink) {
        return std::shared_ptr<VideoSink>();
    }
    return std::shared_ptr<VideoSink>(
        new VideoSink(regInfo->packageDef->pkgVersion, regInfo->info->apiVersion, videoSink));
}