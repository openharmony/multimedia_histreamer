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

#include "include/avbuffer_utils.h"
#include "inner_api/common/log.h"
#include "inner_api/cpp_ext/type_cast_ext.h"
#include "inner_api/meta/any.h"
#include "inner_api/meta/meta_key.h"

namespace OHOS {
namespace Media {
bool Marshalling(MessageParcel &parcel, const Meta &meta)
{
    // (void)parcel;
    // (void)meta;
    // return false;
    MessageParcel metaParcel;
    int32_t metaSize = 0;
    bool ret = true;
    for (auto it = meta.begin(); it != meta.end(); ++it) {
        ++metaSize;
        ret &= metaParcel.WriteString(it->first);
        const auto &type = it->second.Type();
        ret &= metaParcel.WriteString(type.name());
        if (IsSameType<int32_t>(it->second)) {
            ret &= metaParcel.WriteInt32(it->second);
        } else if (IsSameType<int64_t>(it->second)) {
            ret &= metaParcel.WriteInt64(it->second);
        } else if (IsSameType<float>(it->second)) {
            ret &= metaParcel.WriteFloat(it->second);
        } else if (Any::IsSameType<double>(it->second)) {
            ret &= metaParcel.WriteDouble(it->second);
        } else if (IsSameType<std::string>(it->second)) {
            ret &= metaParcel.WriteString(it->second);
        } else if (IsSameType<AVBuffer::MetaData>(it->second)) {
            ret &= metaParcel.WriteInt32(static_cast<int32_t>(it->second.size));
            ret &= metaParcel.WriteUnpadBuffer(reinterpret_cast<const void *>(it->second.data), it->second.size);
        } else {
            MEDIA_LOG_E("fail to Marshalling Key: " PUBLIC_LOG_S, it->first.c_str());
            return false;
        }
        MEDIA_LOG_D("success to Marshalling Key: " PUBLIC_LOG_S, it->first.c_str());
    }
    if (ret) {
        ret &= parcel.WriteUint32(metaSize);
        ret &= parcel.Append(metaParcel);
    }
    return ret;
}

bool Unmarshalling(MessageParcel &parcel, Meta &meta)
{
    // (void)parcel;
    // (void)meta;
    // return false;
    uint32_t size = parcel.ReadUint32();
    for (uint32_t index = 0; index < size; index++) {
        std::string key = parcel.ReadString();
        std::string type = parcel.ReadWriteString();
        if (Plugin::IsSameType(type, Any(int32_t).Type())) {
            meta.Set<key>(parcel.ReadInt32());
        } else if (Plugin::IsSameType(type, Any(int64_t).Type())) {
            meta.Set<key>(parcel.ReadInt64());
        } else if (Plugin::IsSameType(type, Any(float).Type())) {
            meta.Set<key>(parcel.ReadFloat());
        } else if (Plugin::IsSameType(type, Any(double).Type())) {
            meta.Set<key>(parcel.ReadDouble());
        } else if (Plugin::IsSameType(type, Any(std::string).Type())) {
            meta.Set<key>(parcel.ReadString());
        } else if (Plugin::IsSameType(type, Any().Type())) {
            int32_t addrSize = parcel.ReadInt32();
            uint8_t *addr = parcel.ReadUnpadBuffer(static_cast<size_t>(addrSize));
            if (addr == nullptr) {
                MEDIA_LOG_E("fail to ReadBuffer Key: %{public}s", key.c_str());
                return false;
            }
            AVBuffer::MetaData val = {.data = addr, .size = addrSize};
            meta.Set<key>(val);
        } else {
            MEDIA_LOG_E("fail to Unmarshalling Key: %{public}s", key.c_str());
            return false;
        }
        MEDIA_LOG_D("success to Unmarshalling Key: %{public}s", key.c_str());
    }
    return true;
}
} // namespace Media
} // namespace OHOS
