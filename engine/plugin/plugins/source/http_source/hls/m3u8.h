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

#ifndef HISTREAMER_M3U8_H
#define HISTREAMER_M3U8_H

#include <memory>
#include <string>
#include <list>

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
enum class M3U8MediaType : int32_t {
    M3U8_MEDIA_TYPE_INVALID = -1,
    M3U8_MEDIA_TYPE_AUDIO,
    M3U8_MEDIA_TYPE_VIDEO,
    M3U8_MEDIA_TYPE_SUBTITLES,
    M3U8_MEDIA_TYPE_CLOSED_CAPTIONS,
    M3U8_N_MEDIA_TYPES,
};

struct M3U8InitFile {
    std::string uri;
    int offset;
    int size;
};

struct M3U8FragmentFile {
    M3U8FragmentFile(std::string &uri, std::string &title, double duration, int sequence);
    std::string uri_;
    std::string title_;
    double duration_;
    int64_t sequence_;
    bool discont_ {false};
    std::string key_ {};
    int iv_[16] {0};
    int offset_ {-1};
    int size_ {0};
};

struct M3U8 {
    bool Update(std::string &playlist);
    std::string uri_;
    std::string name_;
    
    bool endlist_;
    int version_;
    double targetDuration_;
    bool allowCache_;
    
    std::list<std::shared_ptr<M3U8FragmentFile>> files_;

    std::shared_ptr<M3U8FragmentFile> currentFile_;

    int64_t sequence_ {-1};
    int discontSequence_;

    std::string lastestData_;
};

struct M3U8Media {
    M3U8MediaType type_;

    std::string groupID_;
    std::string name_;
    std::string lang_;
    std::string uri_;
    bool isDefault_;
    bool autoSelect_;
    bool forced_;

    std::shared_ptr<M3U8> m3u8_;
};

struct M3U8VariantStream {
    std::string name_;
    std::string uri_;
    std::string codecs_;
    int bandWidth_;
    int programID_;
    int width_;
    int height_;
    bool iframe_;

    std::shared_ptr<M3U8> m3u8_;

    std::list<M3U8Media> media_;
};

struct M3U8MasterPlaylist {
    M3U8MasterPlaylist(std::string &playlist, std::string uri);

    std::list<std::shared_ptr<M3U8VariantStream>> variants_;

    std::list<M3U8VariantStream> iframeVariants_;

    std::shared_ptr<M3U8VariantStream> defaultVariant_;

    int version_;
    bool isSimple_ {false};

    std::string latestData_;
};
}
}
}
}
#endif