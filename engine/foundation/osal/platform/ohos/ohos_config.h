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

#ifndef HISTREAMER_FOUNDATION_OSAL_PLATFORM_OHOS_OHOS_CONFIG_H
#define HISTREAMER_FOUNDATION_OSAL_PLATFORM_OHOS_OHOS_CONFIG_H

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

namespace OHOS {
namespace Media {
using ThreadObject = pthread_t;
using MutexObject = pthread_mutex_t;
using SemaphoreObject = sem_t;
using ConditionObject = pthread_cond_t;
}
}
#endif // HISTREAMER_FOUNDATION_OSAL_PLATFORM_OHOS_OHOS_CONFIG_H
