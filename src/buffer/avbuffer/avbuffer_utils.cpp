/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "avbuffer_utils.h"
#include "buffer/avbuffer.h"
#include "buffer/avbuffer_common.h"
#include "common/log.h"
#include "cpp_ext/type_cast_ext.h"
#include "meta/any.h"
#include "meta/meta.h"
#include "meta/meta_key.h"
#include "surface_type.h"
#include <unordered_map>

namespace {
using namespace OHOS;
using namespace OHOS::Media;
bool WriteSurfaceBufferConfig(MessageParcel &parcel, const BufferRequestConfig &config)
{
#ifdef MEDIA_OHOS
    return parcel.WriteInt32(config.width) && parcel.WriteInt32(config.height) &&
           parcel.WriteInt32(config.strideAlignment) && parcel.WriteInt32(config.format) &&
           parcel.WriteUint64(config.usage) && parcel.WriteInt32(config.timeout) &&
           parcel.WriteInt32(static_cast<GraphicColorGamut>(config.colorGamut)) &&
           parcel.WriteInt32(static_cast<GraphicTransformType>(config.transform));
#else
    return false;
#endif
}

void ReadSurfaceBufferConfig(MessageParcel &parcel, BufferRequestConfig &config)
{
#ifdef MEDIA_OHOS
    config.width = parcel.ReadInt32();
    config.height = parcel.ReadInt32();
    config.strideAlignment = parcel.ReadInt32();
    config.format = parcel.ReadInt32();
    config.usage = parcel.ReadUint64();
    config.timeout = parcel.ReadInt32();
    config.colorGamut = static_cast<GraphicColorGamut>(parcel.ReadInt32());
    config.transform = static_cast<GraphicTransformType>(parcel.ReadInt32());
#endif
}
} // namespace
namespace OHOS {
namespace Media {
bool MarshallingConfig(MessageParcel &parcel, const AVBufferConfig &config)
{
#ifdef MEDIA_OHOS
    MessageParcel configParcel;
    bool ret = configParcel.WriteInt32(config.size) && configParcel.WriteInt32(config.align) &&
               configParcel.WriteUint8(static_cast<uint8_t>(config.memoryType)) &&
               configParcel.WriteUint8(static_cast<uint8_t>(config.memoryFlag)) &&
               WriteSurfaceBufferConfig(configParcel, config.surfaceBufferConfig) &&
               configParcel.WriteInt32(config.capacity) && configParcel.WriteInt32(config.dmaFd);

    if (ret) {
        ret &= parcel.Append(configParcel);
    }
    return ret;
#else
    return false;
#endif
}

bool UnmarshallingConfig(MessageParcel &parcel, AVBufferConfig &config)
{
#ifdef MEDIA_OHOS
    config.size = parcel.ReadUint32();
    config.align = parcel.ReadInt32();
    config.memoryType = static_cast<MemoryType>(parcel.ReadUint8());
    config.memoryFlag = static_cast<MemoryFlag>(parcel.ReadUint8());
    ReadSurfaceBufferConfig(parcel, config.surfaceBufferConfig);
    config.capacity = parcel.ReadInt32();
    config.dmaFd = parcel.ReadInt32();
#endif
    return true;
}
} // namespace Media
} // namespace OHOS
