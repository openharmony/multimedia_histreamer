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

#ifndef HISTREAMER_PLUGIN_MEDIA_SOURCE_H
#define HISTREAMER_PLUGIN_MEDIA_SOURCE_H

#include <map>
#include <memory>
#ifndef OHOS_LITE
#include "common/media_data_source.h"
#endif
#include "meta/media_types.h"

namespace OHOS {
namespace Media {
namespace Plugins {
/// End of Stream Buffer Flag
#define BUFFER_FLAG_EOS 0x00000001
/// Video Key Frame Flag
#define BUFFER_FLAG_KEY_FRAME 0x00000002
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

class MediaSource {
public:
    /// Construct an a specified URI.
    explicit MediaSource(std::string uri);

#ifndef OHOS_LITE
    explicit MediaSource(std::shared_ptr<IMediaDataSource> dataSrc);
#endif

    MediaSource(std::string uri, std::map<std::string, std::string> header);

    /// Destructor
    virtual ~MediaSource() = default;

    /// Obtains the source type.
    SourceType GetSourceType() const;

    /// Obtains the media source URI.
    const std::string &GetSourceUri() const;

    const std::map<std::string, std::string> &GetSourceHeader() const;

    //std::shared_ptr<DataConsumer> GetDataConsumer() const;
#ifndef OHOS_LITE
    std::shared_ptr<IMediaDataSource> GetDataSrc() const;
#endif
private:
    std::string uri_ {};
    SourceType type_ {};
    std::map<std::string, std::string> header_ {};
    //std::shared_ptr<DataConsumer> dataConsumer_ {};
#ifndef OHOS_LITE
    std::shared_ptr<IMediaDataSource> dataSrc_ {};
#endif
};
} // namespace Plugins
} // namespace Media
} // namespace OHOS
#endif