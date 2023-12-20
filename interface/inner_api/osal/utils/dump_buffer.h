/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PIPELINE_DUMP_BUFFER_H
#define HISTREAMER_PIPELINE_DUMP_BUFFER_H

#define DEMUXER_INPUT_PEEK "hst_dump_demuxer_input_peek.data"
#define DEMUXER_INPUT_GET "hst_dump_demuxer_input_get.data"
#define DEMUXER_OUTPUT "hst_dump_demuxer_output.data"
#define DECODER_OUTPUT "hst_dump_decoder_output.data"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include "plugin/plugin_buffer.h"

namespace OHOS {
namespace Media {
// Modify DUMP_BUFFER2FILE_ENABLE to 1 to dump buffer to file.
#ifndef DUMP_BUFFER2FILE_ENABLE
#define DUMP_BUFFER2FILE_ENABLE 0
#endif

// Modify DUMP_BUFFER2LOG_ENABLE to 1 to dump buffer first 10 bytes to log.
#ifndef DUMP_BUFFER2LOG_ENABLE
#define DUMP_BUFFER2LOG_ENABLE 0
#endif

#if DUMP_BUFFER2FILE_ENABLE
#define DUMP_BUFFER2FILE(fileName, buffer) OHOS::Media::Pipeline::DumpBufferToFile(fileName, buffer)
#define DUMP_BUFFER2FILE_PREPARE() OHOS::Media::Pipeline::PrepareDumpDir()
#define DUMP_BUFFER2FILE_END() OHOS::Media::Pipeline::EndDumpFile()
#else
#define DUMP_BUFFER2FILE(fileName, buffer)
#define DUMP_BUFFER2FILE_PREPARE()
#define DUMP_BUFFER2FILE_END()
#endif

#define DUMP_BUFFER2LOG_SIZE 10 // Dump first 10 bytes of buffer.
#if DUMP_BUFFER2LOG_ENABLE
#define DUMP_BUFFER2LOG(desc, buffer, offset) \
    OHOS::Media::Pipeline::DumpBufferToLog(desc, buffer, offset, DUMP_BUFFER2LOG_SIZE)
#else
#define DUMP_BUFFER2LOG(desc, buffer, offset)
#endif

void DumpBufferToFile(const std::string& fileName, const std::shared_ptr<Plugins::Buffer>& buffer);
void PrepareDumpDir();
void EndDumpFile();
void DumpBufferToLog(const char* desc, const std::shared_ptr<Plugins::Buffer>& buffer, uint64_t offset,
    size_t dumpSize);
} // Media
} // OHOS
#endif // HISTREAMER_PIPELINE_DUMP_BUFFER_H
