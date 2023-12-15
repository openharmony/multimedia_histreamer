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

#include "meta/format.h"
#include <sstream>
#include "common/log.h"
#include "common/status.h"
#include "meta/meta.h"
#include "securec.h"

namespace {
using namespace OHOS::Media;
using FormatDataMap = Format::FormatDataMap;
constexpr size_t BUFFER_SIZE_MAX = 1 * 1024 * 1024;

void CopyFormatVectorMap(const Format::FormatVectorMap &from, Format::FormatVectorMap &to)
{
    to = from;
}

#ifdef MEDIA_OHOS
bool PutIntValueToFormatMap(FormatDataMap &formatMap, const std::string_view &key, int32_t value)
{
    FormatData data;
    data.type = FORMAT_TYPE_INT32;
    data.val.int32Val = value;
    auto ret = formatMap.insert(std::make_pair(std::string(key), data));
    return ret.second;
}

bool PutLongValueToFormatMap(FormatDataMap &formatMap, const std::string_view &key, int64_t value)
{
    FormatData data;
    data.type = FORMAT_TYPE_INT64;
    data.val.int64Val = value;
    auto ret = formatMap.insert(std::make_pair(std::string(key), data));
    return ret.second;
}

bool PutFloatValueToFormatMap(FormatDataMap &formatMap, const std::string_view &key, float value)
{
    FormatData data;
    data.type = FORMAT_TYPE_FLOAT;
    data.val.floatVal = value;
    auto ret = formatMap.insert(std::make_pair(std::string(key), data));
    return ret.second;
}

bool PutDoubleValueToFormatMap(FormatDataMap &formatMap, const std::string_view &key, double value)
{
    FormatData data;
    data.type = FORMAT_TYPE_DOUBLE;
    data.val.doubleVal = value;
    auto ret = formatMap.insert(std::make_pair(std::string(key), data));
    return ret.second;
}

bool PutStringValueToFormatMap(FormatDataMap &formatMap, const std::string_view &key, const std::string_view &value)
{
    FormatData data;
    data.type = FORMAT_TYPE_STRING;
    data.stringVal = value;
    auto ret = formatMap.insert(std::make_pair(std::string(key), data));
    return ret.second;
}

bool PutBufferToFormatMap(FormatDataMap &formatMap, const std::string_view &key, uint8_t *addr, size_t size)
{
    FormatData data;
    FALSE_RETURN_V_MSG_E(addr != nullptr, false, "PutBuffer error, addr is nullptr");
    data.type = FORMAT_TYPE_ADDR;
    data.addr = addr;
    data.size = size;
    auto ret = formatMap.insert(std::make_pair(std::string(key), data));
    return ret.second;
}
#endif
} // namespace

namespace OHOS {
namespace Media {
Format::~Format()
{
    this->meta_ = nullptr;
}

Format::Format()
{
    this->meta_ = std::make_shared<Meta>();
}

Format::Format(const Format &rhs)
{
    if (&rhs == this) {
        return;
    }
    this->meta_ = std::make_shared<Meta>();
    *(this->meta_) = *(rhs.meta_);
    CopyFormatVectorMap(rhs.formatVecMap_, formatVecMap_);
}

Format::Format(Format &&rhs) noexcept
{
    this->meta_ = rhs.meta_;
    std::swap(formatVecMap_, rhs.formatVecMap_);
}

Format &Format::operator=(const Format &rhs)
{
    if (&rhs == this) {
        return *this;
    }
    *(this->meta_) = *(rhs.meta_);
    CopyFormatVectorMap(rhs.formatVecMap_, this->formatVecMap_);
    return *this;
}

Format &Format::operator=(Format &&rhs) noexcept
{
    if (&rhs == this) {
        return *this;
    }
    this->meta_ = rhs.meta_;
    std::swap(this->formatVecMap_, rhs.formatVecMap_);
    return *this;
}

bool Format::PutIntValue(const std::string_view &key, int32_t value)
{
    return SetMetaData(*meta_, std::string(key), value);
}

bool Format::PutLongValue(const std::string_view &key, int64_t value)
{
    return SetMetaData(*meta_, std::string(key), value);
}

bool Format::PutFloatValue(const std::string_view &key, float value)
{
    meta_->SetData(std::string(key), value);
    return true;
}

bool Format::PutDoubleValue(const std::string_view &key, double value)
{
    meta_->SetData(std::string(key), value);
    return true;
}

bool Format::PutStringValue(const std::string_view &key, const std::string_view &value)
{
    meta_->SetData(std::string(key), std::string(value));
    return true;
}

bool Format::PutBuffer(const std::string_view &key, const uint8_t *addr, size_t size)
{
    FALSE_RETURN_V_MSG_E(addr != nullptr, false, "PutBuffer error, addr is nullptr");
    FALSE_RETURN_V_MSG_E(size <= BUFFER_SIZE_MAX, false, "PutBuffer input size failed. Key: " PUBLIC_LOG_S, key.data());

    auto iter = meta_->Find(std::string(key));
    if (iter == meta_->end()) {
        std::vector<uint8_t> value(addr, addr + size);
        meta_->SetData(std::string(key), std::move(value));
        return true;
    }
    Any *value = const_cast<Any *>(&(iter->second));
    auto tmpVector = AnyCast<std::vector<uint8_t>>(value);
    tmpVector->resize(size);
    uint8_t *anyAddr = tmpVector->data();
    auto error = memcpy_s(anyAddr, size, addr, size);
    FALSE_RETURN_V_MSG_E(error == EOK, 0, "PutBuffer memcpy_s failed, error: %{public}s", strerror(error));

    auto formatMapIter = formatMap_.find(key);
    if (formatMapIter != formatMap_.end()) {
        formatMap_.erase(formatMapIter);
        PutBufferToFormatMap(formatMap_, key, anyAddr, size);
    }
    return true;
}

bool Format::GetIntValue(const std::string_view &key, int32_t &value) const
{
    return GetMetaData(*meta_, std::string(key), value);
}

bool Format::GetLongValue(const std::string_view &key, int64_t &value) const
{
    return GetMetaData(*meta_, std::string(key), value);
}

bool Format::GetFloatValue(const std::string_view &key, float &value) const
{
    return meta_->GetData(std::string(key), value);
}

bool Format::GetDoubleValue(const std::string_view &key, double &value) const
{
    return meta_->GetData(std::string(key), value);
}

bool Format::GetStringValue(const std::string_view &key, std::string &value) const
{
    return meta_->GetData(std::string(key), value);
}

bool Format::GetBuffer(const std::string_view &key, uint8_t **addr, size_t &size) const
{
    using Buf = std::vector<uint8_t>;
    auto iter = meta_->Find(std::string(key));
    if ((iter != meta_->end()) && Any::IsSameTypeWith<Buf>(iter->second)) {
        Any *value = const_cast<Any *>(&(iter->second));
        *addr = (AnyCast<Buf>(value))->data();
        size = (AnyCast<Buf>(value))->size();
        return true;
    }
    return false;
}

bool Format::PutFormatVector(const std::string_view &key, std::vector<Format> &value)
{
    RemoveKey(key);
    auto ret = formatVecMap_.insert(std::make_pair(std::string(key), value));
    return ret.second;
}

bool Format::GetFormatVector(const std::string_view &key, std::vector<Format> &value) const
{
    auto iter = formatVecMap_.find(key);
    if (iter == formatVecMap_.end()) {
        MEDIA_LOG_E("GetFormatVector failed. Key: %{public}s", key.data());
        return false;
    }
    value.assign(iter->second.begin(), iter->second.end());
    return true;
}

bool Format::ContainKey(const std::string_view &key) const
{
    auto iter = meta_->Find(std::string(key));
    if (iter != meta_->end()) {
        return true;
    }
    auto vecMapIter = formatVecMap_.find(key);
    return vecMapIter != formatVecMap_.end();
}

FormatDataType Format::GetValueType(const std::string_view &key) const
{
    auto iter = meta_->Find(std::string(key));
    if (iter != meta_->end()) {
        if (Any::IsSameTypeWith<int32_t>(iter->second)) {
            return FORMAT_TYPE_INT32;
        } else if (Any::IsSameTypeWith<int64_t>(iter->second)) {
            return FORMAT_TYPE_INT64;
        } else if (Any::IsSameTypeWith<float>(iter->second)) {
            return FORMAT_TYPE_FLOAT;
        } else if (Any::IsSameTypeWith<double>(iter->second)) {
            return FORMAT_TYPE_DOUBLE;
        } else if (Any::IsSameTypeWith<std::string>(iter->second)) {
            return FORMAT_TYPE_STRING;
        } else if (Any::IsSameTypeWith<std::vector<uint8_t>>(iter->second)) {
            return FORMAT_TYPE_ADDR;
        } else {
            int64_t valueTemp;
            bool isLongValue = GetMetaData(*meta_, std::string(key), valueTemp);
            return isLongValue ? FORMAT_TYPE_INT64 : FORMAT_TYPE_INT32;
        }
    }
    return FORMAT_TYPE_NONE;
}

void Format::RemoveKey(const std::string_view &key)
{
    meta_->Remove(std::string(key));

    auto vecMapIter = formatVecMap_.find(key);
    if (vecMapIter != formatVecMap_.end()) {
        formatVecMap_.erase(vecMapIter);
    }
}

const Format::FormatDataMap &Format::GetFormatMap() const
{
#ifdef MEDIA_OHOS
    FormatDataMap formatTemp;
    bool ret = true;
    for (auto iter = meta_->begin(); iter != meta_->end(); ++iter) {
        switch (GetValueType(iter->first)) {
            case FORMAT_TYPE_INT32:
                ret = PutIntValueToFormatMap(formatTemp, iter->first, AnyCast<int32_t>(iter->second));
                break;
            case FORMAT_TYPE_INT64:
                ret = PutLongValueToFormatMap(formatTemp, iter->first, AnyCast<int64_t>(iter->second));
                break;
            case FORMAT_TYPE_FLOAT:
                ret = PutFloatValueToFormatMap(formatTemp, iter->first, AnyCast<float>(iter->second));
                break;
            case FORMAT_TYPE_DOUBLE:
                ret = PutDoubleValueToFormatMap(formatTemp, iter->first, AnyCast<double>(iter->second));
                break;
            case FORMAT_TYPE_STRING:
                ret = PutStringValueToFormatMap(formatTemp, iter->first, AnyCast<std::string>(iter->second));
                break;
            case FORMAT_TYPE_ADDR: {
                Any *value = const_cast<Any *>(&(iter->second));
                uint8_t *addr = (AnyCast<std::vector<uint8_t>>(value))->data();
                size_t size = (AnyCast<std::vector<uint8_t>>(value))->size();
                ret = PutBufferToFormatMap(formatTemp, iter->first, addr, size);
                break;
            }
            default:
                MEDIA_LOG_E("Format::Stringify failed. Key: %{public}s", iter->first.c_str());
        }
        if (!ret) {
            MEDIA_LOG_E("Put value to formatMap failed, key = %{public}s", iter->first.c_str());
        }
    }
    FormatDataMap *formatMapRef = const_cast<FormatDataMap *>(&formatMap_);
    swap(formatTemp, *formatMapRef);
#endif
    return formatMap_;
}

const Format::FormatVectorMap &Format::GetFormatVectorMap() const
{
    return formatVecMap_;
}

std::string Format::Stringify() const
{
    std::stringstream dumpStream;
    for (auto iter = meta_->begin(); iter != meta_->end(); ++iter) {
        switch (GetValueType(iter->first)) {
            case FORMAT_TYPE_INT32:
                dumpStream << iter->first << " = " << std::to_string(AnyCast<int32_t>(iter->second)) << " | ";
                break;
            case FORMAT_TYPE_INT64:
                dumpStream << iter->first << " = " << std::to_string(AnyCast<int64_t>(iter->second)) << " | ";
                break;
            case FORMAT_TYPE_FLOAT:
                dumpStream << iter->first << " = " << std::to_string(AnyCast<float>(iter->second)) << " | ";
                break;
            case FORMAT_TYPE_DOUBLE:
                dumpStream << iter->first << " = " << std::to_string(AnyCast<double>(iter->second)) << " | ";
                break;
            case FORMAT_TYPE_STRING:
                dumpStream << iter->first << " = " << AnyCast<std::string>(iter->second) << " | ";
                break;
            case FORMAT_TYPE_ADDR: {
                Any *value = const_cast<Any *>(&(iter->second));
                dumpStream << iter->first << ", bufferSize = " << (AnyCast<std::vector<uint8_t>>(value))->size()
                           << " | ";
                break;
            }
            default:
                MEDIA_LOG_E("Format::Stringify failed. Key: %{public}s", iter->first.c_str());
        }
    }
    return dumpStream.str();
}

std::shared_ptr<Meta> Format::GetMeta()
{
    return meta_;
}

bool Format::SetMeta(std::shared_ptr<Meta> meta)
{
    if (meta == nullptr) {
        return false;
    }
    if (meta.use_count() > 1) {
        *meta_ = *meta;
    } else {
        meta_ = meta;
    }
    return true;
}
} // namespace Media
} // namespace OHOS