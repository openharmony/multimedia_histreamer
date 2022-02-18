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

#include <cstdio>
#include <direct.h>
#include <dirent.h>
#include "dump_buffer.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Pipeline {

// Specify the dump file dir in BUILD.gn
#ifndef DUMP_FILE_DIR
#define DUMP_FILE_DIR ""
#endif

std::string GetDumpFileDir()
{
    auto rootDir = std::string(DUMP_FILE_DIR);
    return rootDir.empty() ? "histreamer_dump_files/" : rootDir + "/histreamer_dump_files/";
}

void DumpBufferToFile(const std::string& fileName, const std::shared_ptr<Plugin::Buffer>& buffer)
{
    size_t bufferSize = buffer->GetMemory()->GetSize();
    FALSE_RETURN(bufferSize != 0);

    FILE* filePtr = nullptr;
    std::string filePath = GetDumpFileDir() + fileName;
    errno_t error = fopen_s(&filePtr, filePath.c_str(), "ab+");
    FALSE_RET_MSG(error == 0, "Open file(%" PUBLIC_LOG_S ") failed(%" PUBLIC_LOG_D32 ").", filePath.c_str(), error);
    fwrite(reinterpret_cast<const char*>(buffer->GetMemory()->GetReadOnlyData()), bufferSize, 1, filePtr);
    fclose(filePtr);
}

void RemoveFilesInDir(const std::string& path) {
    DIR *directory;
    struct dirent *info;
    if ((directory = opendir(path.c_str())) != nullptr) {
        while ((info = readdir(directory)) != nullptr) {
            if (strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0) {
                continue;
            }
            std::string fullPath = path + info->d_name;
            remove(fullPath.c_str());
        }
        closedir(directory);
    }
}

void PrepareDumpDir()
{
    std::string dumpDir = GetDumpFileDir();
    const char* fileDir = dumpDir.c_str();
    if (access(fileDir, 0) == 0) { // 目录存在
        RemoveFilesInDir(fileDir);
    } else {
        _mkdir(fileDir);
    }
}
} // Pipeline
} // Media
} // OHOS