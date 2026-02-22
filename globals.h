#pragma once
#include <atomic>
#include <string>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

//The config, will add the frames and such later, maybe in a tab system?
struct AtlasSettings {
    int themeInd = 0; //default theme is 0
    char storagePath[256] = "C:\\ATLAS\\Temp"; //default path for temp is here, once i add the video encoding thing, it will/should go into C:\\ATLAS\\output or something like that
    bool showStats = true; //for now debug (frames and capture int) will be visible
};

extern AtlasSettings settings;
extern std::atomic<bool> isRecording;
extern std::atomic<int> sharedframecount;
extern std::atomic<int> sharedcaptureint;
extern std::string activewindow;
extern std::mutex windowMutex;