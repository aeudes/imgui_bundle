#include "hello_imgui/hello_imgui.h"
#include "demo_utils/api_demos.h"

#include "imgui.h"
#include "nvg_imgui/nvg_imgui.h"

#include "demo_nanovg_full/demo_nanovg_full_impl.h"


struct MyNvgDemo
{
    bool Blowup = false;
    DemoData nvgDemoData = {};
    NVGcontext* vg;

    MyNvgDemo(NVGcontext* _vg)
        : vg(_vg)
    {
        int status = loadDemoData(vg, &nvgDemoData);
        IM_ASSERT((status == 0) && "Could not load demo data!");
    }

    ~MyNvgDemo()
    {
        freeDemoData(vg, &nvgDemoData);
    }

    void Render(float width, float height, int mousex, int mousey, float t)
    {
        renderDemo(vg, mousex, mousey, width, height, t, Blowup, &nvgDemoData);
    }

};



struct AppState
{
    std::unique_ptr<MyNvgDemo> myNvgDemo;
    NVGcontext * vg = nullptr;

    std::unique_ptr<NvgImgui::NvgFramebuffer> myFramebuffer;

    ImVec4 ClearColor = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
    bool DisplayInFrameBuffer = false;
};



int main(int, char**)
{
    ChdirBesideAssetsFolder();

    AppState appState;

    HelloImGui::RunnerParams runnerParams;
    runnerParams.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::NoDefaultWindow;
    runnerParams.appWindowParams.windowGeometry.size = {1200, 900};

    runnerParams.callbacks.EnqueuePostInit([&]()
    {
        appState.vg = NvgImgui::CreateNvgContext_GL(
            NvgImgui::NVG_ANTIALIAS | NvgImgui::NVG_STENCIL_STROKES | NvgImgui::NVG_DEBUG);
        appState.myNvgDemo = std::make_unique<MyNvgDemo>(appState.vg);
        int nvgImageFlags = 0; //NVG_IMAGE_FLIPY | NVG_IMAGE_PREMULTIPLIED;
        appState.myFramebuffer = std::make_unique<NvgImgui::NvgFramebuffer>(appState.vg, 1000, 600, nvgImageFlags);
    });
    runnerParams.callbacks.EnqueueBeforeExit([&]()
    {
        appState.myNvgDemo.reset();
        appState.myFramebuffer.reset();
        NvgImgui::DeleteNvgContext_GL(appState.vg);
    });

    auto nvgDrawingFunction = [&](NVGcontext *vg, float width, float height)
    {
        double now = ImGui::GetTime();
        ImVec2 mousePos(
            ImGui::GetIO().MousePos.x - ImGui::GetMainViewport()->Pos.x,
            ImGui::GetIO().MousePos.y - ImGui::GetMainViewport()->Pos.y
            );
        appState.myNvgDemo->Render(width, height, (int)mousePos.x, (int)mousePos.y, (float)now);
    };

    runnerParams.callbacks.CustomBackground = [&]()
    {
        NvgImgui::RenderNvgToBackground(appState.vg, nvgDrawingFunction, appState.ClearColor);
    };

    runnerParams.callbacks.ShowGui = [&]()
    {
        ImGui::Begin("My Window!", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Checkbox("Display in FrameBuffer", &appState.DisplayInFrameBuffer);
        ImGui::Checkbox("Blowup", &appState.myNvgDemo->Blowup);
        ImGui::SetNextItemWidth(HelloImGui::EmSize(15.f));
        ImGui::ColorEdit4("Clear color", &appState.ClearColor.x);

        if (appState.DisplayInFrameBuffer)
        {
            NvgImgui::RenderNvgToFrameBuffer(appState.vg, *appState.myFramebuffer, nvgDrawingFunction, appState.ClearColor);
            ImGui::Image(appState.myFramebuffer->TextureId, ImVec2(1000, 600));
        }

        ImGui::End();
    };

    runnerParams.fpsIdling.enableIdling = false;

    HelloImGui::Run(runnerParams);
    return 0;
}
