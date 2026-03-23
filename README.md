Quick disclaimer, this is my first project written in C++, so do be wary if reading my comments or going through my code.
ATLAS stands for Adaptive Time-Lapse Automated System. Did I purely choose the name ATLAS because it's an acronym, possibly, but I thought it was pretty cool and fitted my project.
Requirements:
MSYS2 (msys64) installed
glfw3
OpenCV
To install OpenCV and glfw3, download MSYS2 and open the UCRT64 terminal
Update it; pacman -Syu
Install OpenCV; pacman -S mingw-w64-ucrt-x86_64-opencv
Install glfw3; pacman -S mingw-w64-ucrt-x86_64-glfw
Make sure to update include and library paths according to your local environment.
Build Instructions:
Run the Build.bat unless you are using a different c++ compiler, where then you must use the correct directory of where your dependencies are. (Build.bat just runs the command below without having to look at that mess of a compile command)
Compile command;
g++ main.cpp exporter.cpp themes.cpp recorder.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp -Iimgui -Iimgui/backends -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_videoio -lopencv_imgproc -lglfw3 -lopengl32 -lgdi32 -ldwmapi -lshcore -lcurl -lole32 -luuid -o ATLAS.exe -std=c++17

I intend to add more detailed comments and explanations soon, but for now enjoy the beta release.

To run ATLAS, just type ./ATLAS.exe
Same goes for the building it; ./build.bat


Features:

ATLAS IS STILL IN ITS BETA PHASE SO ANY AND ALL FEATURES WILL AND CAN BE CHANGED


It currently has Smart Motion, Adaptive frames, Themes, Rough Storage Calculator, PAR’s (Productivity Analysis Report), Slack Webhook Integration, Auto Cloud Upload (if one has a onedrive on their computer), Adjustable Framerate and Video Time, 2 video types to choose from, and customizable keybinds for an ease of starting and stopping timelapses (along with pausing!).
