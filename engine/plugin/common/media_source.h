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

#ifndef HISTREAMER_PLUGIN_MEDIA_SOURCE_H
#define HISTREAMER_PLUGIN_MEDIA_SOURCE_H

#include <map>
#include <memory>
#include "plugin_buffer.h"
#include "plugin_types.h" // NOLINT: used it

namespace OHOS {
namespace Media {
namespace Plugin {
/**
 * @brief Unified enumerates media source types.
 *
 * @since 1.0
 * @version 1.0
 */
enum class SourceType : int32_t {
    /** Local file path or network address */
    SOURCE_TYPE_URI = 0,
    /** Local file descriptor */
    SOURCE_TYPE_FD,
    /** Stream data */
    SOURCE_TYPE_STREAM,
};

class DataStream {
    /**
     * @brief Read data from data source.
     *
     * The function is valid only after RUNNING state.
     *
     * @param buffer Storage of the read data
     * @param expectedLen   Expected data size to be read
     * @return  Execution status return
     *  @retval OK: Plugin reset succeeded.
     *  @retval ERROR_NOT_ENOUGH_DATA: Data not enough
     *  @retval END_OF_STREAM: End of stream
     */
    virtual Status Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen) = 0;

    /**
     * @brief Get data source size.
     *
     * The function is valid only after INITIALIZED state.
     *
     * @param size data source size.
     * @return  Execution status return.
     *  @retval OK: Plugin reset succeeded.
     */
    virtual Status GetSize(size_t& size) = 0;
};

class MediaSource {
public:
    /// Construct an a specified URI.
    explicit MediaSource(std::string uri);

    explicit MediaSource(std::shared_ptr<DataStream> dataStream);

    MediaSource(std::string uri, std::map<std::string, std::string> header);

    /// Destructor
    virtual ~MediaSource() = default;

    /// Obtains the source type.
    SourceType GetSourceType() const;

    /// Obtains the media source URI.
    const std::string &GetSourceUri() const;

    const std::map<std::string, std::string> &GetSourceHeader() const;

    std::shared_ptr<DataStream> GetDataStream() const;

private:
    std::string uri_ {};
    SourceType type_ {};
    std::map<std::string, std::string> header_ {};
    std::shared_ptr<DataStream> dataStream_ {};
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif