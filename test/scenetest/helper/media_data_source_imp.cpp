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

#include "common/media_data_source.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "media_data_source_impl.h"
#include "media_errors.h"
#include "securec.h"
#include "foundation/osal/utils/util.h"

namespace OHOS {
namespace Media {
IMediaDataSourceImpl::IMediaDataSourceImpl(std::string url, Plugin::Seekable seekable)
    : url_(std::move(url)), readPos_(0), seekable_(seekable)
{
    size_ = ReadDataFromFile();
}


int32_t IMediaDataSourceImpl::ReadAt(const std::shared_ptr<AVSharedMemory> &mem, uint32_t length, int64_t pos)
{
    readPos_ = pos;
    return GetDataFromSource(mem, length);
}

int32_t IMediaDataSourceImpl::ReadAt(int64_t pos, uint32_t length, const std::shared_ptr<AVSharedMemory> &mem)
{
    readPos_ = pos;
    return GetDataFromSource(mem, length);
}

int IMediaDataSourceImpl::ReadDataFromFile()
{
    std::string dataFullPath;
    if (OHOS::Media::OSAL::ConvertFullPath(url_, dataFullPath) && !dataFullPath.empty()) {
        url_ = dataFullPath;
    }
    std::fstream fs(url_);
    if (!fs.is_open()) {
        std::cout << "failed to open " << url_ << '\n';
        return 0;
    }
    std::stringstream ss;
    while (!fs.eof()) {
        std::string s;
        fs >> s;
        ss << s;
    }
    std::string data = ss.str();
    uint32_t formatData;
    const char* split = ",";
    char* p = strtok(const_cast<char *>(data.c_str()), split);
    while (p != nullptr) {
        if (sscanf_s(p, "%x", &formatData) != -1) {
            data_.push_back(formatData);
            p = strtok(nullptr, split);
        }
    }
    return data_.size() * 4; // 4
}

uint32_t IMediaDataSourceImpl::GetDataFromSource(const std::shared_ptr<AVSharedMemory> &mem, uint32_t length)
{
    if (readPos_ >= size_) {
        return SOURCE_ERROR_EOF;
    }
    if (memcpy_s(mem->GetBase(), mem->GetSize(), &data_[0] + readPos_ / sizeof(uint32_t), length) != EOK) {
        return SOURCE_ERROR_IO;
    }
    readPos_ += length;
    return length;
}

int32_t IMediaDataSourceImpl::ReadAt(uint32_t length, const std::shared_ptr<AVSharedMemory> &mem)
{
    return GetDataFromSource(mem, length);
}

int32_t IMediaDataSourceImpl::GetSize(int64_t &size)
{
    if (seekable_ == Plugin::Seekable::UNSEEKABLE) {
        size = -1;
        return MSERR_OK;
    }
    size = size_;
    return MSERR_OK;
}
}
}
