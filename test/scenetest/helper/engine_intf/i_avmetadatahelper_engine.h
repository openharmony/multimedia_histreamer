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

#ifndef I_AVMETADATAHELPER_ENGINE_H
#define I_AVMETADATAHELPER_ENGINE_H

#include "avmetadatahelper.h"
#include "i_avmetadatahelper_service.h"

namespace OHOS {
namespace Media {
class IAVMetadataHelperEngine {
public:
    virtual ~IAVMetadataHelperEngine() = default;

    /**
     * Set the media source uri to use. Calling this method before the reset
     * of the methods in this class. This method maybe time consuming.
     * @param uri the URI of input media source.
     * @param usage indicates which scene the avmedatahelper's instance will
     * be used to, see {@link AVMetadataUsage}. If the usage need to be changed,
     * this method must be called again.
     * @return Returns {@link MSERR_OK} if the setting is successful; returns
     * an error code otherwise.
     */
    virtual int32_t SetSource(const std::string &uri, int32_t usage) = 0;

    /**
     * Retrieve the meta data associated with the specified key. This method must be
     * called after the SetSource.
     * @param key One of the constants listed above at the definition of {@link AVMetadataCode}.
     * @return Returns the meta data value associate with the given key code on
     * success; empty string on failure.
     */
    virtual std::string ResolveMetadata(int32_t key) = 0;

    /**
     * Retrieve all meta data within the listed above at the definition of {@link AVMetadataCode}.
     * This method must be called after the SetSource.
     * @return Returns the meta data values on success; empty string on failure.
     */
    virtual std::unordered_map<int32_t, std::string> ResolveMetadata() = 0;

    /**
     * Fetch the album art picture associated with the data source. If there are
     * more than one pictures, the cover image will be returned preferably.
     * @return Returns the a chunk of shared memory containing a picture, which can be
     * null, if such a picture can not be fetched.
     */
    virtual std::shared_ptr<AVSharedMemory> FetchArtPicture() = 0;

    /**
     * Fetch a representative video frame near a given timestamp by considering the given
     * option if possible, and return a video frame with given parameters. This method must be
     * called after the SetSource.
     * @param timeMs The time position in microseconds where the frame will be fetched.
     * When fetching the frame at the given time position, there is no guarantee that
     * the video source has a frame located at the position. When this happens, a frame
     * nearby will be returned. If timeUs is negative, time position and option will ignored,
     * and any frame that the implementation considers as representative may be returned.
     * @param option the hint about how to fetch a frame, see {@link AVMetadataQueryOption}
     * @param param the desired configuration of returned video frame, see {@link OutputConfiguration}.
     * @return Returns a chunk of shared memory containing a scaled video frame, which
     * can be null, if such a frame cannot be fetched.
     */
    virtual std::shared_ptr<AVSharedMemory> FetchFrameAtTime(
        int64_t timeUs, int32_t option, const OutputConfiguration &param) = 0;
};
} // namespace Media
} // namespace OHOS

#endif