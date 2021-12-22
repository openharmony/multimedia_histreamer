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
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("SetParameter key %d is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return SetPluginParameterLocked(tag, value);
}

ErrorCode DecoderFilterBase::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("GetParameter key %d is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->GetParameter(tag, value));
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
