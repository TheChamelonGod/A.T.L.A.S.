#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <windows.h>
#include <shellscalingapi.h>
#include <thread>
#include <curl/curl.h>
#include <shobjidl.h>

//Local Includes (the ones we(I) wrote)
#include "globals.h"
#include "themes.h"
#include "recorder.h"

AtlasSettings settings;

//atomic variables for thread stuff
std::atomic<bool> isRecording(false);
std::atomic<int> sharedframecount(0);
std::atomic<int> sharedcaptureint(1);

//Mutex for UI stuff (Mutex protects elements that can/will be updated by the background thread)
std::mutex windowMutex;
std::string activewindow = "";

//UI stuff
bool showSettings = false;
ImFont* titleFont = nullptr;

//Keybind mappings (Ill add more w keycombos, or just rewrite this whole thing)
const char* keyNames[12] = {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"};
int keyCodes[12] = { VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12};

//Ts took too long, the folder picker thing u see in all *polished* apps
std::string SelectFolder() {
    std::string path;
    IFileDialog* pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS); //restricts to only folders
        }
        if (SUCCEEDED(pfd->Show(NULL))) {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR pszPath;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                    path = fs::path(pszPath).string();
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
    return path;
}

// main :3
int main(int, char**) {
    //Initializations
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    curl_global_init(CURL_GLOBAL_DEFAULT);

    SetProcessDPIAware(); //THE F$CKING NEW THINGY DOESNT WORK, it took me ~2 hours of doomscrolling on stackoverflow to give up and just use the old one. Shouldn't be an issue tho.

    // GLFW & window stuff
    if(!glfwInit()) return 1;
    const char* glsl_version = "#version 130"; //mc shader flashbacks
    GLFWwindow* window = glfwCreateWindow(1500, 900, "ATLAS", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    //get monitor dpi to scale stuff properly on higher or lower res stuff, i think it only does for higher res but i couldnt get it to work for me sooo, might get culled in final release.
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    float scale = xscale;

    //ImGui initializations
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    GLuint settingsTexture = 0;
    int settingsWidth = 0, settingsHeight = 0;

    //Font thingies
    io.Fonts->Clear();
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 3;
    fontConfig.OversampleV = 3;
    if (fs::exists("C:\\Windows\\Fonts\\consola.ttf")) {
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f * scale, &fontConfig);
    } else {
        io.Fonts->AddFontDefault();
    }

    titleFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 28.0f * scale);
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    //Scaling and themes
    ImGui::GetStyle().ScaleAllSizes(scale);
    ApplyTheme(settings.themeInd);

    //Background thread stuff, prob couldve put this in a better 
    std::thread* recorderThread = nullptr;

    auto StartRecording=[&]() {
        // clears out prev temp storage directory or creates it if it doesnt exist
        if (fs::exists(settings.storagePath)) {
            for(const auto& entry : fs::directory_iterator(settings.storagePath))
            fs::remove_all(entry.path());
        }
        else {
            fs::create_directory(settings.storagePath);
        }

        //juhhh, yeah (words are not wording rn)
        sharedframecount.store(0);
        sharedcaptureint.store(1);
        sharedRecordingDuration.store(0);
        isPaused.store(false);
        isRecording.store(true);
        isProccesing.store(false);

        // launch the recording loop in backround thread
        recorderThread = new std::thread(backRecorder);

        //gaurd be protecting fr
        {
            std::lock_guard<std::mutex> lock(windowMutex);
            activewindow = "Preparing";
        }
    };

    //flag backthread to storp
    auto StopRecording=[&]() {
        isRecording.store(false);
    };

    static bool prevRecordKey = false;
    static bool prevPauseKey = false;

    //one could say this is the real main.
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        //hotkey handling + prevent rapid toggle while holding key down

        //record logic
        bool currentRecordKey = (GetAsyncKeyState(settings.toggleRecordKey) & 0x8000)!= 0;
        if(currentRecordKey && !prevRecordKey) {
            if(isRecording.load()) StopRecording();
            else StartRecording();
        }

        prevRecordKey = currentRecordKey;

        //same thing, pause logic
        bool currentPauseKey = (GetAsyncKeyState(settings.togglePauseKey) & 0x8000)!= 0;
        if(currentPauseKey && !prevPauseKey && isRecording.load()) {
            isPaused.store(!isPaused.load());
        }

        prevPauseKey = currentPauseKey;

        // i have immense respect for gui makers now (r their official names app designers?) also im not going to explain what i did here cause i dont want to relive traumatic experiences.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15 * scale, 15 * scale));

        ImGui::Begin("AtlasMain", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);
        ImGui::PushFont(io.Fonts->Fonts[0]);

        //this is the settings button btw, couldnt find a good logo and part of why i have a font system becuase i tried importing a font thing for a settings logo
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
            ImGui::Text("SETTINGS");
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
            ImGui::Separator();
            ImGui::Text("Capture Quality");

            const char* resolutions[] = {"1080p", "1440p", "4K",};
            if (ImGui::BeginCombo("Resolution", resolutions[settings.scaleInd])) {
                for (int i = 0; i < IM_ARRAYSIZE(resolutions); i++) {
                    bool isSelected = (settings.scaleInd == i);
                    if (ImGui::Selectable(resolutions[i], isSelected)) settings.scaleInd = i;
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::SliderInt("JPEG Quality", &settings.jpegQuality, 10, 100, "%d%%");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Lower = Smaller quality but small file size");
            }

            ImGui::Spacing();
            ImGui::Text("TimeLapse Settings");
            ImGui::SliderInt("Length (Seconds)", &settings.targetVideoLength, 10, 600);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("THIS IS THE TARGET LENGTH, not the actual length, actual length will 1/2 < video < your desired length"); // do the math to make it adaptive (the 1/2 and desired length)
            }
            ImGui::SliderInt("Frames Per Second", &settings.exportFPS,  1, 60);
            //add a video length calc, for the stupid ppl (me)

            float mbPerFrame = 0.40f; //deafaults at 1080p
            if(settings.scaleInd == 2) mbPerFrame = 1.50f;
            else if(settings.scaleInd == 1) mbPerFrame = 0.80f;
            else if(settings.scaleInd == 0) mbPerFrame = 0.40f;
//why is upside down, becuase i messed up and dont feel like making it pretty.

            float qualityFactor = settings.jpegQuality / 100.0f;
            float estimatedMB = (settings.targetVideoLength * settings.exportFPS) * mbPerFrame * qualityFactor;
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Estimated Storage Required: %.1f MB", estimatedMB);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Output");

            //fake... ill make it actually reformat w codec options and stuff later
            const char* formats[] = { ".mp4", ".avi" };
            if (ImGui::BeginCombo("Video Format", formats[settings.outputFormatInd])) {
                for (int i = 0; i< IM_ARRAYSIZE(formats); i++) {
                    if(ImGui::Selectable(formats[i],settings.outputFormatInd == i)) settings.outputFormatInd =i;
                }
                ImGui::EndCombo();
            }
            
            ImGui::InputText("Output Folder", settings.outputPath, IM_ARRAYSIZE(settings.outputPath));
            ImGui::SameLine();
            if (ImGui::Button("#")) {
                std::string folder = SelectFolder();
                if (!folder.empty()) strncpy(settings.outputPath, folder.c_str(), IM_ARRAYSIZE(settings.outputPath));
            }


            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Keybinds");
            auto KeybindCombo=[](const char* label, int& currentVK) {
                int currentInd = -1;
                for (int i = 0; i < 12; i++)
                    if (keyCodes[i] == currentVK)
                        currentInd = i;
                if (currentInd < 0) currentInd = 0;
                if (ImGui::BeginCombo(label, keyNames[currentInd])) {
                    for  (int i = 0; i < 12; i++) {
                        if (ImGui::Selectable(keyNames[i], currentInd == i)) currentVK = keyCodes[i];
                    }
                    ImGui::EndCombo();
                }
            };

            KeybindCombo("Record", settings.toggleRecordKey);
            KeybindCombo("Pause", settings.togglePauseKey);
            ImGui::Spacing();
            ImGui::Text("Slack Webhook URL");
            ImGui::InputText("##slack", settings.slackWebhook, IM_ARRAYSIZE(settings.slackWebhook));

            ImGui::Spacing();
            ImGui::Checkbox("Enable Debug Stats", &settings.showStats);

            ImGui::Spacing();
            ImGui::Checkbox("Enable Smart Motion (Skip duplicate frames)", &settings.enableSmartMotion);

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

        if (isRecording.load()) {
            if(isPaused.load()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
                CenterText("PAUSED");
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                CenterText("RECORDING");
            }
        } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                CenterText("Idle");
            }
        ImGui::PopStyleColor(2);
        ImGui::Spacing();
        ImGui::Spacing();

        float buttWidth = windowWidth * 0.7f; //buttWidth is the buttons width just to clarify >:)   <--- Corny
        ImGui::SetCursorPosX((windowWidth - buttWidth)*0.5f);

        if(isRecording.load()) {
            //just for terminal and amber, red doesnt look great with those themes.             Im gonna get rid of this, maybe.
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
                StopRecording();
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
                StartRecording();
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

            ImGui::Text("Video Length");
            ImGui::NextColumn();
            float currentVid = (float)sharedframecount.load() / (float)settings.exportFPS;
            ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_Text], "---> %.2f s", currentVid);
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
        ImGui::TextDisabled("v0.1.0 Beta Release");

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

        //thread cleanup logic
        if (!isRecording.load() && recorderThread != nullptr) {
            if (isThreadFinished.load()) {
                if (recorderThread->joinable()) recorderThread->join();
                delete recorderThread;
                recorderThread = nullptr;
                isThreadFinished.store(false);
            }
        }
    }

    //Ensuring the threads to cleanly close when exiting the app.
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
    CoUninitialize();
    curl_global_cleanup();
    return 0;
}