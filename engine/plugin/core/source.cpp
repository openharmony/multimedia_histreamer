/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#include "plugin/core/source.h"
#include "plugin/interface/source_plugin.h"

using namespace OHOS::Media::Plugin;

Source::Source(uint32_t pkgVer, uint32_t apiVer, std::shared_ptr<SourcePlugin> plugin)
    : Base(pkgVer, apiVer, plugin), source_(std::move(plugin))
{
}

Status Source::SetSource(std::shared_ptr<MediaSource> source)
{
    return source_->SetSource(source);
}

Status Source::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    return source_->Read(buffer, expectedLen);
}

Status Source::GetSize(uint64_t& size)
{
    return source_->GetSize(size);
}

Seekable Source::GetSeekable()
{
    return source_->GetSeekable();
}

Status Source::SeekToPos(int64_t offset)
{
    return source_->SeekToPos(offset);
}

Status Source::SeekToTime(int64_t offset)
{
	return source_->SeekToTime(offset);
}

Status Source::GetDuration(int64_t& duration)
{
    return source_->GetDuration(duration);
}

Status Source::GetBitRates(std::vector<uint32_t>& bitRates)
{
    return source_->GetBitRates(bitRates);
}

Status Source::SelectBitRate(uint32_t bitRate)
{
    return source_->SelectBitRate(bitRate);
}