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
#define HST_LOG_TAG "M3U8"
#include "foundation/log.h"
#include "m3u8.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
M3U8FragmentFile::M3U8FragmentFile(std::string &uri, std::string &title, double duration, int sequence)
    :uri_(uri),
    title_(title),
    duration_(duration),
    sequence_(sequence)
{
}

namespace {
bool StrHasPrefix(std::string &str, std::string prefix)
{
    return str.find(prefix) == 0;
}

std::string UriJoin(std::string &baseUri, std::string name)
{
    if (baseUri.find("qingting") != std::string::npos) {
        return "https:" + name;
    } else {
        int pos = baseUri.rfind("/");
        return baseUri.substr(0, pos + 1) + name;
    }
}
}

bool M3U8::Update(std::string &playlist)
{
    if (lastestData_.compare(playlist) == 0) {
        MEDIA_LOG_I("playlist does not change ");
        return true;
    }
    if (!StrHasPrefix(playlist, "#EXTM3U")) {
        MEDIA_LOG_I("playlist doesn't start with #EXTM3U " PUBLIC_LOG_S, playlist.c_str());
        return false;
    }
    if (playlist.find("\n#EXT-X-STREAM-INF:") != std::string::npos) {
        MEDIA_LOG_I("Not a media playlist, but a master playlist! " PUBLIC_LOG_S, playlist.c_str());
        return false;
    }

    files_.clear();

    lastestData_ = playlist;
    double duration = 0;
    std::string title;
    int mediaSequence = 0;
    bool haveMediasequence = false;
    bool discontinuity = false;
    int pos = 8;
    int len = lastestData_.size();
    int t = -1;
    std::string line;
    while (pos < len) {
        t = lastestData_.find("\n", pos);
        if (t == -1) {
            line = lastestData_.substr(pos);
            pos = len;
        } else {
            line = lastestData_.substr(pos, t - pos);
            pos = t + 1;
        }
        MEDIA_LOG_I("media playlist new line " PUBLIC_LOG_S, line.c_str());

        if (line.length() >= 1) {
            if (line[0] != '#' && line[0] != '\0') {
                if (duration <= 0) {
                    MEDIA_LOG_E("duration <=0 " PUBLIC_LOG_S, line.c_str());
                    continue;
                }
                std::string uri = UriJoin(uri_, line);
                std::shared_ptr<M3U8FragmentFile> file = std::make_shared<M3U8FragmentFile>(uri, title, duration,
                                                                                            mediaSequence++);
                file->discont_ = discontinuity;

                duration = 0;
                title = "";
                discontinuity = false;

                files_.emplace_back(file);
            } else if (StrHasPrefix(line, "#EXTINF:")) {
                std::string s = line.substr(8); // 8
                int p = s.find(",");
                duration = std::stod(s.substr(0, p), nullptr);
                title = s.substr(p + 1);
            } else if (StrHasPrefix(line, "#EXT-X-")) {
                std::string s = line.substr(7); // 7
                if (StrHasPrefix(s, "ENDLIST")) {
                    endlist_ = true;
                } else if (StrHasPrefix(s, "VERSION:")) {
                    version_ = std::stoi(s.substr(8), nullptr); // 8
                } else if (StrHasPrefix(s, "TARGETDURATION:")) {
                    targetDuration_ = std::stod(s.substr(15), nullptr); // 15
                } else if (StrHasPrefix(s, "MEDIA-SEQUENCE:")) {
                    mediaSequence = std::stoi(s.substr(15), nullptr); // 15
                    haveMediasequence = true;
                } else if (StrHasPrefix(s, "DISCONTINUITY-SEQUENCE:")) {
                    discontSequence_ = std::stoi(s.substr(23), nullptr); // 23
                    discontinuity = true;
                } else if (StrHasPrefix(s, "DISCONTINUITY")) {
                    discontSequence_++;
                    discontinuity = true;
                } else if (StrHasPrefix(s, "ALLOW-CACHE:")) {
                    allowCache_ = s.substr(12).compare("YES") == 0; // 12
                } else if (StrHasPrefix(s, "KEY:")) {
                    MEDIA_LOG_I("need to parse " PUBLIC_LOG_S, line.c_str());
                } else if (StrHasPrefix(s, "BYTERANGE:")) {
                    MEDIA_LOG_I("need to parse " PUBLIC_LOG_S, line.c_str());
                } else if (StrHasPrefix(s, "MAP:")) {
                    MEDIA_LOG_I("need to parse " PUBLIC_LOG_S, line.c_str());
                }
            }
        }
    }
    if (!files_.empty() && sequence_ == -1) {
        int i = 0;
        for (auto it = files_.rbegin(); it!= files_.rend() && i < 3; i++, it++) { // 3
            sequence_ = (*it)->sequence_;
            currentFile_ = *it;
        }
    }
    return true;
}

M3U8MasterPlaylist::M3U8MasterPlaylist(std::string &playlist, std::string uri)
{
    latestData_ = playlist;
    int pos = 0;
    int len = latestData_.size();
    std::shared_ptr<M3U8VariantStream> pendingStream;

    if (!StrHasPrefix(latestData_, "#EXTM3U")) {
        MEDIA_LOG_I("playlist doesn't start with #EXTM3U " PUBLIC_LOG_S, uri.c_str());
    }
    if (latestData_.find("\n#EXTINF:") != std::string::npos) {
        MEDIA_LOG_I("This is a simple media playlist, not a master playlist " PUBLIC_LOG_S, uri.c_str());
        isSimple_ = true;
        pendingStream = std::make_shared<M3U8VariantStream>();
        std::shared_ptr<M3U8> m3u8 = std::make_shared<M3U8>();
        m3u8->uri_ = uri;
        pendingStream->m3u8_ = m3u8;
        pendingStream->name_ = uri;
        pendingStream->uri_ = uri;
        variants_.emplace_back(pendingStream);
        defaultVariant_ = pendingStream;
        m3u8->Update(playlist);
    } else {
        pos = 8; // 8
        int t ;
        std::string line;

        while (pos < len) {
            t = latestData_.find("\n", pos);
            if (t == -1) {
                line = latestData_.substr(pos);
                pos = len;
            } else {
                line = latestData_.substr(pos, t - pos);
                pos = t + 1;
            }
            MEDIA_LOG_I("master playlist new line " PUBLIC_LOG_S, line.c_str());

            if (line.length() >= 1) {
                if (line[0] != '#' && line[0] != '\0') {
                    pendingStream->name_ = line;
                    pendingStream->uri_ = UriJoin(uri, line);
                    pendingStream->m3u8_->name_ =  pendingStream->name_;
                    pendingStream->m3u8_->uri_ = pendingStream->uri_;
                    variants_.emplace_back(pendingStream);
                    if (defaultVariant_ == nullptr) {
                        defaultVariant_ = pendingStream;
                    }
                } else if (StrHasPrefix(line, "#EXT-X-VERSION:")) {
                    version_ = std::stoi(line.substr(15), nullptr); // 15
                } else if (StrHasPrefix(line, "#EXT-X-STREAM-INF:") ||
                           StrHasPrefix(line, "#EXT-X-I-FRAME-STREAM-INF:")) {
                    std::shared_ptr<M3U8VariantStream> stream = std::make_shared<M3U8VariantStream>();
                    std::shared_ptr<M3U8> m3u8 = std::make_shared<M3U8>();
                    stream->m3u8_ = m3u8;

                    stream->iframe_ = StrHasPrefix(line, "#EXT-X-I-FRAME-STREAM-INF:");
                    int posAttBegin = stream->iframe_ ? 26: 18 ; // 26 18
                    int posAttNext = 0;
                    while (posAttNext != -1) {
                        posAttNext = line.find(",", posAttBegin);
                        std::string att = line.substr(posAttBegin, posAttNext-posAttBegin);
                        if (StrHasPrefix(att, "BANDWIDTH")) {
                            stream->bandWidth_ = std::stoi(att.substr(10), nullptr); // 10
                        }
                        if (StrHasPrefix(att, "RESOLUTION")) {
                            int p = att.find("x");
                            stream->width_ = std::stoi(att.substr(11, p - 11), nullptr) ; // 11
                            stream->height_ = std::stoi(att.substr(p + 1), nullptr) ; // 1
                        }

                        if (posAttNext == -1) {
                            break;
                        }
                        posAttBegin = posAttNext + 1;
                    }
                    pendingStream = stream;
                }
            }
        }
    }
}
}
}
}
}