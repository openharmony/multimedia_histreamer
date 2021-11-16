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

#ifndef HISTREAMER_FOUNDATION_LOG_H
#define HISTREAMER_FOUNDATION_LOG_H

#include <cinttypes>
#include <pthread.h>
#include <string>
#include <vector>

#include "error_code.h"

#ifdef MEDIA_OHOS
#include "hilog/log.h"
#include "media_log.h"
#else
#include "log_adapter.h"
#endif

#ifndef HST_LOG_TAG
#define HST_LOG_TAG "NULL"
#endif

#ifdef MEDIA_OHOS
#ifndef OHOS_DEBUG
#define HST_DECORATOR_HILOG(op, fmt, args...) \
    do { \
        op(LOG_CORE, "%s:" fmt, HST_LOG_TAG, ##args); \
    } while (0)
#else
#define HST_DECORATOR_HILOG(op, fmt, args...)\
    do { \
        op(LOG_CORE, "%s[%d]:" fmt, HST_LOG_TAG, __LINE__, ##args); \
    } while (0)
#endif

#define MEDIA_LOG_D(fmt, ...) HST_DECORATOR_HILOG(HILOG_DEBUG, fmt, ##__VA_ARGS__)
#define MEDIA_LOG_I(fmt, ...) HST_DECORATOR_HILOG(HILOG_INFO, fmt, ##__VA_ARGS__)
#define MEDIA_LOG_W(fmt, ...) HST_DECORATOR_HILOG(HILOG_WARN, fmt, ##__VA_ARGS__)
#define MEDIA_LOG_E(fmt, ...) HST_DECORATOR_HILOG(HILOG_ERROR, fmt, ##__VA_ARGS__)
#define MEDIA_LOG_F(fmt, ...) HST_DECORATOR_HILOG(HILOG_FATAL, fmt, ##__VA_ARGS__)
#endif


// Control the MEDIA_LOG_D.
// If MEDIA_LOG_D is needed, #define MEDIA_LOG_DEBUG 1 at the beginning of the cpp file.
#ifndef MEDIA_LOG_DEBUG
#define MEDIA_LOG_DEBUG 0
#endif

#if !MEDIA_LOG_DEBUG
#undef MEDIA_LOG_D
#define MEDIA_LOG_D(msg, ...) ((void)0)
#endif

#ifndef FAIL_RETURN
#define FAIL_RETURN(exec)                                                                                              \
    do {                                                                                                               \
        ErrorCode ret = (exec);                                                                                        \
        if (ret != ErrorCode::SUCCESS) {                                                                                          \
            MEDIA_LOG_E("FAIL_RETURN on ErrorCode(%d).", ret);                                                         \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FAIL_LOG
#define FAIL_LOG(exec)                                                                                                 \
    do {                                                                                                               \
        ErrorCode ret = (exec);                                                                                        \
        if (ret != ErrorCode::SUCCESS) {                                                                                          \
            MEDIA_LOG_E("FAIL_LOG on ErrorCode(%d).", ret);                                                            \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FALSE_RETURN
#define FALSE_RETURN(exec)                                                                                             \
    do {                                                                                                               \
        bool ret = (exec);                                                                                             \
        if (!ret) {                                                                                                    \
            MEDIA_LOG_E("FALSE_RETURN " #exec);                                                                        \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FALSE_RETURN_V
#define FALSE_RETURN_V(exec, ret)                                                                                      \
    do {                                                                                                               \
        bool value = (exec);                                                                                           \
        if (!value) {                                                                                                  \
            MEDIA_LOG_E("FALSE_RETURN_V " #exec);                                                                      \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#endif

#ifndef ASSERT_CONDITION
#define ASSERT_CONDITION(exec, msg)                                                                                    \
    do {                                                                                                               \
        bool value = (exec);                                                                                           \
        if (!value) {                                                                                                  \
            MEDIA_LOG_E("ASSERT_CONDITION(msg:%s) " #exec, msg);                                                       \
        }                                                                                                              \
    } while (0)
#endif

#ifndef RETURN_ERROR_IF_NULL
#define RETURN_ERROR_IF_NULL(ptr)                                                                                      \
    do {                                                                                                               \
        if ((ptr) == nullptr) {                                                                                        \
            MEDIA_LOG_E("Null pointer error: " #ptr);                                                                  \
            return ErrorCode::NULL_POINTER_ERROR;                                                                      \
        }                                                                                                              \
    } while (0)
#endif

#define RETURN_TARGET_ERR_MESSAGE_LOG_IF_FAIL(err, returnErr, msg)                                                     \
    if ((err) != ErrorCode::SUCCESS) {                                                                                            \
        MEDIA_LOG_E(msg);                                                                                              \
        return returnErr;                                                                                              \
    }

#define RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, msg) RETURN_TARGET_ERR_MESSAGE_LOG_IF_FAIL(err, err, msg)

#endif // HISTREAMER_FOUNDATION_LOG_H