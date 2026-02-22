This is my first time writing in c++, ill add more here once im on the polishing stage, u can look at my comments tho.
btw, it stands for Adaptive Time-Lapse Automated System, hate on it if u want, i thought it was cool.

BUidl:
g++ main.cpp themes.cpp recorder.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp -Iimgui -Iimgui/backends -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lglfw3 -lopengl32 -lgdi32 -ldwmapi -lshcore -o ATLAS.exe -std=c++17

gotta have msys64 and downloaded opencv, change dir to where urs are.
