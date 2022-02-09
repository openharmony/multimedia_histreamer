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
#include "utils/memory_helper.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
CodecFilterBase::CodecFilterBase(const std::string &name): FilterBase(name) {}

CodecFilterBase::~CodecFilterBase()= default;

ErrorCode CodecFilterBase::ConfigureWithMetaLocked(const std::shared_ptr<const Plugin::Meta> &meta)
{
    auto parameterMap = PluginParameterTable::FindAllowedParameterMap(filterType_);
    for (const auto& keyPair : parameterMap) {
        auto outValPtr = meta->GetData(static_cast<Plugin::MetaID>(keyPair.first));
        if (outValPtr &&
            (keyPair.second.second & PARAM_SET) &&
            keyPair.second.first(keyPair.first, *outValPtr)) {
            SetPluginParameterLocked(keyPair.first, *outValPtr);
        } else {
            if (g_tagInfoMap.count(keyPair.first) == 0) {
                MEDIA_LOG_W("tag %" PUBLIC_LOG_D32 " is not in map, may be update it?", keyPair.first);
            } else {
                MEDIA_LOG_W("parameter %" PUBLIC_LOG_S " in meta is not found or type mismatch",
                            std::get<0>(g_tagInfoMap.at(keyPair.first)));
            }
        }
    }
    return ErrorCode::SUCCESS;
}

ErrorCode CodecFilterBase::SetPluginParameterLocked(Tag tag, const Plugin::ValueType &value)
{
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode CodecFilterBase::SetParameter(int32_t key, const Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("SetParameter key %" PUBLIC_LOG "d is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return SetPluginParameterLocked(tag, value);
}

ErrorCode CodecFilterBase::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("GetParameter key %" PUBLIC_LOG "d is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->GetParameter(tag, value));
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
