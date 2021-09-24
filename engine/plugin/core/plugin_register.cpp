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

#include "plugin_register.h"
#include "interface/audio_sink_plugin.h"
#include "interface/codec_plugin.h"
#include "interface/demuxer_plugin.h"
#include "interface/source_plugin.h"
#include "interface/video_sink_plugin.h"

#include "all_plugin_static.h"

#include <dirent.h>

using namespace OHOS::Media::Plugin;

static std::map<PluginType, int> g_apiVersionMap = {
    {PluginType::SOURCE, SOURCE_API_VERSION},
    {PluginType::DEMUXER, DEMUXER_API_VERSION},
    {PluginType::CODEC, CODEC_API_VERSION},
    {PluginType::AUDIO_SINK, AUDIO_SINK_API_VERSION},
    {PluginType::VIDEO_SINK, VIDEO_SINK_API_VERSION},
};

static std::string g_libFileHead = "libplugin_";

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
static std::string g_libFileTail = ".dll";
static std::string g_fileSeparator = "\\";
#else
static std::string g_libFileTail = ".so";
static std::string g_fileSeparator = "/";
#endif

PluginRegister::~PluginRegister()
{
    UnregisterAllPlugins();
    registerData->registerNames.clear();
    registerData->registerTable.clear();
}

Status PluginRegister::RegisterImpl::AddPackage(const PackageDef& def)
{
    return SetPackageDef(def);
}

Status PluginRegister::RegisterImpl::SetPackageDef(const PackageDef& def)
{
    packageDef->name = def.name;
    packageDef->licenseType = def.licenseType;
    packageDef->pkgVersion = def.pkgVersion;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::AddPlugin(const PluginDefBase& def)
{
    if (!Verification(def)) {
        // 插件定义参数校验不合法
        return Status::ERROR_INVALID_DATA;
    }
    if (!VersionMatched(def)) {
        // 版本不匹配，不给注册
        return Status::ERROR_UNKNOWN;
    }
    if (registerData->IsPluginExist(def.pluginType, def.name)) {
        if (MoreAcceptable(registerData->registerTable[def.pluginType][def.name], def)) {
            registerData->registerTable[def.pluginType].erase(def.name);
        } else {
            // 重复注册，且有更合适的版本存在
            return Status::ERROR_ALREADY_EXISTS;
        }
    }
    registerData->registerNames[def.pluginType].insert(def.name);
    registerData->registerTable[def.pluginType][def.name] = BuildRegInfo(def);
    return Status::OK;
}

std::shared_ptr<PluginRegInfo> PluginRegister::RegisterImpl::BuildRegInfo(const PluginDefBase& def)
{
    std::shared_ptr<PluginRegInfo> regInfo = std::make_shared<PluginRegInfo>();
    regInfo->packageDef = packageDef;
    switch (def.pluginType) {
        case PluginType::SOURCE: {
            InitSourceInfo(regInfo, def);
            break;
        }
        case PluginType::DEMUXER: {
            InitDemuxerInfo(regInfo, def);
            break;
        }
        case PluginType::CODEC: {
            InitCodecInfo(regInfo, def);
            break;
        }
        case PluginType::AUDIO_SINK: {
            InitAudioSinkInfo(regInfo, def);
            break;
        }
        case PluginType::VIDEO_SINK: {
            InitVideoSinkInfo(regInfo, def);
            break;
        }
        default:
            return std::shared_ptr<PluginRegInfo>();
    }
    regInfo->loader = std::move(pluginLoader);
    return regInfo;
}

bool PluginRegister::RegisterImpl::Verification(const PluginDefBase& definition)
{
    if (definition.rank < 0 || definition.rank > 100) {
        return false;
    }
    return (definition.pluginType != PluginType::INVALID_TYPE);
}

bool PluginRegister::RegisterImpl::VersionMatched(const PluginDefBase& definition)
{
    int major = (definition.apiVersion >> 16) & 0xFFFF; // 16
    int minor = definition.apiVersion & 0xFFFF;
    uint32_t version = g_apiVersionMap[definition.pluginType];
    int coreMajor = (version >> 16) & 0xFFFF; // 16
    int coreMinor = version & 0xFFFF;
    return (major == coreMajor) && (minor <= coreMinor);
}

bool PluginRegister::RegisterImpl::MoreAcceptable(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    return false;
}

void PluginRegister::RegisterImpl::SetPluginInfo(std::shared_ptr<PluginInfo>& info, const PluginDefBase& def)
{
    info->apiVersion = def.apiVersion;
    info->pluginType = def.pluginType;
    info->name = def.name;
    info->description = def.description;
    info->rank = def.rank;
}

Status PluginRegister::RegisterImpl::InitSourceInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    auto& base = (SourcePluginDef&)def;
    reg->creator = reinterpret_cast<PluginCreatorFunc<PluginBase>>(base.creator);
    std::shared_ptr<PluginInfo> info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->extra.insert({PLUGIN_INFO_EXTRA_PROTOCOL, base.protocol});
    SourceCapabilityConvert(info, def);
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitDemuxerInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    auto& base = (DemuxerPluginDef&)def;
    reg->creator = reinterpret_cast<PluginCreatorFunc<PluginBase>>(base.creator);
    reg->sniffer = base.sniffer;
    std::shared_ptr<PluginInfo> info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->extra.insert({PLUGIN_INFO_EXTRA_EXTENSIONS, base.extensions});
    DemuxerCapabilityConvert(info, def);
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitCodecInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    auto& base = (CodecPluginDef&)def;
    reg->creator = reinterpret_cast<PluginCreatorFunc<PluginBase>>(base.creator);
    std::shared_ptr<PluginInfo> info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->extra.insert({PLUGIN_INFO_EXTRA_CODEC_TYPE, base.codecType});
    CodecCapabilityConvert(info, def);
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitAudioSinkInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = reinterpret_cast<PluginCreatorFunc<PluginBase>>(((AudioSinkPluginDef&)def).creator);
    std::shared_ptr<PluginInfo> info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    AudioSinkCapabilityConvert(info, def);
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitVideoSinkInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = reinterpret_cast<PluginCreatorFunc<PluginBase>>(((VideoSinkPluginDef&)def).creator);
    std::shared_ptr<PluginInfo> info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    VideoSinkCapabilityConvert(info, def);
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::SourceCapabilityConvert(std::shared_ptr<PluginInfo>& info,
                                                             const PluginDefBase& def)
{
    auto& base = (SourcePluginDef&)def;
    info->outCaps = base.outCaps;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::DemuxerCapabilityConvert(std::shared_ptr<PluginInfo>& info,
                                                              const PluginDefBase& def)
{
    auto& base = (DemuxerPluginDef&)def;
    info->inCaps = base.inCaps;
    info->outCaps = base.outCaps;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::CodecCapabilityConvert(std::shared_ptr<PluginInfo>& info, const PluginDefBase& def)
{
    auto& base = (CodecPluginDef&)def;
    info->inCaps = base.inCaps;
    info->outCaps = base.outCaps;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::AudioSinkCapabilityConvert(std::shared_ptr<PluginInfo>& info,
                                                                const PluginDefBase& def)
{
    auto& base = (AudioSinkPluginDef&)def;
    info->inCaps = base.inCaps;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::VideoSinkCapabilityConvert(std::shared_ptr<PluginInfo>& info,
                                                                const PluginDefBase& def)
{
    auto& base = (VideoSinkPluginDef&)def;
    info->inCaps = base.inCaps;
    return Status::OK;
}

std::set<std::string> PluginRegister::ListPlugins(PluginType type)
{
    return registerData->registerNames[type];
}

std::shared_ptr<PluginRegInfo> PluginRegister::GetPluginRegInfo(PluginType type, const std::string& name)
{
    if (registerData->IsPluginExist(type, name)) {
        return registerData->registerTable[type][name];
    }
    return std::shared_ptr<PluginRegInfo>();
}

void PluginRegister::RegisterPlugins()
{
    RegisterStaticPlugins();
    RegisterDynamicPlugins();
}

void PluginRegister::RegisterStaticPlugins()
{
    RegisterPluginStatic(std::make_shared<RegisterImpl>(registerData));
}

void PluginRegister::RegisterDynamicPlugins()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    RegisterPluginsFromPath(".");
    RegisterPluginsFromPath(".\\plugins");
    RegisterPluginsFromPath("..\\src\\plugin\\plugins\\source\\file_source");
#endif
}

void PluginRegister::RegisterPluginsFromPath(const char* libDirPath)
{
    DIR* libDir = opendir(libDirPath);
    if (libDir) {
        struct dirent* lib = nullptr;
        std::shared_ptr<PluginLoader> loader = nullptr;
        while ((lib = readdir(libDir))) {
            if (lib->d_name[0] == '.') {
                continue;
            }
            std::string libName = lib->d_name;
            if (libName.find(g_libFileHead) ||
                libName.compare(libName.size() - g_libFileTail.size(), g_libFileTail.size(), g_libFileTail)) {
                continue;
            }
            std::string pluginName =
                libName.substr(g_libFileHead.size(), libName.size() - g_libFileHead.size() - g_libFileTail.size());
            std::string libPath = libDirPath + g_fileSeparator + lib->d_name;
            loader = PluginLoader::Create(pluginName, libPath);
            if (loader) {
                loader->FetchRegisterFunction()(std::make_shared<RegisterImpl>(registerData, loader));
                registeredLoaders.push_back(loader);
            }
        }
        closedir(libDir);
    }
}

void PluginRegister::UnregisterAllPlugins()
{
    UnregisterPluginStatic();
    for (auto& loader : registeredLoaders) {
        EraseRegisteredPlugins(loader);
        loader->FetchUnregisterFunction()();
        loader.reset();
    }
    registeredLoaders.clear();
}

void PluginRegister::EraseRegisteredPlugins(std::shared_ptr<PluginLoader> loader)
{
    for (auto& it : registerData->registerTable) {
        PluginType type = it.first;
        auto plugins = it.second;
        for (auto info = plugins.begin(); info != plugins.end();) {
            if (info->second->loader == loader) {
                registerData->registerNames[type].erase(info->first);
                info = plugins.erase(info);
            } else {
                info++;
            }
        }
    }
}

bool PluginRegister::RegisterData::IsPluginExist(PluginType type, const std::string& name)
{
    return (registerTable.find(type) != registerTable.end() &&
            registerTable[type].find(name) != registerTable[type].end());
}
