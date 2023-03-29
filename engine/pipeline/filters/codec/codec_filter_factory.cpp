/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "CodecFilterFactory"

#include "pipeline/filters/codec/codec_filter_factory.h"
#include "pipeline/filters/codec/async_mode.h"
#include "pipeline/filters/codec/audio_decoder/audio_decoder_filter.h"
#include "pipeline/filters/codec/audio_encoder/audio_encoder_filter.h"
#include "pipeline/filters/codec/codec_filter_base.h"
#include "pipeline/filters/codec/video_decoder/video_decoder_filter.h"
#include "pipeline/filters/codec/sync_mode.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
std::shared_ptr<CodecFilterBase> CreateCodecFilter(const std::string& name, FilterCodecMode type)
{
    switch (type) {
        case FilterCodecMode::AUDIO_SYNC_DECODER:
            return std::make_shared<AudioDecoderFilter>(name, std::make_shared<SyncMode>("audioDec"));
        case FilterCodecMode::AUDIO_ASYNC_DECODER:
            return std::make_shared<AudioDecoderFilter>(name, std::make_shared<AsyncMode>("audioDec"));
#ifdef VIDEO_SUPPORT
        case FilterCodecMode::VIDEO_SYNC_DECODER:
            return std::make_shared<VideoDecoderFilter>(name, std::make_shared<SyncMode>("videoDec"));
        case FilterCodecMode::VIDEO_ASYNC_DECODER:
            return std::make_shared<VideoDecoderFilter>(name, std::make_shared<AsyncMode>("videoDec"));
#endif
        default:
            return nullptr;
    }
}
} // Pipeline
} // Media
} // OHOS

