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
#include "plugin_manager.h"
#include "plugin_types.h"

#define private public
#define protected public

#include "plugins/UtSourceTest1.h"
#include "plugins/UtSourceTest2.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Media::Plugin;

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin;
HWTEST(TestPluginManager, ListPlugins_case3, TestSize.Level1)
{
    UtSourceTest1::available = true;
    UtSourceTest2::available = true;
    std::vector<std::string> plugins = PluginManager::Instance().ListPlugins(PluginType::SOURCE);
    ASSERT_TRUE(plugins.size() > 1);
    for (const auto & plugin : plugins) {
        ASSERT_NE(plugin, "");
    }
}

HWTEST(TestPluginManager, ListPlugins_case4, TestSize.Level1)
{
    std::vector<std::string> plugins = PluginManager::Instance().ListPlugins(PluginType::DEMUXER);
    ASSERT_TRUE(plugins.size() > 1);
    for (const auto & plugin : plugins) {
        ASSERT_NE(plugin, "");
    }
}

HWTEST(TestPluginManager, ListPlugins_case5, TestSize.Level1)
{
    std::vector<std::string> plugins = PluginManager::Instance().ListPlugins(PluginType::AUDIO_DECODER);
    ASSERT_TRUE(plugins.size() > 1);
    for (const auto & plugin : plugins) {
        ASSERT_NE(plugin, "");
    }
}

HWTEST(TestPluginManager, ListPlugins_case6, TestSize.Level1)
{
    std::vector<std::string> plugins = PluginManager::Instance().ListPlugins(PluginType::AUDIO_SINK);
    ASSERT_FALSE(plugins.size() > 1);
}

HWTEST(TestPluginManager, ListPlugins_case7, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().ListPlugins(PluginType::INVALID_TYPE).empty());
}

HWTEST(TestPluginManager, ListPlugins_case8, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().ListPlugins(PluginType(256)).empty());
}

HWTEST(TestPluginManager, GetPluginInfo_case1, TestSize.Level1)
{
    std::shared_ptr<PluginInfo> info = PluginManager::Instance().GetPluginInfo(
            PluginType::SOURCE, "UtSourceTest1");
    ASSERT_FALSE(info != nullptr);
}

HWTEST(TestPluginManager, GetPluginInfo_case2, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().GetPluginInfo(PluginType::SOURCE, "UtSourceTest3") == nullptr);
}

HWTEST(TestPluginManager, GetPluginInfo_case3, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().GetPluginInfo(PluginType::SOURCE, "") == nullptr);
}

HWTEST(TestPluginManager, GetPluginInfo_case4, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().GetPluginInfo(PluginType::INVALID_TYPE, "") == nullptr);
}

HWTEST(TestPluginManager, GetPluginInfo_case5, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().GetPluginInfo(PluginType(256), "") == nullptr);
}

HWTEST(TestPluginManager, CreateSourcePlugin_case2, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateSourcePlugin("UtSourceTest3") == nullptr);
}

HWTEST(TestPluginManager, CreateSourcePlugin_case3, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateSourcePlugin("") == nullptr);
}

HWTEST(TestPluginManager, CreateDemuxerPlugin_case2, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateDemuxerPlugin("UtDemuxerTest3") == nullptr);
}

HWTEST(TestPluginManager, CreateDemuxerPlugin_case3, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateDemuxerPlugin("") == nullptr);
}

HWTEST(TestPluginManager, CreateCodecPlugin_case2, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateCodecPlugin("UtCodecTest3", PluginType::AUDIO_DECODER) == nullptr);
}

HWTEST(TestPluginManager, CreateCodecPlugin_case3, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateCodecPlugin("", PluginType::AUDIO_DECODER) == nullptr);
}

HWTEST(TestPluginManager, CreateAudioSinkPlugin_case2, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateAudioSinkPlugin("UtAudioSinkTest3") == nullptr);
}

HWTEST(TestPluginManager, CreateAudioSinkPlugin_case3, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().CreateAudioSinkPlugin("") == nullptr);
}

class UtDataSourceHelperTest1 : public DataSourceHelper {
public:
    ~UtDataSourceHelperTest1() override = default;

    Status ReadAt(int64_t offset, std::shared_ptr<Buffer> &buffer, size_t expectedLen) override
    {
        buffer->GetMemory()->Write(reinterpret_cast<const uint8_t *>("UtPlugin"), 8);
        return Status::OK;
    }

    Status GetSize(uint64_t &size) override
    {
        return Status::OK;
    }

    OHOS::Media::Plugin::Seekable GetSeekable() override
    {
        return OHOS::Media::Plugin::Seekable::SEEKABLE;
    }
};

class UtDataSourceHelperTest2 : public DataSourceHelper {
public:
    ~UtDataSourceHelperTest2() override = default;

    Status ReadAt(int64_t offset, std::shared_ptr<Buffer> &buffer, size_t expectedLen) override
    {
        buffer->GetMemory()->Write(reinterpret_cast<const uint8_t *>("12345678"), 8);
        return Status::OK;
    }

    Status GetSize(uint64_t &size) override
    {
        return Status::OK;
    }

    OHOS::Media::Plugin::Seekable GetSeekable() override
    {
        return OHOS::Media::Plugin::Seekable::SEEKABLE;
    }
};

HWTEST(TestPluginManager, Sniffer_case2, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().Sniffer("UtDemuxerTest1",
                                                  std::make_shared<UtDataSourceHelperTest2>()) == 0);
}

HWTEST(TestPluginManager, Sniffer_case3, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().Sniffer("UtDemuxerTest3",
                                                  std::make_shared<UtDataSourceHelperTest1>()) == 0);
}

HWTEST(TestPluginManager, Sniffer_case4, TestSize.Level1)
{
    ASSERT_TRUE(PluginManager::Instance().Sniffer("UtDemuxerTest1", nullptr) == 0);
}
} // namespace Test
} // namespace Media
} // namespace OHOS
