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

#define LOG_TAG "AudioDecoderFilter"

#include "decoder_filter_base.h"
#include "utils/constants.h"
#include "utils/memory_helper.h"
#include "osal/utils/util.h"
#include "factory/filter_factory.h"
#include "common/plugin_utils.h"
#include "plugin/common/plugin_audio_tags.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
DecoderFilterBase::DecoderFilterBase(const std::string &name): FilterBase(name) {}

DecoderFilterBase::~DecoderFilterBase(){}

ErrorCode DecoderFilterBase::SetPluginParameterLocked(Tag tag, const Plugin::ValueType &value)
{
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode DecoderFilterBase::SetParameter(int32_t key, const Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_STATE;
    }
    switch (key) {
        case KEY_CODEC_DRIVE_MODE: {
            if (state_ == FilterState::READY || state_ == FilterState::RUNNING || state_ == FilterState::PAUSED) {
                MEDIA_LOG_W("decoder cannot set parameter KEY_CODEC_DRIVE_MODE in this state");
                return ErrorCode::ERROR_STATE;
            }
            if (value.Type() != typeid(ThreadDrivingMode)) {
                return ErrorCode::ERROR_INVALID_PARAM_TYPE;
            }
            drivingMode_ = Plugin::AnyCast<ThreadDrivingMode>(value);
            return ErrorCode::SUCCESS;
        }
        default: {
            Tag tag = Tag::INVALID;
            if (!TranslateIntoParameter(key, tag)) {
                MEDIA_LOG_I("SetParameter key %d is out of boundary", key);
                return ErrorCode::ERROR_INVALID_PARAM_VALUE;
            }
            RETURN_PLUGIN_NOT_FOUND_IF_NULL(plugin_);
            return SetPluginParameterLocked(tag, value);
        }
    }
}

ErrorCode DecoderFilterBase::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_STATE;
    }
    switch (key) {
        case KEY_CODEC_DRIVE_MODE:
            value = drivingMode_;
            return ErrorCode::SUCCESS;
        default: {
            Tag tag = Tag::INVALID;
            if (!TranslateIntoParameter(key, tag)) {
                MEDIA_LOG_I("GetParameter key %d is out of boundary", key);
                return ErrorCode::ERROR_INVALID_PARAM_VALUE;
            }
            RETURN_PLUGIN_NOT_FOUND_IF_NULL(plugin_);
            return TranslatePluginStatus(plugin_->GetParameter(tag, value));
        }
    }
}
}
}
}
