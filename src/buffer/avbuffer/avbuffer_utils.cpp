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
#include "inner_api/buffer/avbuffer.h"
#include "inner_api/common/log.h"
#include "inner_api/cpp_ext/type_cast_ext.h"
#include "inner_api/meta/any.h"
#include "inner_api/meta/meta.h"
#include "inner_api/meta/meta_key.h"
#include <unordered_map>

namespace OHOS {
namespace Media {
bool Marshalling(MessageParcel &parcel, const Meta &meta)
{
#ifndef HST_ANY_WITH_NO_RTTI
    (void)parcel;
    (void)meta;
    return false;
#else
    MessageParcel metaParcel;
    int32_t metaSize = 0;
    bool ret = true;
    for (auto it = meta.begin(); it != meta.end(); ++it) {
        ++metaSize;
        ret &= metaParcel.WriteString(it->first);
        const auto &type = it->second.TypeName();
        ret &= metaParcel.WriteString(std::string(type));
        if (Any::IsSameTypeWith<int32_t>(it->second)) {
            ret &= metaParcel.WriteInt32(AnyCast<int32_t>(it->second));
        } else if (Any::IsSameTypeWith<int64_t>(it->second)) {
            ret &= metaParcel.WriteInt64(AnyCast<int64_t>(it->second));
        } else if (Any::IsSameTypeWith<float>(it->second)) {
            ret &= metaParcel.WriteFloat(AnyCast<float>(it->second));
        } else if (Any::IsSameTypeWith<double>(it->second)) {
            ret &= metaParcel.WriteDouble(AnyCast<double>(it->second));
        } else if (Any::IsSameTypeWith<std::string>(it->second)) {
            ret &= metaParcel.WriteString(AnyCast<std::string>(it->second));
        } else if (Any::IsSameTypeWith<AVBuffer::MetaData>(it->second)) {
            ret &= metaParcel.WriteInt32(static_cast<int32_t>(AnyCast<AVBuffer::MetaData>(it->second).size()));
            ret &= metaParcel.WriteUnpadBuffer(
                reinterpret_cast<const void *>(AnyCast<AVBuffer::MetaData>(it->second).data()),
                AnyCast<AVBuffer::MetaData>(it->second).size());
        } else {
            MEDIA_LOG_E("fail to Marshalling Key: " PUBLIC_LOG_S, it->first);
            return false;
        }
        MEDIA_LOG_D("success to Marshalling Key: " PUBLIC_LOG_S, it->first);
    }
    if (ret) {
        ret &= parcel.WriteUint32(metaSize);
        ret &= parcel.Append(metaParcel);
    }
    return ret;
#endif
}

bool Unmarshalling(MessageParcel &parcel, Meta &meta)
{
#ifndef HST_ANY_WITH_NO_RTTI
    (void)parcel;
    (void)meta;
    return false;
#else
    uint32_t size = parcel.ReadUint32();
    for (uint32_t index = 0; index < size; index++) {
        std::string key = parcel.ReadString();
        std::string type = parcel.ReadString();
        if (MakeAny<int32_t>().SameTypeWith(std::string_view(type))) {
            meta.SetData(key.c_str(), parcel.ReadInt32());
        } else if (MakeAny<int64_t>().SameTypeWith(std::string_view(type))) {
            meta.SetData(key.c_str(), parcel.ReadInt64());
        } else if (MakeAny<float>().SameTypeWith(std::string_view(type))) {
            meta.SetData(key.c_str(), parcel.ReadFloat());
        } else if (MakeAny<double>().SameTypeWith(std::string_view(type))) {
            meta.SetData(key.c_str(), parcel.ReadDouble());
        } else if (MakeAny<std::string>().SameTypeWith(std::string_view(type))) {
            meta.SetData(key.c_str(), parcel.ReadString());
        } else if (MakeAny<AVBuffer::MetaData>().SameTypeWith(std::string_view(type))) {
            int32_t addrSize = parcel.ReadInt32();
            auto addr = parcel.ReadUnpadBuffer(static_cast<size_t>(addrSize));
            if (addr == nullptr) {
                MEDIA_LOG_E("fail to ReadBuffer Key: %{public}s", key.c_str());
                return false;
            }
            meta.SetData(key.c_str(), AVBuffer::MetaData(addr, addr + addrSize));
        } else {
            MEDIA_LOG_E("fail to Unmarshalling Key: %{public}s", key.c_str());
            return false;
        }
        MEDIA_LOG_D("success to Unmarshalling Key: %{public}s", key.c_str());
    }
    return true;
#endif
}
} // namespace Media
} // namespace OHOS
