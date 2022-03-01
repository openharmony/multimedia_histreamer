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

#ifdef MEDIA_OHOS
#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B00
#endif
#ifndef LOG_TAG
#define LOG_TAG "MultiMedia"
#endif
#include "hilog/log.h"
#else
#include "log_adapter.h"
#endif

#ifndef HST_LOG_TAG
#define HST_LOG_TAG "NULL"
#endif

#if defined(MEDIA_OHOS)
#define PUBLIC_LOG "%{public}"
#else
#define PUBLIC_LOG "%"
#endif

#define PUBLIC_LOG_C PUBLIC_LOG "c"
#define PUBLIC_LOG_S PUBLIC_LOG "s"
#define PUBLIC_LOG_D8 PUBLIC_LOG PRId8
#define PUBLIC_LOG_D16 PUBLIC_LOG PRId16
#define PUBLIC_LOG_D32 PUBLIC_LOG PRId32
#define PUBLIC_LOG_D64 PUBLIC_LOG PRId64
#define PUBLIC_LOG_U8 PUBLIC_LOG PRIu8
#define PUBLIC_LOG_U16 PUBLIC_LOG PRIu16
#define PUBLIC_LOG_U32 PUBLIC_LOG PRIu32
#define PUBLIC_LOG_U64 PUBLIC_LOG PRIu64
#define PUBLIC_LOG_F PUBLIC_LOG "f"
#define PUBLIC_LOG_P PUBLIC_LOG "p"
#define PUBLIC_LOG_ZU PUBLIC_LOG "zu"
#define PUBLIC_LOG_HU PUBLIC_LOG "hu"

#ifdef MEDIA_OHOS
#ifndef OHOS_DEBUG
#define HST_DECORATOR_HILOG(op, fmt, args...) \
    do { \
        op(LOG_CORE, PUBLIC_LOG "s:" fmt, HST_LOG_TAG, ##args); \
    } while (0)
#else
#define HST_DECORATOR_HILOG(op, fmt, args...)\
    do { \
        op(LOG_CORE, PUBLIC_LOG "s[" PUBLIC_LOG "d]:" fmt, HST_LOG_TAG, __LINE__, ##args); \
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
        if (ret != ErrorCode::SUCCESS) {                                                                               \
            MEDIA_LOG_E("FAIL_RETURN on ErrorCode(" PUBLIC_LOG "d).", ret);                                           \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FAIL_RET_ERR_CODE_MSG
#define FAIL_RET_ERR_CODE_MSG(loglevel, exec, fmt, args...)                                                            \
    do {                                                                                                               \
        ErrorCode ret = (exec);                                                                                        \
        if (ret != ErrorCode::SUCCESS) {                                                                               \
            loglevel(fmt, ##args);                                                                                     \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FAIL_RET_ERR_CODE_MSG_W
#define FAIL_RET_ERR_CODE_MSG_W(exec, fmt, args...) FAIL_RET_ERR_CODE_MSG(MEDIA_LOG_W, exec, fmt, ##args)
#endif

#ifndef FAIL_RET_ERR_CODE_MSG_E
#define FAIL_RET_ERR_CODE_MSG_E(exec, fmt, args...) FAIL_RET_ERR_CODE_MSG(MEDIA_LOG_E, exec, fmt, ##args)
#endif

#ifndef FAIL_LOG
#define FAIL_LOG(exec)                                                                                                 \
    do {                                                                                                               \
        ErrorCode ret = (exec);                                                                                        \
        if (ret != ErrorCode::SUCCESS) {                                                                               \
            MEDIA_LOG_E("FAIL_LOG on ErrorCode(" PUBLIC_LOG "d).", ret);                                              \
        }                                                                                                              \
    } while (0)
#endif

#ifndef NOK_RETURN
#define NOK_RETURN(exec)                                                                                               \
    do {                                                                                                               \
        Status ret = (exec);                                                                                           \
        if (ret != Status::OK) {                                                                                       \
            MEDIA_LOG_E("NOK_RETURN on Status(" PUBLIC_LOG "d).", ret);                                               \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#endif

#ifndef NOK_LOG
#define NOK_LOG(exec)                                                                                                  \
    do {                                                                                                               \
        Status ret = (exec);                                                                                           \
        if (ret != Status::OK) {                                                                                       \
            MEDIA_LOG_E("NOK_LOG on Status(" PUBLIC_LOG "d).", ret);                                                  \
        }                                                                                                              \
    } while (0)
#endif

// If exec not return zero, then record the error code, especially when call system C function.
#ifndef NZERO_LOG
#define NZERO_LOG(exec)                                                                                                \
    do {                                                                                                               \
        int ret = (exec);                                                                                              \
        if (ret != 0) {                                                                                                \
            MEDIA_LOG_E("NZERO_LOG when call (" #exec "), return " PUBLIC_LOG_D32, ret);                              \
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

#ifndef FALSE_RETURN_W
#define FALSE_RETURN_W(exec)                                                                                           \
    do {                                                                                                               \
        bool ret = (exec);                                                                                             \
        if (!ret) {                                                                                                    \
            MEDIA_LOG_W("FALSE_RETURN " #exec);                                                                        \
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

#ifndef FALSE_RET_MSG
#define FALSE_RET_MSG(exec, fmt, args...)                                                                              \
    do {                                                                                                               \
        bool ret = (exec);                                                                                             \
        if (!ret) {                                                                                                    \
            MEDIA_LOG_E(fmt, ##args);                                                                                  \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FALSE_RET_V_MSG
#define FALSE_RET_V_MSG(loglevel, exec, ret, fmt, args...)                                                             \
    do {                                                                                                               \
        bool value = (exec);                                                                                           \
        if (!value) {                                                                                                  \
            loglevel(fmt, ##args);                                                                                     \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FALSE_RET_V_MSG_W
#define FALSE_RET_V_MSG_W(exec, ret, fmt, args...) FALSE_RET_V_MSG(MEDIA_LOG_W, exec, ret, fmt, ##args)
#endif

#ifndef FALSE_RET_V_MSG_E
#define FALSE_RET_V_MSG_E(exec, ret, fmt, args...) FALSE_RET_V_MSG(MEDIA_LOG_E, exec, ret, fmt, ##args)
#endif

#ifndef FALSE_LOG
#define FALSE_LOG(exec)                                                                                                \
    do {                                                                                                               \
        bool value = (exec);                                                                                           \
        if (!value) {                                                                                                  \
            MEDIA_LOG_E("FALSE_LOG: " #exec);                                                                          \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FALSE_LOG_MSG
#define FALSE_LOG_MSG(loglevel, exec, fmt, args...)                                                                    \
    do {                                                                                                               \
        bool value = (exec);                                                                                           \
        if (!value) {                                                                                                  \
            loglevel(fmt, ##args);                                                                                     \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FALSE_LOG_MSG_W
#define FALSE_LOG_MSG_W(exec, fmt, args...) FALSE_LOG_MSG(MEDIA_LOG_W, exec, fmt, ##args)
#endif


#ifndef FALSE_LOG_MSG_E
#define FALSE_LOG_MSG_E(exec, fmt, args...) FALSE_LOG_MSG(MEDIA_LOG_E, exec, fmt, ##args)
#endif

#ifndef ASSERT_CONDITION
#define ASSERT_CONDITION(exec, msg)                                                                                    \
    do {                                                                                                               \
        bool value = (exec);                                                                                           \
        if (!value) {                                                                                                  \
            MEDIA_LOG_E("ASSERT_CONDITION(msg:" PUBLIC_LOG_S ") " #exec, msg);                                        \
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

#ifndef RETURN_TARGET_ERR_MSG_LOG_IF_FAIL
#define RETURN_TARGET_ERR_MSG_LOG_IF_FAIL(err, returnErr, msg)                                                         \
    do {                                                                                                               \
        if ((err) != ErrorCode::SUCCESS) {                                                                             \
            MEDIA_LOG_E(msg);                                                                                          \
            return returnErr;                                                                                          \
        }                                                                                                              \
    } while (0)
#endif

#ifndef RETURN_ERR_MESSAGE_LOG_IF_FAIL
#define RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, msg) RETURN_TARGET_ERR_MSG_LOG_IF_FAIL(err, err, msg)
#endif

#endif // HISTREAMER_FOUNDATION_LOG_H