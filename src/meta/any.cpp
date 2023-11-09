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
#include "meta/any.h"

namespace OHOS {
namespace Media {
#ifdef MEDIA_OHOS
bool Any::BaseTypesToParcel(const Any *operand, MessageParcel &parcel) noexcept
{
    bool ret = true;
    const auto &type = operand->TypeName();
    ret &= parcel.WriteString(std::string(type));
    if (Any::IsSameTypeWith<int32_t>(*operand)) {
        ret &= parcel.WriteInt32(*AnyCast<int32_t>(operand));
    } else if (Any::IsSameTypeWith<uint32_t>(*operand)) {
        ret &= parcel.WriteUint32(*AnyCast<uint32_t>(operand));
    } else if (Any::IsSameTypeWith<int64_t>(*operand)) {
        ret &= parcel.WriteInt64(*AnyCast<int64_t>(operand));
    } else if (Any::IsSameTypeWith<uint64_t>(*operand)) {
        ret &= parcel.WriteUint64(*AnyCast<uint64_t>(operand));
    } else if (Any::IsSameTypeWith<float>(*operand)) {
        ret &= parcel.WriteFloat(*AnyCast<float>(operand));
    } else if (Any::IsSameTypeWith<double>(*operand)) {
        ret &= parcel.WriteDouble(*AnyCast<double>(operand));
    } else if (Any::IsSameTypeWith<std::string>(*operand)) {
        ret &= parcel.WriteString(*AnyCast<std::string>(operand));
    } else if (Any::IsSameTypeWith<std::vector<uint8_t>>(*operand)) {
        ret &= parcel.WriteUInt8Vector(*AnyCast<std::vector<uint8_t>>(operand));
    } else if (Any::IsSameTypeWith<std::vector<int32_t>>(*operand)) {
        ret &= parcel.WriteInt32Vector(*AnyCast<std::vector<int32_t>>(operand));
    } else if (Any::IsSameTypeWith<std::vector<std::string>>(*operand)) {
        ret &= parcel.WriteStringVector(*AnyCast<std::vector<std::string>>(operand));
    }
    return ret;
}
// returnValue : 0 -- success; 1 -- retry enum; 2 -- failed no retry
// TODO: 这种方式，必须把类型字符串序列化，如果针对基本类型特化 ToParcel，则有可能不用传类型字符串
/*
parcel 支持的类型（//的部分meta根据后面添加的类型来选择支持）：
    // bool WriteBool(bool value);
    // bool WriteInt8(int8_t value);
    // bool WriteInt16(int16_t value);
    bool WriteInt32(int32_t value);
    bool WriteInt64(int64_t value);
    // bool WriteUint8(uint8_t value);
    // bool WriteUint16(uint16_t value);
    bool WriteUint32(uint32_t value);
    bool WriteUint64(uint64_t value);
    bool WriteFloat(float value);
    bool WriteDouble(double value);
    // bool WritePointer(uintptr_t value);

    // bool ReadBoolVector(std::vector<bool> *val);
    // bool ReadInt8Vector(std::vector<int8_t> *val);
    // bool ReadInt16Vector(std::vector<int16_t> *val);
    bool ReadInt32Vector(std::vector<int32_t> *val);
    // bool ReadInt64Vector(std::vector<int64_t> *val);
    bool ReadUInt8Vector(std::vector<uint8_t> *val);
    // bool ReadUInt16Vector(std::vector<uint16_t> *val);
    // bool ReadUInt32Vector(std::vector<uint32_t> *val);
    // bool ReadUInt64Vector(std::vector<uint64_t> *val);
    // bool ReadFloatVector(std::vector<float> *val);
    // bool ReadDoubleVector(std::vector<double> *val);
    bool ReadStringVector(std::vector<std::string> *val);
    // bool ReadString16Vector(std::vector<std::u16string> *val);
*/
int Any::BaseTypesFromParcel(Any *operand, MessageParcel &parcel) noexcept
{
    std::string type = parcel.ReadString();
    Any tmp;
    if (type == "int32_t") {
        tmp.Emplace<int32_t>(parcel.ReadInt32());
        operand->Swap(tmp);
        return 0;
    } else if (type == "uint32_t") {
        tmp.Emplace<uint32_t>(parcel.ReadUint32());
        operand->Swap(tmp);
        return 0;
    } else if (type == "int64_t") {
        tmp.Emplace<int64_t>(parcel.ReadInt64());
        operand->Swap(tmp);
        return 0;
    } else if (type == "uint64_t") {
        tmp.Emplace<uint64_t>(parcel.ReadUint64());
        operand->Swap(tmp);
        return 0;
    } else if (type == "float") {
        tmp.Emplace<float>(parcel.ReadFloat());
        operand->Swap(tmp);
        return 0;
    } else if (type == "double") {
        tmp.Emplace<double>(parcel.ReadDouble());
        operand->Swap(tmp);
        return 0;
    } else if (type == "std::string") {
        tmp.Emplace<std::string>(parcel.ReadString());
        operand->Swap(tmp);
        return 0;
    } else if (type == "std::vector<uint8_t>") {
        std::vector<uint8_t> val;
        (void)parcel.ReadUInt8Vector(&val);
        tmp.Emplace<std::vector<uint8_t>>(val);
        operand->Swap(tmp);
        return 0;
    } else if (type == "std::vector<int32_t>" ) {
        std::vector<int32_t> val;
        (void)parcel.ReadInt32Vector(&val);
        tmp.Emplace<std::vector<int32_t>>(val);
        operand->Swap(tmp);
        return 0;
    } else if (type == "std::vector<std::string>") {
        std::vector<std::string> val;
        (void)parcel.ReadStringVector(&val);
        tmp.Emplace<std::vector<std::string>>(val);
        operand->Swap(tmp);
        return 0;
    }
    return 1;
}

#endif
}
} // namespace OHOS