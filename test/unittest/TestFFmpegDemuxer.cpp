// /*
//  * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *     http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  *

#include <memory>

#include "foundation/utils/constants.h"
#include "gtest/gtest.h"
#include "plugin/plugins/ffmpeg_adapter/demuxer/ffmpeg_track_meta.h"

using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace Test {
using namespace Plugin;
using namespace Plugin::Ffmpeg;
using namespace std;

void propagateAVSampleFormats(const AVStream& avStream,
                              const shared_ptr<AVFormatContext>& avFormatContextPtr,
                              const shared_ptr<AVCodecContext>& avCodecContextPtr,
                              Meta& meta)
{
    AVSampleFormat avSampleFormats[] = {AV_SAMPLE_FMT_NONE, AV_SAMPLE_FMT_U8};
    for (auto avSampleFormat : avSampleFormats) {
        avCodecContextPtr->sample_fmt = avSampleFormat;
        ConvertAVStreamToMetaInfo(avStream, avFormatContextPtr, avCodecContextPtr, meta);
    }
}

void propagateFrameSize(const AVStream& avStream,
                        const shared_ptr<AVFormatContext>& avFormatContextPtr,
                        const shared_ptr<AVCodecContext>& avCodecContextPtr,
                        Meta& meta)
{
    int fameSizes[] = {0, 2073600};
    for (auto frameSize : fameSizes) {
        avCodecContextPtr->frame_size = frameSize;
        avStream.codecpar->frame_size = frameSize;
        propagateAVSampleFormats(avStream, avFormatContextPtr, avCodecContextPtr, meta);
    }
}

void propagateChannelLayouts(const AVStream& avStream,
                             const shared_ptr<AVFormatContext>& avFormatContextPtr,
                             const shared_ptr<AVCodecContext>& avCodecContextPtr,
                             Meta& meta)
{
    int channelLayoutsNum = 12;
    for (int shift = 0, channelLayout = 0; shift <= channelLayoutsNum; shift++) {
        channelLayout = channelLayout << shift;
        avCodecContextPtr->channel_layout = channelLayout;
        propagateFrameSize(avStream, avFormatContextPtr, avCodecContextPtr, meta);
    }
}

void propagateChannels(const AVStream& avStream,
                       const shared_ptr<AVFormatContext>& avFormatContextPtr,
                       const shared_ptr<AVCodecContext>& avCodecContextPtr,
                       Meta& meta)
{
    int channels[] = {0, 1, 2, 3, 4, 6, 8, 10, 12, 14, 16, 24};
    for (auto channel : channels) {
        avCodecContextPtr->channels = channel;
        propagateChannelLayouts(avStream, avFormatContextPtr, avCodecContextPtr, meta);
    }
}

void propagateBitRate(const AVStream& avStream,
                      const shared_ptr<AVFormatContext>& avFormatContextPtr,
                      const shared_ptr<AVCodecContext>& avCodecContextPtr,
                      Meta& meta)
{
    int bitRates[] = {0, 14400};
    for (auto bitRate : bitRates) {
        avCodecContextPtr->bit_rate = bitRate;
        propagateChannels(avStream, avFormatContextPtr, avCodecContextPtr, meta);
    }
}

void propagateExtraData(const AVStream& avStream,
                        const shared_ptr<AVFormatContext>& avFormatContextPtr,
                        const shared_ptr<AVCodecContext>& avCodecContextPtr,
                        Meta& meta)
{
    int extraDataSize = avCodecContextPtr->extradata_size;
    avCodecContextPtr->extradata = new uint8_t[extraDataSize];

    uint8_t extraDatasInValid[] = {0x2A, 0xE6};
    for (int index = 0; index < extraDataSize; index++) {
        avCodecContextPtr->extradata[index] = extraDatasInValid[index];
    }
    propagateBitRate(avStream, avFormatContextPtr, avCodecContextPtr, meta);

    uint8_t extraDatasValid[] = {0x8A, 0xE6};
    for (int index = 0; index < extraDataSize; index++) {
        avCodecContextPtr->extradata[index] = extraDatasValid[index];
    }
    propagateBitRate(avStream, avFormatContextPtr, avCodecContextPtr, meta);

    delete[] avCodecContextPtr->extradata;
}

HWTEST(FFmpegDemuxerTrackMetaTest, test_ffmpetrack_meta, TestSize.Level1)
{
    AVStream avStream;

    avStream.codecpar = new AVCodecParameters();
    avStream.codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
    avStream.index_entries = new AVIndexEntry();

    const shared_ptr<AVFormatContext> avFormatContextPtr = make_shared<AVFormatContext>();
    const shared_ptr<AVCodecContext> avCodecContextPtr = make_shared<AVCodecContext>();
    Meta meta;

    AVCodecID codecIds[] = {AV_CODEC_ID_PCM_S16LE, AV_CODEC_ID_PCM_S16BE, AV_CODEC_ID_PCM_U16LE, AV_CODEC_ID_PCM_U16BE,
        AV_CODEC_ID_PCM_S24LE, AV_CODEC_ID_PCM_F32LE, AV_CODEC_ID_PCM_S8, AV_CODEC_ID_PCM_U8, AV_CODEC_ID_MP1,
        AV_CODEC_ID_MP2, AV_CODEC_ID_MP3, AV_CODEC_ID_AAC, AV_CODEC_ID_AAC_LATM, AV_CODEC_ID_VORBIS, AV_CODEC_ID_FLAC,
        AV_CODEC_ID_APE};

    int extraDataSizes[] = {0, 2};

    for (auto codecId : codecIds) {
        avStream.codecpar->codec_id = codecId;
        for (auto extraDataSize : extraDataSizes) {
            avCodecContextPtr->extradata_size = extraDataSize;
            propagateExtraData(avStream, avFormatContextPtr, avCodecContextPtr, meta);
        }
    }

    std::string mimeType;
    meta.Get<Tag::MIME>(mimeType);

    EXPECT_EQ(mimeType, MEDIA_MIME_AUDIO_APE);

    delete avStream.codecpar;
    delete avStream.index_entries;
}
}
}
}
