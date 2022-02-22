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

#define HST_LOG_TAG "CodecFilterBase"

#include "codec_filter_base.h"

#include "pipeline/core/plugin_attr_desc.h"
#include "pipeline/filters/common/plugin_settings.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
CodecFilterBase::CodecFilterBase(const std::string &name): FilterBase(name) {}

CodecFilterBase::~CodecFilterBase() = default;

ErrorCode CodecFilterBase::ConfigureWithMetaLocked(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto parameterMap = PluginParameterTable::FindAllowedParameterMap(filterType_);
    for (const auto& keyPair : parameterMap) {
        if ((keyPair.second.second & PARAM_SET) == 0) {
            continue;
        }
        auto outValPtr = meta->GetData(static_cast<Plugin::MetaID>(keyPair.first));
        if (outValPtr && keyPair.second.first(keyPair.first, *outValPtr)) {
            SetPluginParameterLocked(keyPair.first, *outValPtr);
        } else {
            if (!HasTagInfo(keyPair.first)) {
                MEDIA_LOG_W("tag %" PUBLIC_LOG_D32 " is not in map, may be update it?", keyPair.first);
            } else {
                MEDIA_LOG_W("parameter %" PUBLIC_LOG_S " in meta is not found or type mismatch",
                            GetTagStrName(keyPair.first));
            }
        }
    }
    return ErrorCode::SUCCESS;
}

ErrorCode CodecFilterBase::UpdateMetaAccordingToPlugin(Plugin::Meta& meta)
{
    auto parameterMap = PluginParameterTable::FindAllowedParameterMap(filterType_);
    for (const auto& keyPair : parameterMap) {
        if ((keyPair.second.second & PARAM_GET) == 0) {
            continue;
        }
        Plugin::ValueType tmpVal;
        auto ret = TranslatePluginStatus(plugin_->GetParameter(keyPair.first, tmpVal));
        if (ret != ErrorCode::SUCCESS) {
            if (HasTagInfo(keyPair.first)) {
                MEDIA_LOG_I("GetParameter %" PUBLIC_LOG_S " from plugin %" PUBLIC_LOG_S "failed with code %"
                    PUBLIC_LOG_D32, GetTagStrName(keyPair.first), pluginInfo_->name.c_str(), ret);
            } else {
                MEDIA_LOG_I("Tag %" PUBLIC_LOG_D32 " is not is map, may be update it?", keyPair.first);
                MEDIA_LOG_I("GetParameter %" PUBLIC_LOG_D32 " from plugin %" PUBLIC_LOG_S " failed with code %"
                    PUBLIC_LOG_D32, keyPair.first, pluginInfo_->name.c_str(), ret);
            }
            continue;
        }
        if (!keyPair.second.first(keyPair.first, tmpVal)) {
            if (HasTagInfo(keyPair.first)) {
                MEDIA_LOG_I("Type of Tag %" PUBLIC_LOG_S " should be %" PUBLIC_LOG_S,
                            GetTagStrName(keyPair.first), std::get<2>(g_tagInfoMap.at(keyPair.first)));
            } else {
                MEDIA_LOG_I("Tag %" PUBLIC_LOG_D32 " is not is map, may be update it?", keyPair.first);
                MEDIA_LOG_I("Type of Tag %" PUBLIC_LOG_D32 "mismatch", keyPair.first);
            }
            continue;
        }
        meta.SetData(static_cast<Plugin::MetaID>(keyPair.first), tmpVal);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode CodecFilterBase::SetPluginParameterLocked(Tag tag, const Plugin::ValueType& value)
{
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode CodecFilterBase::SetParameter(int32_t key, const Plugin::Any& inVal)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    FALSE_RET_V_MSG_E(TranslateIntoParameter(key, tag), ErrorCode::ERROR_INVALID_PARAMETER_VALUE,
                      "key %" PUBLIC_LOG_D32 " is out of boundary", key);
    RETURN_AGAIN_IF_NULL(plugin_);
    return SetPluginParameterLocked(tag, inVal);
}

ErrorCode CodecFilterBase::GetParameter(int32_t key, Plugin::Any& outVal)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    FALSE_RET_V_MSG_E(TranslateIntoParameter(key, tag), ErrorCode::ERROR_INVALID_PARAMETER_VALUE,
                      "key %" PUBLIC_LOG_D32 " is out of boundary", key);
    RETURN_AGAIN_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->GetParameter(tag, outVal));
}

void CodecFilterBase::OnInputBufferDone(const std::shared_ptr<Plugin::Buffer>& input)
{
    MEDIA_LOG_D("CodecFilterBase::OnInputBufferDone");
}

void CodecFilterBase::OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer>& output)
{
    MEDIA_LOG_D("CodecFilterBase::OnOutputBufferDone");
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
