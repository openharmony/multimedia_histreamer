/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef MEDIA_DATA_SOURCE_H_
#define MEDIA_DATA_SOURCE_H_

#include "buffer/avsharedmemory.h"

namespace OHOS {
namespace Media {
/**
 * @brief Use with IMediaDataSource::ReadAt.
 */
enum MediaDataSourceError : int32_t {
    /**
     * use with ReadAt.the resource is cut off and player will end.
     * And the player will complete buffers and return an error.
     */
    SOURCE_ERROR_IO = -2,
    /* use with ReadAt.the resource is eos and player will complete. */
    SOURCE_ERROR_EOF = -1,
};

/**
 * @brief the mediaDataSource instance need set to player.
 *
 */
class IMediaDataSource {
public:
    virtual ~IMediaDataSource() = default;

    /**
     * @brief Player use ReadAt to tell user the desired file position and length.(length is number of Bytes)
     * Then usr filled the mem, and return the actual length of mem.
     * @param mem The stream mem need to fill. see avsharedmemory.h.
     * @param length The stream length player want to get.
     * @param pos The stream pos player want get start.
     * The length of the filled memory must match the actual length returned.
     * @return The actual length of stream mem filled, if failed or no mem return MediaDataSourceError.
     */
    virtual int32_t ReadAt(const std::shared_ptr<AVSharedMemory> &mem, uint32_t length, int64_t pos = -1) = 0;

    /**
     * @brief Get the total size of the stream.
     * If the user does not know the length of the stream, size should be assigned -1,
     * player will use the datasource not seekable.
     * @param size Total size of the stream. If no size set -1.
     * @return MSERR_OK if ok; others if failed. see media_errors.h
     */
    virtual int32_t GetSize(int64_t &size) = 0;

    // This interface has been deprecated
    virtual int32_t ReadAt(int64_t pos, uint32_t length, const std::shared_ptr<AVSharedMemory> &mem) = 0;
    // This interface has been deprecated
    virtual int32_t ReadAt(uint32_t length, const std::shared_ptr<AVSharedMemory> &mem) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // MEDIA_DATA_SOURCE_H_