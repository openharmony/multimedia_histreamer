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

#include "gtest/gtest.h"
#include "scene/player/standard/hiplayer_impl.h"
#include "plugin/common/plugin_meta.h"
#include "pipeline/filters/codec/codec_filter_factory.h"

using namespace testing::ext;
using namespace OHOS::Media::Pipeline;

namespace OHOS {
namespace Media {
namespace Test {
HWTEST(TestMeta, port_link, TestSize.Level1)
{
    std::shared_ptr<EventReceiver> receiver = std::make_shared<PipelineCore>();
    std::shared_ptr<FilterCallback> callback = std::make_shared<HiPlayerImpl>(1, 1);
    std::shared_ptr<FilterBase> sourceFilter = std::make_shared<MediaSourceFilter>("MediaSourceFilter");
    std::shared_ptr<FilterBase> demuxerFilter = std::make_shared<DemuxerFilter>("DemuxerFilter");
    sourceFilter->Init(&*receiver, &*callback);
    demuxerFilter->Init(&*receiver, &*callback);
    sourceFilter->GetOutPort(PORT_NAME_DEFAULT)->Connect(demuxerFilter->GetInPort(PORT_NAME_DEFAULT));
    demuxerFilter->GetInPort(PORT_NAME_DEFAULT)->Connect(sourceFilter->GetOutPort(PORT_NAME_DEFAULT));
    auto preFilters = demuxerFilter->GetPreFilters();
    auto nextFilters = sourceFilter->GetNextFilters();
    ASSERT_TRUE(preFilters.size() != 0);
    ASSERT_TRUE(nextFilters.size() != 0);
    demuxerFilter->UnlinkPrevFilters();
    preFilters = demuxerFilter->GetPreFilters();
    ASSERT_TRUE(preFilters.size() == 0);
}

HWTEST(TestMeta, create_filters, TestSize.Level1)
{
    ASSERT_TRUE(CreateCodecFilter("builtin.player.audiodecoder", FilterCodecMode::AUDIO_ASYNC_DECODER) != nullptr);
    ASSERT_TRUE(CreateCodecFilter("builtin.player.audiodecoder", FilterCodecMode::AUDIO_SYNC_DECODER) != nullptr);
    ASSERT_TRUE(CreateCodecFilter("builtin.player.videodecoder", FilterCodecMode::VIDEO_ASYNC_DECODER) != nullptr);
    ASSERT_TRUE(CreateCodecFilter("builtin.player.videodecoder", FilterCodecMode::VIDEO_SYNC_DECODER) != nullptr);
    ASSERT_TRUE(CreateCodecFilter("builtin.player.audioencoder", FilterCodecMode::AUDIO_SYNC_ENCODER) == nullptr);
}
} // namespace Test
} // namespace Media
} // namespace OHOS
