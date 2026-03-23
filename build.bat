@echo off

g++ main.cpp exporter.cpp themes.cpp recorder.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp -Iimgui -Iimgui/backends -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_videoio -lopencv_imgproc -lglfw3 -lopengl32 -lgdi32 -ldwmapi -lshcore -lcurl -lole32 -luuid -o ATLAS.exe -std=c++17

pause