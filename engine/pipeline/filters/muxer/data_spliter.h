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

#ifndef HISTREAMER_PIPELINE_DATA_SPLITER_H
#define HISTREAMER_PIPELINE_DATA_SPLITER_H

#include <memory>
#include <utility>
#include "foundation/error_code.h"
#include "utils/type_define.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class DataCollector;

class DataSpliter {
public:
    DataSpliter() = default;
    void SetOutput(std::shared_ptr<DataCollector> output)
    {
        output_ = std::move(output);
    }

    void SetMaxOutputSize(size_t size)
    {
        maxOutputSize_ = size;
    }
    void SetMaxDurationUs(size_t duration)
    {
        maxDurationUs_ = duration;
    }
    virtual ErrorCode PushData(int32_t trackId, std::shared_ptr<AVBuffer> buffer) = 0;

    virtual ErrorCode Resume()
    {
        return ErrorCode::SUCCESS;
    }
    virtual ErrorCode Pause()
    {
        return ErrorCode::SUCCESS;
    }

    virtual ErrorCode SplitMuxBegin() = 0;
    virtual ErrorCode SplitMuxEnd() = 0;
protected:
    static const size_t DEFAULT_MAX_DURATION_US = 60 * 1000 * 1000;
    static const size_t DEFAULT_MAX_OUTPUT_SIZE = 0;
    std::shared_ptr<DataCollector> output_;
    size_t maxOutputSize_{DEFAULT_MAX_OUTPUT_SIZE};
    size_t maxDurationUs_{DEFAULT_MAX_DURATION_US};
};
} // Pipeline
} // Media
} // OHOS
#endif // HISTREAMER_PIPELINE_DATA_SPLITER_H
