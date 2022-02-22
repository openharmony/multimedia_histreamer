/*
 * Copyright (c) 2022-2022Huawei Device Co., Ltd.
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
#include "filesystem.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <log.h>

namespace OHOS {
namespace Media {
namespace OSAL {
bool FileSystem::IsRegularFile(const std::string& path)
{
    struct stat s {};
    return (stat(path.c_str(), &s) == 0) && S_ISREG(s.st_mode);
}

bool FileSystem::IsDirectory(const std::string& path)
{
    struct stat s {};
    return (stat(path.c_str(), &s) == 0) && S_ISDIR(s.st_mode);
}

// judge regular file, directory, symbolic link file path exists
bool FileSystem::IsExists(const std::string& path)
{
    return access(path.c_str(), 0) != -1;
}

bool FileSystem::MakeDir(const std::string& path)
{
#ifdef _WIN32
    if (mkdir(path.c_str()) == -1) {
        MEDIA_LOG_E("Fail to create dir %" PUBLIC_LOG_S " due to %" PUBLIC_LOG_S, path.c_str(), strerror(errno));
        return false;
    }
#else
    oldMask = umask(0);
    if (mkdir(path.c_str(), 755) == -1) { // 755 directory access permissions
        MEDIA_LOG_E("Fail to create dir %" PUBLIC_LOG_S " due to %" PUBLIC_LOG_S, path.c_str(), strerror(errno));
        umask(oldMask);
        return false;
    }
    umask(oldMask);
#endif
    return true;
}

bool FileSystem::MakeMultipleDir(const std::string& path)
{
    FALSE_RETURN_V(!IsExists(path), true);
    // pos is 1, not 0  example: D:/a/b, /local/tmp/
    // Avoid Linux root path before is empty string, which makes it impossible to judge whether the path exists
    int index = path.find("/", 1);
    while (index != std::string::npos) {
        std::string tPath = path.substr(0, index);
        FALSE_RETURN_V(IsExists(tPath) || MakeDir(tPath), false);
        index = path.find("/", index + 1);
    }
    return path[path.size() - 1] == '/' || MakeDir(path);
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS