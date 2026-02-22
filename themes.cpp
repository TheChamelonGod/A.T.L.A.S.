#include "themes.h"
#include "imgui.h"

void ApplyTheme(int themeIndex) {
    ImGuiStyle& style = ImGui::GetStyle();

    //Global styling
    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    ImVec4* colors = style.Colors;

    if (themeIndex == 0) { //default
        ImGui::StyleColorsDark();
        //blue things and red things and green things
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.90f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); 
    }
    else if (themeIndex == 1) { // Simple Black
        colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.80f, 0.30f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        
        //scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    }
    else if (themeIndex == 2) { // Terminal
        colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.00f, 0.60f, 0.00f, 0.60f);
        colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.60f, 0.00f, 0.60f);
        
        // Button things
        colors[ImGuiCol_Button] = ImVec4(0.00f, 0.25f, 0.00f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.40f, 0.00f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.60f, 0.00f, 1.00f); //change

        // interaction states
        colors[ImGuiCol_Header] = ImVec4(0.00f, 0.25f, 0.00f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.40f, 0.00f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.60f, 0.00f, 1.00f);
        
        // input
        colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.15f, 0.05f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.25f, 0.00f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.40f, 0.00f, 1.00f);
        
        colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f); 

        // scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.25f, 0.00f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 0.40f, 0.00f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.60f, 0.00f, 1.00f);
    }
    else if (themeIndex == 3) { // Amber
        colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.04f, 0.02f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(1.00f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(1.00f, 0.70f, 0.00f, 0.50f);
        colors[ImGuiCol_Separator] = ImVec4(1.00f, 0.70f, 0.00f, 0.40f);

        // Button things
        colors[ImGuiCol_Button] = ImVec4(0.30f, 0.20f, 0.00f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.30f, 0.00f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.40f, 0.00f, 1.00f); //change

        // interaction states
        colors[ImGuiCol_Header] = ImVec4(0.30f, 0.20f, 0.00f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.30f, 0.00f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.40f, 0.00f, 1.00f);

        // Input Fields
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.15f, 0.05f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.30f, 0.10f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.50f, 0.35f, 0.10f, 1.00f);
        
        colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.70f, 0.00f, 1.00f);

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.04f, 0.02f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.20f, 0.00f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.30f, 0.00f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.40f, 0.00f, 1.00f);
    }
    else if (themeIndex == 4) { //navy
        colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.08f, 0.15f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(0.70f, 0.85f, 1.00f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.30f, 0.50f, 0.80f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.15f, 0.30f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.10f, 0.25f, 0.45f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.35f, 0.60f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.30f, 0.50f, 0.80f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.70f, 0.85f, 1.00f, 1.00f);
        
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.04f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.10f, 0.25f, 0.45f, 1.00f);
    }
    else if (themeIndex == 5) { // cotton candy!
        colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.07f, 0.13f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 1.00f, 1.00f); 
        colors[ImGuiCol_Border] = ImVec4(0.90f, 0.40f, 0.80f, 0.50f);
        colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.80f, 0.90f, 1.00f);
        
        // buttons, pink but hot
        colors[ImGuiCol_Button] = ImVec4(0.80f, 0.30f, 0.60f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.40f, 0.70f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.20f, 0.50f, 1.00f);

        // inputs, purple
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.12f, 0.25f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.18f, 0.35f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.20f, 0.50f, 1.00f);

        // checkmark, cyan
        colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.90f, 1.00f, 1.00f);
        
        // scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.07f, 0.13f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.30f, 0.60f, 1.00f);
    }
    else if (themeIndex == 6) { //melon
        // Backgrounds
        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.06f, 0.07f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.14f, 0.18f, 1.00f);

        // Text
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);

        // Greenish outline / structure
        colors[ImGuiCol_Border] = ImVec4(0.45f, 0.80f, 0.55f, 0.65f);
        colors[ImGuiCol_Separator] = ImVec4(0.45f, 0.80f, 0.55f, 0.55f);

        // Buttons – watermelon pink
        colors[ImGuiCol_Button] = ImVec4(0.85f, 0.28f, 0.42f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.95f, 0.35f, 0.50f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.20f, 0.35f, 1.00f);

        // Headers / Selectables
        colors[ImGuiCol_Header] = ImVec4(0.85f, 0.28f, 0.42f, 0.75f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.95f, 0.35f, 0.50f, 0.85f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.70f, 0.20f, 0.35f, 1.00f);

        // Checkmarks & focus accents – melon green
        colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.90f, 0.60f, 1.00f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.55f, 0.90f, 0.60f, 1.00f);
        
        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.06f, 0.07f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.45f, 0.80f, 0.55f, 0.85f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.90f, 0.60f, 1.00f);
    }
    else if (themeIndex == 7) { //melon^2
        colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.02f, 0.05f, 1.00f); // Dark Reddish Background
        colors[ImGuiCol_Text] = ImVec4(1.00f, 0.80f, 0.85f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.90f, 0.40f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.90f, 0.40f, 0.80f);
        
        // Buttons
        colors[ImGuiCol_Button] = ImVec4(0.90f, 0.10f, 0.40f, 1.00f); 
        colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.20f, 0.50f, 1.00f); 
        colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.05f, 0.30f, 1.00f); 

        // Inputs
        colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.05f, 0.10f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.10f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.45f, 0.15f, 0.20f, 1.00f);

        // Checkmark, rind color
        colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.90f, 0.40f, 1.00f); 

        // Header
        colors[ImGuiCol_Header] = ImVec4(0.90f, 0.10f, 0.40f, 1.00f); 
        colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.20f, 0.50f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.70f, 0.05f, 0.30f, 1.00f);

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.02f, 0.05f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.90f, 0.40f, 1.00f);
    }
}