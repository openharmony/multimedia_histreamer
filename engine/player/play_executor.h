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

#ifndef MEDIA_PIPELINE_PLAYER_EXECUTOR_H
#define MEDIA_PIPELINE_PLAYER_EXECUTOR_H

#include <memory>
#include "foundation/error_code.h"
#include "source.h"

namespace OHOS {
namespace Media {
using MediaSource = OHOS::Media::Source;

class PlayExecutor {
public:
    virtual ~PlayExecutor()
    {
    }

    virtual ErrorCode PrepareFilters()
    {
        return SUCCESS;
    }

    virtual ErrorCode DoSetSource(const std::shared_ptr<MediaSource>& source) const
    {
        (void)source;
        return SUCCESS;
    }

    virtual ErrorCode DoPlay()
    {
        return SUCCESS;
    }

    virtual ErrorCode DoPause()
    {
        return SUCCESS;
    }

    virtual ErrorCode DoResume()
    {
        return SUCCESS;
    }

    virtual ErrorCode DoStop()
    {
        return SUCCESS;
    }

    virtual ErrorCode DoSeek(int64_t msec)
    {
        (void)msec;
        return SUCCESS;
    }

    virtual ErrorCode DoOnReady()
    {
        return SUCCESS;
    }

    virtual ErrorCode DoOnComplete()
    {
        return SUCCESS;
    }

    virtual ErrorCode DoOnError(ErrorCode errorCode)
    {
        (void)errorCode;
        return SUCCESS;
    }
};
} // namespace Media
} // namespace OHOS

#endif
