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

#ifndef HISTREAMER_PLUGIN_ALL_PLUGIN_STATIC_H
#define HISTREAMER_PLUGIN_ALL_PLUGIN_STATIC_H

#include "interface/plugin_base.h"

#define PLUGIN_REGISTER_STATIC_DECLARE(name)                                                                           \
    extern "C" OHOS::Media::Plugin::Status PLUGIN_PASTE(register_,                                                     \
                                                        name)(std::shared_ptr<OHOS::Media::Plugin::Register> reg);     \
    extern "C" OHOS::Media::Plugin::Status PLUGIN_PASTE(unregister_, name)();

#define REGISTER_STATIC(name, reg) PLUGIN_PASTE(register_, name)(reg)
#define UNREGISTER_STATIC(name) PLUGIN_PASTE(unregister_, name)()

PLUGIN_REGISTER_STATIC_DECLARE(FileSource);
PLUGIN_REGISTER_STATIC_DECLARE(StreamSource);
PLUGIN_REGISTER_STATIC_DECLARE(FFmpegDemuxer)
PLUGIN_REGISTER_STATIC_DECLARE(FFmpegAudioDecoders);
#ifdef VIDEO_SUPPORT
PLUGIN_REGISTER_STATIC_DECLARE(FFmpegVideoDecoders);
#endif
#ifdef MEDIA_OHOS
PLUGIN_REGISTER_STATIC_DECLARE(HdiAuSink);
#else
PLUGIN_REGISTER_STATIC_DECLARE(SdlAudioSink);
#ifdef VIDEO_SUPPORT
PLUGIN_REGISTER_STATIC_DECLARE(SdlVideoSink);
#endif
#endif

void RegisterPluginStatic(std::shared_ptr<OHOS::Media::Plugin::Register> reg)
{
    REGISTER_STATIC(FileSource, reg);
#ifndef UNIT_TEST
    REGISTER_STATIC(StreamSource, reg);
    REGISTER_STATIC(FFmpegDemuxer, reg);
    REGISTER_STATIC(FFmpegAudioDecoders, reg);
#ifdef VIDEO_SUPPORT
    REGISTER_STATIC(FFmpegVideoDecoders, reg);
#endif
#ifdef MEDIA_OHOS
    REGISTER_STATIC(HdiAuSink, reg);
#else
    REGISTER_STATIC(SdlAudioSink, reg);
#ifdef VIDEO_SUPPORT
    REGISTER_STATIC(SdlVideoSink, reg);
#endif
#endif
#endif
}

void UnregisterPluginStatic()
{
    UNREGISTER_STATIC(FileSource);
#ifndef UNIT_TEST
    UNREGISTER_STATIC(StreamSource);
    UNREGISTER_STATIC(FFmpegDemuxer);
    UNREGISTER_STATIC(FFmpegAudioDecoders);
#ifdef MEDIA_OHOS
    UNREGISTER_STATIC(HdiAuSink);
#else
    UNREGISTER_STATIC(SdlAudioSink);
#endif
#endif
}
#endif // HISTREAMER_PLUGIN_ALL_PLUGIN_STATIC_H
