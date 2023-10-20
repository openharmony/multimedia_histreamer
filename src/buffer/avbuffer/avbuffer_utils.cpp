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
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVBufferUtils"};
} // namespace
namespace OHOS {
namespace MediaAVCodec {
bool Marshalling(MessageParcel &parcel, const Format &format)
{
    auto dataMap = format.GetFormatMap();
    (void)parcel.WriteUint32(dataMap.size());
    for (auto it = dataMap.begin(); it != dataMap.end(); ++it) {
        (void)parcel.WriteString(it->first);
        (void)parcel.WriteUint32(it->second.type);
        switch (it->second.type) {
            case FORMAT_TYPE_INT32:
                (void)parcel.WriteInt32(it->second.val.int32Val);
                break;
            case FORMAT_TYPE_INT64:
                (void)parcel.WriteInt64(it->second.val.int64Val);
                break;
            case FORMAT_TYPE_FLOAT:
                (void)parcel.WriteFloat(it->second.val.floatVal);
                break;
            case FORMAT_TYPE_DOUBLE:
                (void)parcel.WriteDouble(it->second.val.doubleVal);
                break;
            case FORMAT_TYPE_STRING:
                (void)parcel.WriteString(it->second.stringVal);
                break;
            case FORMAT_TYPE_ADDR:
                (void)parcel.WriteInt32(static_cast<int32_t>(it->second.size));
                (void)parcel.WriteUnpadBuffer(reinterpret_cast<const void *>(it->second.addr), it->second.size);
                break;
            default:
                AVCODEC_LOGE("fail to Marshalling Key: %{public}s", it->first.c_str());
                return false;
        }
        AVCODEC_LOGD("success to Marshalling Key: %{public}s", it->first.c_str());
    }

    return true;
}

bool Unmarshalling(MessageParcel &parcel, Format &format)
{
    uint32_t size = parcel.ReadUint32();
    for (uint32_t index = 0; index < size; index++) {
        std::string key = parcel.ReadString();
        uint32_t valType = parcel.ReadUint32();
        switch (valType) {
            case FORMAT_TYPE_INT32:
                (void)format.PutIntValue(key, parcel.ReadInt32());
                break;
            case FORMAT_TYPE_INT64:
                (void)format.PutLongValue(key, parcel.ReadInt64());
                break;
            case FORMAT_TYPE_FLOAT:
                (void)format.PutFloatValue(key, parcel.ReadFloat());
                break;
            case FORMAT_TYPE_DOUBLE:
                (void)format.PutDoubleValue(key, parcel.ReadDouble());
                break;
            case FORMAT_TYPE_STRING:
                (void)format.PutStringValue(key, parcel.ReadString());
                break;
            case FORMAT_TYPE_ADDR: {
                auto addrSize = parcel.ReadInt32();
                auto addr = parcel.ReadUnpadBuffer(static_cast<size_t>(addrSize));
                if (addr == nullptr) {
                    AVCODEC_LOGE("fail to ReadBuffer Key: %{public}s", key.c_str());
                    return false;
                }
                (void)format.PutBuffer(key, addr, static_cast<size_t>(addrSize));
                break;
            }
            default:
                AVCODEC_LOGE("fail to Unmarshalling Key: %{public}s", key.c_str());
                return false;
        }
        AVCODEC_LOGD("success to Unmarshalling Key: %{public}s", key.c_str());
    }

    return true;
}
} // namespace MediaAVCodec
} // namespace OHOS
