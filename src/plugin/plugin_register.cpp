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
#include "plugin/plugin_register.h"

#include <algorithm>
#include <dirent.h>

#include "common/log.h"
#include "common/status.h"
#include "meta/meta_key.h"
#include "plugin/plugin_caps.h"

namespace OHOS {
namespace Media {
namespace Plugins {

PluginRegister::~PluginRegister()
{
    registerData_->registerNames.clear();
    registerData_->registerTable.clear();
}

Status PluginRegister::RegisterImpl::AddPackage(const PackageDef& def)
{
    return SetPackageDef(def);
}

Status PluginRegister::RegisterImpl::SetPackageDef(const PackageDef& def)
{
    packageDef = std::make_shared<PackageDef>(def);
    return Status::OK;
}

Status PluginRegister::RegisterImpl::AddPlugin(const PluginDefBase& def)
{
    if (!Verification(def)) {
        return Status::ERROR_INVALID_DATA;
    }
    if (!VersionMatched(def)) {
        return Status::ERROR_UNKNOWN;
    }
    if (registerData->IsPluginExist(def.pluginType, def.name)) {
        if (MoreAcceptable(registerData->registerTable[def.pluginType][def.name], def)) {
            registerData->registerTable[def.pluginType].erase(def.name);
        } else {
            return Status::ERROR_PLUGIN_ALREADY_EXISTS;
        }
    }
    UpdateRegisterTableAndRegisterNames(def);
    return Status::OK;
}

void PluginRegister::RegisterImpl::UpdateRegisterTableAndRegisterNames(const PluginDefBase& def)
{
    auto regInfo = std::make_shared<PluginRegInfo>();
    regInfo->packageDef = packageDef;
    switch (def.pluginType) {
        case PluginType::SOURCE:
            InitSourceInfo(regInfo, def);
            break;
        case PluginType::DEMUXER:
            InitDemuxerInfo(regInfo, def);
            break;
        case PluginType::MUXER:
            InitMuxerInfo(regInfo, def);
            break;
        case PluginType::AUDIO_DECODER:
        case PluginType::VIDEO_DECODER:
        case PluginType::AUDIO_ENCODER:
        case PluginType::VIDEO_ENCODER:
            InitCodecInfo(regInfo, def);
            break;
        case PluginType::AUDIO_SINK:
            InitAudioSinkInfo(regInfo, def);
            break;
        case PluginType::VIDEO_SINK:
            InitVideoSinkInfo(regInfo, def);
            break;
        case PluginType::OUTPUT_SINK:
            InitOutputSinkInfo(regInfo, def);
            break;
        case PluginType::GENERIC_PLUGIN:
            InitGenericPlugin(regInfo, def);
            break;
        default:
            return;
    }
    regInfo->loader = std::move(pluginLoader);
    registerData->registerTable[def.pluginType][def.name] = regInfo;
    if ((def.pluginType == PluginType::AUDIO_DECODER || def.pluginType == PluginType::VIDEO_DECODER
        || def.pluginType == PluginType::AUDIO_ENCODER || def.pluginType == PluginType::VIDEO_ENCODER)
        && AnyCast<CodecMode>(regInfo->info->extra[PLUGIN_INFO_EXTRA_CODEC_MODE]) == CodecMode::HARDWARE) {
        registerData->registerNames[def.pluginType].insert(registerData->registerNames[def.pluginType].begin(),
            def.name);
    } else {
        registerData->registerNames[def.pluginType].push_back(def.name);
    }
}

bool PluginRegister::RegisterImpl::Verification(const PluginDefBase& definition)
{
    if (definition.rank < 0 || definition.rank > 100) { // 100
        return false;
    }
    return (definition.pluginType != PluginType::INVALID_TYPE);
}

bool PluginRegister::RegisterImpl::VersionMatched(const PluginDefBase& /* definition */)
{
    return true;
}

// NOLINTNEXTLINE: should be static or anonymous namespace
bool PluginRegister::RegisterImpl::MoreAcceptable(std::shared_ptr<PluginRegInfo>& regInfo, const PluginDefBase& def)
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
    reg->creator = def.GetCreator();
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);

    auto capSet = def.GetInCaps();
    FALSE_RETURN_V_MSG_E(capSet.size() == 1, Status::ERROR_INVALID_PARAMETER, "capSet size is not 1");

    Any protocol;
    capSet[0].GetFixedValue<Any>(Tag::MEDIA_PROTOCOL_TYPE, protocol);
    FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<std::vector<ProtocolType>>(protocol),
                         Status::ERROR_INVALID_PARAMETER, "protocol param type error");
    auto type = AnyCast<std::vector<ProtocolType>>(protocol);
    info->extra.insert({PLUGIN_INFO_EXTRA_PROTOCOL, type});

    Any input;
    capSet[0].GetFixedValue<Any>(Tag::SRC_INPUT_TYPE, input);
    if (Any::IsSameTypeWith<SrcInputType>(input)) {
        auto inputType = AnyCast<SrcInputType>(input);
        info->extra.insert({PLUGIN_INFO_EXTRA_INPUT_TYPE, inputType});
    }

    info->outCaps = def.GetOutCaps();
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitDemuxerInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = def.GetCreator();
    reg->sniffer = def.GetSniffer();
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->extra.insert({PLUGIN_INFO_EXTRA_EXTENSIONS, def.GetExtensions()});
    info->inCaps = def.GetInCaps();
    info->outCaps = def.GetOutCaps();
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitMuxerInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = def.GetCreator();
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->inCaps = def.GetInCaps();
    info->outCaps = def.GetOutCaps();
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitCodecInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = def.GetCreator();
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);

    auto capSet = def.GetInCaps();
    FALSE_RETURN_V_MSG_E(capSet.size() == 1, Status::ERROR_INVALID_PARAMETER, "capSet size is not 1");

    Any mode;
    capSet[0].GetFixedValue<Any>(Tag::MEDIA_CODEC_MODE, mode);
    FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<CodecMode>(mode),
                         Status::ERROR_INVALID_PARAMETER, "codec mode param type error");
    auto codecMode = AnyCast<CodecMode>(mode);
    info->extra.insert({PLUGIN_INFO_EXTRA_CODEC_MODE, codecMode});

    info->inCaps = def.GetInCaps();
    info->outCaps = def.GetOutCaps();
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitAudioSinkInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = def.GetCreator();
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->inCaps = def.GetInCaps();
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitVideoSinkInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = def.GetCreator();
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->inCaps = def.GetInCaps();
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitOutputSinkInfo(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    reg->creator = def.GetCreator();
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);

    auto capSet = def.GetInCaps();
    FALSE_RETURN_V_MSG_E(capSet.size() == 1, Status::ERROR_INVALID_PARAMETER, "capSet size is not 1");

    Any protocol;
    capSet[0].GetFixedValue<Any>(Tag::MEDIA_PROTOCOL_TYPE, protocol);
    FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<ProtocolType>(protocol),
                         Status::ERROR_INVALID_PARAMETER, "protocol type error");
    auto protocolType = AnyCast<ProtocolType>(protocol);
    info->extra[PLUGIN_INFO_EXTRA_OUTPUT_TYPE] = protocolType;
    info->inCaps = def.GetInCaps();
    reg->info = info;
    return Status::OK;
}

Status PluginRegister::RegisterImpl::InitGenericPlugin(std::shared_ptr<PluginRegInfo>& reg, const PluginDefBase& def)
{
    auto& base = (GenericPluginDef&)def;
    reg->creator = base.creator;
    auto info = std::make_shared<PluginInfo>();
    SetPluginInfo(info, def);
    info->inCaps = base.inCaps;
    info->outCaps = base.outCaps;
    reg->info = info;
    return Status::OK;
}

std::vector<std::string> PluginRegister::ListPlugins(PluginType type, CodecMode preferredCodecMode)
{
    if ((type == PluginType::AUDIO_DECODER || type == PluginType::VIDEO_DECODER
        || type == PluginType::AUDIO_ENCODER || type == PluginType::VIDEO_ENCODER)
        && preferredCodecMode != CodecMode::HARDWARE) {
        std::vector<std::string> pluginNames {registerData_->registerNames[type]};
        std::reverse(pluginNames.begin(), pluginNames.end());
        return pluginNames;
    } else {
        return registerData_->registerNames[type];
    }
}

int PluginRegister::GetAllRegisteredPluginCount()
{
    int count = 0;
    for (auto it : registerData_->registerTable) {
        count += it.second.size();
    }
    return count;
}

std::shared_ptr<PluginRegInfo> PluginRegister::GetPluginRegInfo(PluginType type, const std::string& name)
{
    if (registerData_->IsPluginExist(type, name)) {
        return registerData_->registerTable[type][name];
    }
    return {};
}

void PluginRegister::RegisterPlugins()
{
    RegisterStaticPlugins();
    RegisterDynamicPlugins();
}

void PluginRegister::RegisterGenericPlugin(const GenericPluginDef& pluginDef)
{
    (void)staticPluginRegister_->AddPackage({pluginDef.pkgVersion, pluginDef.pkgName, pluginDef.license});
    FALSE_LOG_MSG(staticPluginRegister_->AddPlugin(pluginDef) == Status::OK,
        "Plugin " PUBLIC_LOG_S  " register fail.", pluginDef.name.c_str());
}

void PluginRegister::RegisterGenericPlugins(const std::vector<GenericPluginDef>& vecPluginDef)
{
    for (auto& pluginDef : vecPluginDef) {
        (void)staticPluginRegister_->AddPackage({pluginDef.pkgVersion, pluginDef.pkgName, pluginDef.license});
        FALSE_LOG_MSG(staticPluginRegister_->AddPlugin(pluginDef) == Status::OK,
            "Plugin " PUBLIC_LOG_S  " register fail.", pluginDef.name.c_str());
    }
}

void PluginRegister::RegisterStaticPlugins()
{
}

void PluginRegister::RegisterDynamicPlugins()
{
    RegisterPluginsFromPath(HST_PLUGIN_PATH);
}

void PluginRegister::RegisterPluginsFromPath(const char* libDirPath)
{
#ifdef DYNAMIC_PLUGINS
    static std::string libFileHead = "libmedia_plugin_";
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    static std::string fileSeparator = "\\";
    #else
    static std::string fileSeparator = "/";
    #endif
    static std::string libFileTail = HST_PLUGIN_FILE_TAIL;
    MEDIA_LOG_D("plugin path %{public}s", libDirPath);
    DIR* libDir = opendir(libDirPath);
    if (libDir) {
        struct dirent* lib = nullptr;
        std::shared_ptr<PluginLoader> loader = nullptr;
        while ((lib = readdir(libDir))) {
            if (lib->d_name[0] == '.') {
                continue;
            }
            MEDIA_LOG_D("plugin name %{public}s", lib->d_name);
            std::string libName = lib->d_name;
            if (libName.find(libFileHead) ||
                libName.compare(libName.size() - libFileTail.size(), libFileTail.size(), libFileTail)) {
                continue;
            }
            std::string pluginName =
                libName.substr(libFileHead.size(), libName.size() - libFileHead.size() - libFileTail.size());
            std::string libPath = libDirPath + fileSeparator + lib->d_name;
            loader = PluginLoader::Create(pluginName, libPath);
            if (loader) {
                loader->FetchRegisterFunction()(std::make_shared<RegisterImpl>(registerData_, loader));
                registeredLoaders_.push_back(loader);
            }
        }
        closedir(libDir);
    }
#endif
}

void PluginRegister::UnregisterAllPlugins()
{
#ifdef DYNAMIC_PLUGINS
    for (auto& loader : registeredLoaders_) {
        EraseRegisteredPluginsByLoader(loader);
        loader->FetchUnregisterFunction()();
        loader.reset();
    }
#endif
    registeredLoaders_.clear();
}

void PluginRegister::DeletePlugin(std::map<std::string, std::shared_ptr<PluginRegInfo>>& plugins,
    std::map<std::string, std::shared_ptr<PluginRegInfo>>::iterator& info)
{
    auto type = info->second->info->pluginType;
    for (auto it = registerData_->registerNames[type].begin();
         it != registerData_->registerNames[type].end();) {
        if (*it == info->first) {
            it = registerData_->registerNames[type].erase(it);
        } else {
            ++it;
        }
    }
    registerData_->registerTable[type].erase(info->first);
    info = plugins.erase(info);
}
void PluginRegister::EraseRegisteredPluginsByLoader(const std::shared_ptr<PluginLoader>& loader)
{
    for (auto& it : registerData_->registerTable) {
        auto plugins = it.second;
        for (auto info = plugins.begin(); info != plugins.end();) {
            if (info->second->loader == loader) {
                DeletePlugin(plugins, info);
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
} // namespace Plugins
} // namespace Media
} // namespace OHOS