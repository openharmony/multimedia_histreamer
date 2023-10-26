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
#include "inner_api/meta/any.h"
#include "inner_api/meta/meta_key.h"

namespace OHOS {
namespace Media {
bool Marshalling(MessageParcel &parcel, const Meta &meta)
{
    (void)parcel;
    (void)meta;
    return false;
    // MessageParcel metaParcel;
    // int32_t metaSize = 0;
    // bool ret = metaParcel.WriteUint32(meta.size());
    // for (auto it = meta.begin(); it != meta.end(); ++it) {
    //     ++metaSize;
    //     ret &= metaParcel.WriteString(it->first);
    //     const auto &type = it->second.Type();
    //     ret &= metaParcel.WriteString(type.name());
    //     if (type == typeid(int32_t)) {
    //         ret &= metaParcel.WriteInt32(it->second);
    //     } else if (type == typeid(int64_t)) {
    //         ret &= metaParcel.WriteInt64(it->second);
    //     } else if (type == typeid(float)) {
    //         ret &= metaParcel.WriteFloat(it->second);
    //     } else if (type == typeid(double)) {
    //         ret &= metaParcel.WriteDouble(it->second);
    //     } else if (type == typeid(std::string)) {
    //         ret &= metaParcel.WriteString(it->second);
    //     // } else if (type == typeid(void *)) {
    //     //     ret &= metaParcel.WriteInt32(static_cast<int32_t>(it->second.size));
    //     //     ret &= metaParcel.WriteUnpadBuffer(reinterpret_cast<const void *>(it->second.addr), it->second.size);
    //     } else {
    //         MEDIA_LOG_E("fail to Marshalling Key: " PUBLIC_LOG_S, it->first.c_str());
    //         return false;
    //     }
    //     MEDIA_LOG_D("success to Marshalling Key: " PUBLIC_LOG_S, it->first.c_str());
    // }
    // if (ret) {
    //     parcel.Append(metaParcel);
    // }
    // return ret;

}

bool Unmarshalling(MessageParcel &parcel, Meta &meta)
{
    (void)parcel;
    (void)meta;
    return false;
    // uint32_t size = parcel.ReadUint32();
    // for (uint32_t index = 0; index < size; index++) {
    //     std::string key = parcel.ReadString();
    //     std::string valType = parcel.ReadWriteString();
    //     if (valType == typeid(int32_t).name()) {
    //         meta.Get<key>(parcel.ReadInt32());
    //     } else if (valType == typeid(int64_t).name()) {
    //         meta.Get<key>(parcel.ReadInt64());
    //     } else if (valType == typeid(float).name()) {
    //         meta.Get<key>(parcel.ReadFloat());
    //     } else if (valType == typeid(double).name()) {
    //         meta.Get<key>(parcel.ReadDouble());
    //     } else if (valType == typeid(std::string).name()) {
    //         meta.Get<key>(parcel.ReadString());
    //     // } else if (valType == typeid(void *).name()) {
    //     //     auto addrSize = parcel.ReadInt32();
    //     //     auto addr = parcel.ReadUnpadBuffer(static_cast<size_t>(addrSize));
    //     //     if (addr == nullptr) {
    //     //         MEDIA_LOG_E("fail to ReadBuffer Key: %{public}s", key.c_str());
    //     //         return false;
    //     //     }
    //     //     (void)meta.PutBuffer(key, addr, static_cast<size_t>(addrSize));
    //     } else {
    //         MEDIA_LOG_E("fail to Unmarshalling Key: %{public}s", key.c_str());
    //         return false;
    //     }
    //     MEDIA_LOG_D("success to Unmarshalling Key: %{public}s", key.c_str());
    // }
    // return true;
}
} // namespace Media
} // namespace OHOS
