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

#define HST_LOG_TAG "DecoderFilterBase"

#include "decoder_filter_base.h"

#include "pipeline/filters/common/plugin_settings.h"
#include "utils/memory_helper.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
DecoderFilterBase::DecoderFilterBase(const std::string &name): FilterBase(name) {}

DecoderFilterBase::~DecoderFilterBase()= default;

ErrorCode DecoderFilterBase::ConfigureWithMetaLocked(const std::shared_ptr<const Plugin::Meta> &meta)
{
    auto parameterMap = PluginParameterTable::FindAllowedParameterMap(filterType_);
    for (const auto& keyPair : parameterMap) {
        Plugin::ValueType outValue;
        if (meta->GetData(static_cast<Plugin::MetaID>(keyPair.first), outValue) &&
            keyPair.second.second(outValue)) {
            SetPluginParameterLocked(keyPair.first, outValue);
        } else {
            MEDIA_LOG_W("parameter %s in meta is not found or type mismatch", keyPair.second.first.c_str());
        }
    }
    return ErrorCode::SUCCESS;
}

ErrorCode DecoderFilterBase::SetPluginParameterLocked(Tag tag, const Plugin::ValueType &value)
{
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode DecoderFilterBase::SetParameter(int32_t key, const Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    switch (key) {
        case KEY_CODEC_DRIVE_MODE: {
            if (state_ == FilterState::READY || state_ == FilterState::RUNNING || state_ == FilterState::PAUSED) {
                MEDIA_LOG_W("decoder cannot set parameter KEY_CODEC_DRIVE_MODE in this state");
                return ErrorCode::ERROR_AGAIN;
            }
            if (value.Type() != typeid(ThreadDrivingMode)) {
                return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
            }
            drivingMode_ = Plugin::AnyCast<ThreadDrivingMode>(value);
            return ErrorCode::SUCCESS;
        }
        default: {
            Tag tag = Tag::INVALID;
            if (!TranslateIntoParameter(key, tag)) {
                MEDIA_LOG_I("SetParameter key %d is out of boundary", key);
                return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
            }
            RETURN_AGAIN_IF_NULL(plugin_);
            return SetPluginParameterLocked(tag, value);
        }
    }
}

ErrorCode DecoderFilterBase::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    switch (key) {
        case KEY_CODEC_DRIVE_MODE:
            value = drivingMode_;
            return ErrorCode::SUCCESS;
        default: {
            Tag tag = Tag::INVALID;
            if (!TranslateIntoParameter(key, tag)) {
                MEDIA_LOG_I("GetParameter key %d is out of boundary", key);
                return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
            }
            RETURN_AGAIN_IF_NULL(plugin_);
            return TranslatePluginStatus(plugin_->GetParameter(tag, value));
        }
    }
}
bool DecoderFilterBase::UpdateAndInitPluginByInfo(const std::shared_ptr<Plugin::PluginInfo>& selectedPluginInfo)
{
    if (selectedPluginInfo == nullptr) {
        MEDIA_LOG_W("no available info to update plugin");
        return false;
    }
    if (plugin_ != nullptr){
        if (targetPluginInfo_ != nullptr && targetPluginInfo_->name == selectedPluginInfo->name) {
            if (plugin_->Reset() == Plugin::Status::OK) {
                return true;
            }
            MEDIA_LOG_W("reuse previous plugin %s failed, will create new plugin", targetPluginInfo_->name.c_str());
        }
        plugin_->Deinit();
    }

    plugin_ = Plugin::PluginManager::Instance().CreateCodecPlugin(selectedPluginInfo->name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("cannot create plugin %s", selectedPluginInfo->name.c_str());
        return false;
    }
    auto err = TranslatePluginStatus(plugin_->Init());
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("decoder plugin init error");
        return false;
    }
    targetPluginInfo_ = selectedPluginInfo;
    return true;
}

}
}
}
