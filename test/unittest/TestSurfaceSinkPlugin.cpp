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
#include "plugin/plugins/sink/video_surface_sink/surface_sink_plugin.h"
#include "plugin_utils.h"
#include "ui/rs_surface_node.h"
#include "window.h"
#include "window_option.h"

using namespace testing::ext;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Plugin::VidSurfaceSinkPlugin;
constexpr uint32_t DEFAULT_WIDTH = 640;
constexpr uint32_t DEFAULT_HEIGHT = 480;
constexpr uint32_t DEFAULT_BUFFER_NUM = 32;

namespace OHOS {
namespace Media {
namespace Test {
std::shared_ptr<VideoSinkPlugin> VideoSinkPluginCreate(const std::string& name)
{
    return std::make_shared<SurfaceSinkPlugin>(name);
}

sptr<Surface> GetVideoSurface()
{
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    option->SetWindowRect({ 0, 0, 640, 480 });
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_LAUNCHING);
    option->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    sptr<Rosen::Window> window = Rosen::Window::Create("surface plugin unittest", option);
    if (window == nullptr || window->GetSurfaceNode() == nullptr) {
        return nullptr;
    }
    window->Show();
    return window->GetSurfaceNode()->GetSurface();
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_create, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = VideoSinkPluginCreate("create");
    ASSERT_TRUE(plugin != nullptr);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_process, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("process");
    ASSERT_TRUE(plugin != nullptr);
    sptr<Surface> surface = GetVideoSurface();
    ASSERT_TRUE(surface != nullptr);
    surface->SetUserData("SURFACE_FORMAT", std::to_string(PIXEL_FMT_RGBA_8888));

    plugin->SetParameter(Tag::VIDEO_SURFACE, surface);
    auto initStatus = plugin->Init();
    ASSERT_TRUE(initStatus == Status::OK);

    auto prepareStatus = plugin->Prepare();
    ASSERT_TRUE(prepareStatus == Status::OK);

    auto allocator = plugin->GetAllocator();
    ASSERT_TRUE(allocator != nullptr);

    auto stopStatus = plugin->Stop();
    ASSERT_TRUE(stopStatus == Status::OK);

    auto freeStatus = plugin->Deinit();
    ASSERT_TRUE(freeStatus == Status::OK);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_reset, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("reset");
    ASSERT_TRUE(plugin != nullptr);
    auto resetStatus = plugin->Reset();
    ASSERT_TRUE(resetStatus == Status::OK);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_start, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("start");
    ASSERT_TRUE(plugin != nullptr);
    auto startStatus = plugin->Start();
    ASSERT_TRUE(startStatus == Status::OK);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_pause, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("pause");
    ASSERT_TRUE(plugin != nullptr);
    auto pauseStatus = plugin->Pause();
    ASSERT_TRUE(pauseStatus == Status::OK);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_resume, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("resume");
    ASSERT_TRUE(plugin != nullptr);
    auto resumeStatus = plugin->Resume();
    ASSERT_TRUE(resumeStatus == Status::OK);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_flush, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("flush");
    ASSERT_TRUE(plugin != nullptr);
    auto flushStatus = plugin->Flush();
    ASSERT_TRUE(flushStatus == Status::OK);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_get_latency, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("get latency");
    ASSERT_TRUE(plugin != nullptr);
    uint64_t nanoSec;
    auto latencyStatus = plugin->GetLatency(nanoSec);
    ASSERT_TRUE(latencyStatus == Status::OK);
}

HWTEST(TestSurfaceSinkPlugin, find_surface_sink_plugins_set_parameter, TestSize.Level1)
{
    std::shared_ptr<VideoSinkPlugin> plugin = std::make_shared<SurfaceSinkPlugin>("set parameter");
    ASSERT_TRUE(plugin != nullptr);
    auto widthStatus = plugin->SetParameter(Tag::VIDEO_WIDTH, DEFAULT_WIDTH);
    ASSERT_TRUE(widthStatus == Status::OK);
    auto heightStatus = plugin->SetParameter(Tag::VIDEO_HEIGHT, DEFAULT_HEIGHT);
    ASSERT_TRUE(heightStatus == Status::OK);
    auto formatStatus = plugin->SetParameter(Tag::VIDEO_PIXEL_FORMAT, VideoPixelFormat::NV21);
    ASSERT_TRUE(formatStatus == Status::OK);
    auto surfaceNumStatus = plugin->SetParameter(Tag::VIDEO_MAX_SURFACE_NUM, DEFAULT_BUFFER_NUM);
    ASSERT_TRUE(surfaceNumStatus == Status::OK);
}

} // namespace Test
} // namespace Media
} // namespace OHOS
