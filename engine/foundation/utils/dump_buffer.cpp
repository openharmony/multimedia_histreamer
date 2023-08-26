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
#define HST_LOG_TAG "DumpBuffer"
#define ALL_DUMP_FILES { DEMUXER_INPUT_PEEK, DEMUXER_INPUT_GET, DEMUXER_OUTPUT, DECODER_OUTPUT }

#include "foundation/utils/dump_buffer.h"
#include <cstdio>
#include <cstring>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <dirent.h>
#include "foundation/log.h"
#include "foundation/osal/filesystem/file_system.h"
#include "foundation/osal/utils/util.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
#ifdef _WIN32
#define DUMP_FILE_DIR ""
#endif
// Specify the dump file dir in BUILD.gn
#ifndef DUMP_FILE_DIR
#define DUMP_FILE_DIR "/data/local/tmp"
#endif

std::map<std::string, FILE*> allDumpFileFds;

std::string GetDumpFileDir()
{
    return std::string(DUMP_FILE_DIR);
}

void DumpBufferToFile(const std::string& fileName, const std::shared_ptr<Plugin::Buffer>& buffer)
{
    FALSE_RETURN_MSG(allDumpFileFds[fileName] != nullptr, "fd is null");
    size_t bufferSize = buffer->GetMemory()->GetSize();
    FALSE_RETURN(bufferSize != 0);
    (void)fwrite(reinterpret_cast<const char*>(buffer->GetMemory()->GetReadOnlyData()),
                 1, bufferSize, allDumpFileFds[fileName]);
}

void PrepareDumpDir()
{
    MEDIA_LOG_I("Prepare dumpDir enter.");
    for (auto iter : ALL_DUMP_FILES) {
        std::string filePath = GetDumpFileDir() + "/" + iter;
        MEDIA_LOG_I("Prepare dumpDir: " PUBLIC_LOG_S, filePath.c_str());
        std::string fullPath;
        bool isFileExist = OSAL::ConvertFullPath(filePath, fullPath);
        if (isFileExist) { // 文件存在
            OSAL::FileSystem::ClearFileContent(fullPath);
        }
        allDumpFileFds[iter] = fopen(fullPath.c_str(), "ab+");
        if (allDumpFileFds[iter] == nullptr) {
            MEDIA_LOG_W("Open file(" PUBLIC_LOG_S ") failed(" PUBLIC_LOG_S ").", fullPath.c_str(), strerror(errno));
        }
    }
}

void EndDumpFile()
{
    MEDIA_LOG_I("End dump enter.");
    for (auto iter : ALL_DUMP_FILES) {
        if (allDumpFileFds[iter]) {
            fclose(allDumpFileFds[iter]);
            allDumpFileFds[iter] = nullptr;
        }
    }
}

void DumpBufferToLog(const char* desc, const std::shared_ptr<Plugin::Buffer>& buffer, uint64_t offset, size_t dumpSize)
{
    FALSE_RETURN_MSG(buffer && (!buffer->IsEmpty()),  PUBLIC_LOG_S " Buffer(null or empty)", desc);
    size_t bufferSize = buffer->GetMemory()->GetSize();
    size_t realDumpSize = std::min(dumpSize, bufferSize);
    realDumpSize = std::min(realDumpSize, static_cast<size_t>(DUMP_BUFFER2LOG_SIZE)); // max DUMP_BUFFER2LOG_SIZE bytes
    char tmpStr[2 * DUMP_BUFFER2LOG_SIZE + 10] = {0}; // 字符串长度是打印的buffer长度的2倍 + 1 (字符串结束符)
    char* dstPtr = tmpStr;
    int len;
    const uint8_t* p = buffer->GetMemory()->GetReadOnlyData();
    for (size_t i = 0; i < realDumpSize; i++) {
        len = snprintf_s(dstPtr, 3, 2, "%02x", p[i]); // max write 3 bytes, string len 2
        FALSE_RETURN_MSG(len > 0 && len <= 2, "snprintf_s returned unexpected value " PUBLIC_LOG_D32, len); // max len 2
        dstPtr += len;
    }
    MEDIA_LOG_I(PUBLIC_LOG_S " Buffer(offset " PUBLIC_LOG_D64 ", size " PUBLIC_LOG_ZU ", capacity "
        PUBLIC_LOG_ZU ") : " PUBLIC_LOG_S, desc, offset, bufferSize, buffer->GetMemory()->GetCapacity(), tmpStr);
}
} // Pipeline
} // Media
} // OHOS