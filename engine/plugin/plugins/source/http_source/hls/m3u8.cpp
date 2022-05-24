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

#include <utility>

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
namespace {
bool StrHasPrefix(std::string& str, const std::string& prefix)
{
    return str.find(prefix) == 0;
}

std::string UriJoin(std::string& baseUri, const std::string& name)
{
    if (baseUri.find("qingting") != std::string::npos) {
        return "https:" + name;
    } else {
        std::string::size_type pos = baseUri.rfind('/');
        return baseUri.substr(0, pos + 1) + name;
    }
}
}

M3U8Fragment::M3U8Fragment(std::string uri, std::string title, double duration, int sequence, bool discont)
    : uri_(std::move(uri)), title_(std::move(title)), duration_(duration), sequence_(sequence), discont_(discont)
{
}

M3U8::M3U8(std::string uri, std::string name) : uri_(std::move(uri)), name_(std::move(name))
{
}

bool M3U8::Update(std::string& playlist)
{
    if (playList_ == playlist) {
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
    MEDIA_LOG_I("media playlist " PUBLIC_LOG_S, playlist.c_str());
    auto tags = ParseEntries(playlist);
    UpdateFromTags(tags);
    tags.clear();
    playList_ = playlist;
    return true;
}

void M3U8::UpdateFromTags(std::list<std::shared_ptr<Tag>>& tags)
{
    std::string uri, title;
    double duration = 0;
    bool discontinuity = false;
    for (auto& tag : tags) {
        switch (tag->GetType()) {
            case HlsTag::EXTXVERSION:
                version_ = std::static_pointer_cast<SingleValueTag>(tag)->GetValue().Decimal();
                break;
            case HlsTag::EXTXTARGETDURATION:
                targetDuration_ = std::static_pointer_cast<SingleValueTag>(tag)->GetValue().FloatingPoint();
                break;
            case HlsTag::EXTXMEDIASEQUENCE:
                sequence_ = std::static_pointer_cast<SingleValueTag>(tag)->GetValue().Decimal();
                break;
            case HlsTag::EXTXDISCONTINUITYSEQUENCE: {
                discontSequence_ =  std::static_pointer_cast<SingleValueTag>(tag)->GetValue().Decimal();
                discontinuity = true;
                break;
            }
            case HlsTag::EXTINF:
                GetExtInf(tag, duration, title);
                break;
            case HlsTag::URI:
                uri = UriJoin(uri_, std::static_pointer_cast<SingleValueTag>(tag)->GetValue().QuotedString());
                break;
            case HlsTag::EXTXBYTERANGE:
                MEDIA_LOG_I("need to parse EXTXBYTERANGE");
                break;
            case HlsTag::EXTXDISCONTINUITY: {
                discontSequence_++;
                discontinuity = true;
                break;
            }
            case HlsTag::EXTXENDLIST:
                endlist_ = true;
                break;
            case HlsTag::EXTXKEY:
                MEDIA_LOG_I("need to parse EXTXKEY");
                break;
            case HlsTag::EXTXMAP:
                MEDIA_LOG_I("need to parse EXTXMAP");
                break;
            default:
                break;
        }
        if (!uri.empty()) {
            files_.emplace_back(std::make_shared<M3U8Fragment>(uri, title, duration, sequence_++, discontinuity));
            uri = "", title = "", duration = 0, discontinuity = false;
        }
    }
}

void M3U8::GetExtInf(const std::shared_ptr<Tag>& tag, double& duration, std::string& title) const
{
    auto item = std::static_pointer_cast<ValuesListTag>(tag);
    duration =  item ->GetAttributeByName("DURATION")->FloatingPoint();
    title = item ->GetAttributeByName("TITLE")->QuotedString();
}

bool M3U8::isLive() const
{
    return true;
}

M3U8VariantStream::M3U8VariantStream(std::string name, std::string uri, std::shared_ptr<M3U8> m3u8)
    : name_(std::move(name)), uri_(std::move(uri)), m3u8_(std::move(m3u8))
{
}

M3U8MasterPlaylist::M3U8MasterPlaylist(std::string& playlist, const std::string& uri)
{
    playList_ = playlist;
    uri_ = uri;
    if (!StrHasPrefix(playList_, "#EXTM3U")) {
        MEDIA_LOG_I("playlist doesn't start with #EXTM3U " PUBLIC_LOG_S, uri.c_str());
    }
    if (playList_.find("\n#EXTINF:") != std::string::npos) {
        UpdateMediaPlaylist();
    } else {
        UpdateMasterPlaylist();
    }
}

void M3U8MasterPlaylist::UpdateMediaPlaylist()
{
    MEDIA_LOG_I("This is a simple media playlist, not a master playlist " PUBLIC_LOG_S, uri_.c_str());
    isSimple_ = true;
    auto m3u8 = std::make_shared<M3U8>(uri_, "");
    auto stream = std::make_shared<M3U8VariantStream>(uri_, uri_, m3u8);
    variants_.emplace_back(stream);
    defaultVariant_ = stream;
    m3u8->Update(playList_);
}

void M3U8MasterPlaylist::UpdateMasterPlaylist()
{
    MEDIA_LOG_I("master playlist " PUBLIC_LOG_S, playList_.c_str());
    auto tags = ParseEntries(playList_);
    for (auto& tag : tags) {
        switch (tag->GetType()) {
            case HlsTag::EXTXVERSION:
                version_ = std::static_pointer_cast<SingleValueTag>(tag)->GetValue().Decimal();
                break;
            case HlsTag::EXTXSTREAMINF:
            case HlsTag::EXTXIFRAMESTREAMINF: {
                auto item = std::static_pointer_cast<AttributesTag>(tag);
                auto name = item->GetAttributeByName("URI")->QuotedString();
                auto uri = UriJoin(uri_, name);
                auto stream = std::make_shared<M3U8VariantStream>(name, uri, std::make_shared<M3U8>(uri, name));
                if (tag->GetType() == HlsTag::EXTXIFRAMESTREAMINF) {
                    stream->iframe_ = true;
                }
                stream->bandWidth_ = item->GetAttributeByName("BANDWIDTH")->Decimal();
                auto resolution = item->GetAttributeByName("RESOLUTION");
                if (resolution) {
                    stream->width_ = resolution->GetResolution().first;
                    stream->height_ = resolution->GetResolution().second;
                }
                variants_.emplace_back(stream);
                if (defaultVariant_ == nullptr) {
                    defaultVariant_ = stream;
                }
                break;
            }
            default:
                break;
        }
    }
    tags.clear();
}
}
}
}
}