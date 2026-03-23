#pragma once
#include <atomic>
#include <string>
#include <filesystem>
#include <mutex>
#include <windows.h>

namespace fs = std::filesystem;

//The config, will add the frames and such later, maybe in a tab system?
struct AtlasSettings {
    int themeInd = 0; //default theme is 0    
    int scaleInd = 0; 
    int jpegQuality = 80; 
    char storagePath[256] = "C:\\ATLAS\\Temp"; //default path for temp is here, once i add the video encoding thing, it will/should go into C:\\ATLAS\\output or something like that
    char outputPath[256]  = "C:\\ATLAS\\Output";
    int exportFPS = 30;
    bool enableSmartMotion = true;
    bool showStats = false; //for now debug (frames and capture int) will be visible
    bool autoCloudUpload = true;
    char slackWebhook[256] = "";
    int targetVideoLength = 60; // in seconds
    int outputFormatInd = 0; // 0 = .mp4, 1 = .avi
    int toggleRecordKey = VK_F8; // Default F8
    int togglePauseKey = VK_F9;  // Default F9
};

inline std::atomic<bool> isPaused(false);
inline std::atomic<bool> isThreadFinished(false);
inline std::atomic<bool> isProccesing(true);
inline std::atomic<bool> showSmartMotionPopup(false);
inline std::atomic<bool> smartMade(false);
inline std::atomic<bool> applySmartMotion(false);
inline std::atomic<int> sharedRecordingDuration(0);

extern AtlasSettings settings;
extern std::atomic<bool> isRecording;
extern std::atomic<int> sharedframecount;
extern std::atomic<int> sharedcaptureint;
extern std::string activewindow;
extern std::mutex windowMutex;
extern const char* keyNames[12];
extern int keyCodes[12];