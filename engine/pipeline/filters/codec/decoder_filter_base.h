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

#ifndef HISTREAMER_PIPELINE_FILTER_DECODER_BASE_H
#define HISTREAMER_PIPELINE_FILTER_DECODER_BASE_H

#include <string>

#include "utils/blocking_queue.h"
#include "utils/buffer_pool.h"
#include "utils/type_define.h"
#include "utils/utils.h"
#include "foundation/osal/thread/task.h"
#include "pipeline/core/filter_base.h"
#include "plugin/common/plugin_tags.h"
#include "plugin/core/codec.h"
#include "plugin/core/plugin_info.h"
#include "plugin/core/plugin_meta.h"
#include "common/plugin_utils.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class DecoderFilterBase : public FilterBase {
public:
    explicit DecoderFilterBase(const std::string &name);
    ~DecoderFilterBase() override;

    ErrorCode SetParameter(int32_t key, const Plugin::Any &value) override;

    ErrorCode GetParameter(int32_t key, Plugin::Any &value) override;

protected:
    ErrorCode SetPluginParameterLocked(Tag tag, const Plugin::ValueType &value);

    template<typename T>
    ErrorCode GetPluginParameterLocked(Tag tag, T& value)
    {
        Plugin::Any tmp;
        auto err = TranslatePluginStatus(plugin_->GetParameter(tag, tmp));
        if (err == ErrorCode::SUCCESS && tmp.Type() == typeid(T)) {
            value = Plugin::AnyCast<T>(tmp);
        }
        return err;
    }

    ThreadDrivingMode drivingMode_ {ThreadDrivingMode::ASYNC};

    // plugin
    std::shared_ptr<Plugin::Codec> plugin_ {nullptr};
    std::shared_ptr<Plugin::PluginInfo> targetPluginInfo_ {nullptr};
};
}
}
}
#endif // HISTREAMER_PIPELINE_FILTER_DECODER_BASE_H
