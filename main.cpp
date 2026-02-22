#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <windows.h>
#include <shellscalingapi.h>
#include <thread>

//Local Includes (the ones we(I) wrote)
#include "globals.h"
#include "themes.h"
#include "recorder.h"

AtlasSettings settings;
std::atomic<bool> isRecording(false);
std::atomic<int> sharedframecount(0);
std::atomic<int> sharedcaptureint(1);
std::mutex windowMutex;
std::string activewindow = "Waiting to spy";
bool showSettings = false;
ImFont* titleFont = nullptr;

// main :3
int main(int, char**) {
    SetProcessDPIAware(); //THE F$CKING NEW THINGY DOESNT WORK, it took me ~2 hours of doomscrolling on stackoverflow to give up and just use the old one. Shouldn't be an issue tho.
    if(!glfwInit()) return 1;
    const char* glsl_version = "#version 130"; //mc shader flashbacks
    GLFWwindow* window = glfwCreateWindow(1500, 900, "ATLAS", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    float scale = xscale;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    GLuint settingsTexture = 0;
    int settingsWidth = 0, settingsHeight = 0;

    io.Fonts->Clear();
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 3;
    fontConfig.OversampleV = 3;
    if (fs::exists("C:\\Windows\\Fonts\\consola.ttf")) {
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f * scale, &fontConfig);
    } else {
        io.Fonts->AddFontDefault();
    }
    
    titleFont = io.Fonts->AddFontFromFileTTF(
    "C:\\Windows\\Fonts\\consola.ttf", 28.0f * scale);
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    ImGui::GetStyle().ScaleAllSizes(scale);
    ApplyTheme(settings.themeInd);

    std::thread* recorderThread = nullptr;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15 * scale, 15 * scale));

        ImGui::Begin("AtlasMain", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);
        ImGui::PushFont(io.Fonts->Fonts[0]);

        if(ImGui::Button("-_-", ImVec2(40 * scale, 40 * scale))) {
            showSettings = !showSettings;
        }
        if (ImGui::IsItemHovered())ImGui::SetTooltip("Settings");
        ImGui::PopFont();

        const char* title = "ATLAS";
        float windowWidth = ImGui::GetContentRegionAvail().x;

        if (titleFont)
        ImGui::PushFont(titleFont);
        float textWidth = ImGui::CalcTextSize(title).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);

        ImGui::Text("%s", title);
        if (titleFont)
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(0, 25.0f * scale));
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        if (showSettings) ImGui::OpenPopup("Config");

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.05f, 0.08f, 0.12f, 0.35f));


        if (ImGui::BeginPopupModal("Config", &showSettings, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            ImGui::Text("SeTtInGs");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Themes");
            const char* themes[] = {
                "Default", "Simple Black", "Terminal", "Amber", "Navy", "Cotton Candy", "Melon", "Melon^2"
            };

            if (ImGui::BeginListBox("##theme", ImVec2(-FLT_MIN, 4 * ImGui::GetTextLineHeightWithSpacing()))) {
                for (int i = 0; i < IM_ARRAYSIZE(themes); i++) {
                    bool isSelected = (settings.themeInd == i);

                    if (ImGui::Selectable(themes[i], isSelected))
                    {
                        settings.themeInd = i;
                    
                        ImGui::GetStyle() = ImGuiStyle();
                        ApplyTheme(settings.themeInd);
                        ImGui::GetStyle().ScaleAllSizes(scale);
                    }

                    if (isSelected)
                    ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }


            ImGui::Spacing();
            ImGui::Text("Temp Storage Path");
            ImGui::InputText("##path", settings.storagePath, IM_ARRAYSIZE(settings.storagePath));

            ImGui::Spacing();
            ImGui::Checkbox("Enable Debug Stats", &settings.showStats);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Save and Close", ImVec2(150 * scale, 30 * scale))) {
                showSettings = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();

        }
        ImGui::PopStyleColor();

        auto CenterText = [&](const std::string& text)
        {
            float windowWidth = ImGui::GetContentRegionAvail().x;
            float textWidth   = ImGui::CalcTextSize(text.c_str()).x;
        
            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::Text("%s", text.c_str());
        };

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        CenterText(isRecording.load() ? "(REC)" : "(Idle)");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();

        float buttWidth = windowWidth * 0.7f; //buttWidth is the buttons width just to clarify >:)
        ImGui::SetCursorPosX((windowWidth - buttWidth)*0.5f);

        if(isRecording.load()) {
            //just for terminal and amber, red doesnt look great with those themes.
            bool isMonochrome = (settings.themeInd == 2 || settings.themeInd == 3);

            if(isMonochrome) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            }

            if(ImGui::Button("Stop Recording", ImVec2(buttWidth, 50*scale))) {
                isRecording.store(false);
                if(recorderThread && recorderThread->joinable()) {
                    recorderThread->join();
                    delete recorderThread;
                    recorderThread = nullptr;
                }
            }
            ImGui::PopStyleColor(2);

        }
        else {
            bool forceGreen = (settings.themeInd == 0 || settings.themeInd == 2);

            if(forceGreen) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
            }

            if(ImGui::Button("Start Recording", ImVec2(buttWidth, 50*scale))) {
                if (fs::exists(settings.storagePath)) {
                    for (const auto& entry : fs::directory_iterator(settings.storagePath))
                    fs::remove_all(entry.path());
                }
                else {
                    fs::create_directories(settings.storagePath);
                }

                sharedframecount.store(0);
                sharedcaptureint.store(1);
                activewindow = "Preparing";
                isRecording.store(true);
                recorderThread = new std::thread(backRecorder);
            }

            if(forceGreen) {
                ImGui::PopStyleColor(2);
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();

        if(settings.showStats) {
            ImGui::Spacing();
            ImGui::TextDisabled("Debug Stats");

            ImGui::Columns(2, "Debug Stats", false);
            ImGui::SetColumnWidth(0, 150*scale);

            ImGui::Text("Frames Captured");
            ImGui::NextColumn();
            ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_Text], "---> %d", sharedframecount.load());
            ImGui::NextColumn();

            ImGui::Text("Interval Rate");
            ImGui::NextColumn();
            ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_Text], "---> %d", sharedcaptureint.load());
            ImGui::NextColumn();

            ImGui::Text("Current Target");
            std::string safeWindowName; {
                std::lock_guard<std::mutex> lock(windowMutex);
                safeWindowName = activewindow;
            }
            ImGui::TextWrapped(">> %s", safeWindowName.c_str());
            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        ImGui::SetCursorPosY(ImGui::GetWindowHeight()-(25*scale));
        ImGui::TextDisabled("v0.0.1 GUI");

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    if(isRecording.load()) {
        isRecording.store(false);
        if (recorderThread && recorderThread->joinable()){
            recorderThread->join();
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}