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

inline std::string MediaGetFileName(std::string file)
{
    if (file == "") {
        return "Unknown File";
    }

    return file.substr(file.find_last_of("/\\") + 1);
}

#ifndef LOG_TAG
#define LOG_TAG "NULL"
#endif

// control of logd and logi. If these to function is need, use #define LOG_NDEBUG 0 at header of file
#ifndef LOG_NDEBUG
#define LOG_NDEBUG 1
#endif

#define MEDIA_LOG_MESSAGE(level, msg, ...)                                                                             \
    do {                                                                                                               \
        std::string file(__FILE__);                                                                                    \
        std::string bareFile = MediaGetFileName(file);                                                                 \
        printf("%lu " LOG_TAG " " level " (%s, %d) : Func(%s) " msg "\n", pthread_self(), bareFile.c_str(), __LINE__,  \
               __FUNCTION__, ##__VA_ARGS__);                                                                           \
        fflush(stdout);                                                                                                \
    } while (0)

#if LOG_NDEBUG
#define MEDIA_LOG_I(msg, ...) ((void)0)
#define MEDIA_LOG_D(msg, ...) ((void)0)
#endif

#define MEDIA_LOG_E(msg, ...) MEDIA_LOG_MESSAGE("ERROR", msg, ##__VA_ARGS__)
#define MEDIA_LOG_W(msg, ...) MEDIA_LOG_MESSAGE("WARN", msg, ##__VA_ARGS__)
#if !LOG_NDEBUG
#define MEDIA_LOG_I(msg, ...) MEDIA_LOG_MESSAGE("INFO", msg, ##__VA_ARGS__)
#define MEDIA_LOG_D(msg, ...) MEDIA_LOG_MESSAGE("DEBUG", msg, ##__VA_ARGS__)
#endif

#ifndef FAIL_RETURN
#define FAIL_RETURN(exec)                                                                                              \
    do {                                                                                                               \
        ErrorCode ret = (exec);                                                                                        \
        if (ret != SUCCESS) {                                                                                          \
            MEDIA_LOG_E("FAIL_RETURN on ErrorCode(%d).", ret);                                                         \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#endif

#ifndef FAIL_LOG
#define FAIL_LOG(exec)                                                                                                 \
    do {                                                                                                               \
        ErrorCode ret = (exec);                                                                                        \
        if (ret != SUCCESS) {                                                                                          \
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
    if ((err) != SUCCESS) {                                                                                            \
        MEDIA_LOG_E(msg);                                                                                              \
        return returnErr;                                                                                              \
    }

#define RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, msg) RETURN_TARGET_ERR_MESSAGE_LOG_IF_FAIL(err, err, msg)

#endif // HISTREAMER_FOUNDATION_LOG_H