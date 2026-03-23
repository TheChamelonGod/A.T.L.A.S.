#include "globals.h"
#include "recorder.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "exporter.h"
#include <curl/curl.h>
#include <iomanip>

std::string EscapeJson(const std::string& input)
{
    std::string output;
    output.reserve(input.size());

    for (char c : input)
    {
        switch (c)
        {
            case '\"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n"; break;
            case '\r': break;
            case '\t': output += "\\t"; break;
            default: output += c;
        }
    }

    return output;
}

std::string activewindowtitle() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return "Idle";

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    //open the process to read executable name
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        char exePath[MAX_PATH] = { 0 };
        DWORD size = MAX_PATH;
        
        //gets full path
        if (QueryFullProcessImageNameA(hProcess, 0, exePath, &size)) {
            CloseHandle(hProcess);
            std::string fullPath(exePath);
            
            //just .exe
            size_t pos = fullPath.find_last_of("\\/");
            if (pos != std::string::npos) {
                return fullPath.substr(pos + 1); 
            }
            return fullPath;
        }
        CloseHandle(hProcess);
    }

    //ai said i shud do this, dont need this but better safe than sorry ig
    char buffer[256] = { 0 }; 
    GetWindowTextA(hwnd, buffer, 256);
    return std::string(buffer);
}

struct CaptureContext {
    HDC hScreenDC;
    HDC hMemoryDC;
    HBITMAP hBitmap;
    int width;
    int height;
    BITMAPINFOHEADER bi;
};

//redid the thing for better orginization
CaptureContext InitCaptureContext() {
    CaptureContext ctx;
    ctx.width = GetSystemMetrics(SM_CXSCREEN);
    ctx.height = GetSystemMetrics(SM_CYSCREEN);
    
    ctx.hScreenDC = GetDC(NULL);
    ctx.hMemoryDC = CreateCompatibleDC(ctx.hScreenDC);
    ctx.hBitmap = CreateCompatibleBitmap(ctx.hScreenDC, ctx.width, ctx.height);
    SelectObject(ctx.hMemoryDC, ctx.hBitmap);

    ctx.bi.biSize = sizeof(BITMAPINFOHEADER);
    ctx.bi.biWidth = ctx.width;
    ctx.bi.biHeight = -ctx.height; 
    ctx.bi.biPlanes = 1;
    ctx.bi.biBitCount = 32;
    ctx.bi.biCompression = BI_RGB;
    ctx.bi.biSizeImage = 0;
    ctx.bi.biXPelsPerMeter = 0;
    ctx.bi.biYPelsPerMeter = 0;
    ctx.bi.biClrUsed = 0;
    ctx.bi.biClrImportant = 0;

    return ctx;
}


void CleanupCaptureContext(CaptureContext& ctx) {
    DeleteObject(ctx.hBitmap);
    DeleteDC(ctx.hMemoryDC);
    ReleaseDC(NULL, ctx.hScreenDC);
}

// Screenshot func
// Bassically it asks Windows for the screens pickles and makes them an OpenCV image.
cv::Mat captureScreenMat(CaptureContext& ctx) {
    //this is appretnly better than the stretchblt
    BitBlt(ctx.hMemoryDC, 0, 0, ctx.width, ctx.height, ctx.hScreenDC, 0, 0, SRCCOPY);

    // Create the OpenCV Mat
    cv::Mat src(ctx.height, ctx.width, CV_8UC4);
    
    // Copy pickles from the memory DC straight into the OpenCV Mat
    GetDIBits(ctx.hMemoryDC, ctx.hBitmap, 0, ctx.height, src.data, (BITMAPINFO*)&ctx.bi, DIB_RGB_COLORS);

    return src;
}

// La pièce principale du puzzle (even tho i take French I didn't know how to say this so idk if this is the correct way to say it.)
// This is where the iPhone type of timelapse comes in, very simplified and dumbed down version of it tho. Idk if ill make it more complex l8tr.

void AdaptiveClean(std::string folderPath, int& currentframecount, int& currentInterval) {
    std::cout << "\n[Melontenance] Limit reached! Cleanse underway...\n" << std::endl;
    int newIndex = 0; // Tracks new number numbering system

    for (int i = 0; i < currentframecount; i++) { //loop
        std::string oldFile = folderPath + "\\frame_" + std::to_string(i) + ".jpg"; //constructs filename

        if (i % 2 != 0) { // If i is odd. (Im beyond stupid)
            if (fs::exists(oldFile)) {
                fs::remove(oldFile); //deletes file
                std::cout << "Ded: " << oldFile <<std::endl; //ded :( *FUTURE MELON, Get rid of this after done debugging!!!*
            }
        }
        //if i is even, rename
        else {
            std::string newFile = folderPath + "\\frame_" + std::to_string(newIndex) + ".jpg"; //constructs new filename
            //rename only if name changes, 0->0
            if (oldFile != newFile && fs::exists(oldFile)) {
                fs::rename(oldFile, newFile); //rename
            }
            newIndex++; //increment counter v2
        }
    }
    currentframecount = newIndex; //we now have 1/2 frames
    sharedframecount.store(newIndex);
    currentInterval *= 2; //x2 wait time, but with (correct) better math now.
    sharedcaptureint.store(currentInterval);

    //this will be gone soon :(
    std::cout << "[Melontenance] Complete. Frames reduced to: " << currentframecount << std::endl;
    std::cout << "[Melontenance] New Interval: " << currentInterval << "s\n" << std::endl;
}

void SendSlackNotification(const std::string& message)
{
    std::string webhook = settings.slackWebhook;

    if (webhook.empty()) {
        std::cout << "[SLACK] Webhook empty\n";
        return;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cout << "[SLACK] curl init failed\n";
        return;
    }

    std::string safeMsg = EscapeJson(message);
    std::string payload = "{\"text\":\"" + safeMsg + "\"}";

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, webhook.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        std::cout << "[SLACK] Send failed: " << curl_easy_strerror(res) << std::endl;
    else
        std::cout << "[SLACK] Notification sent!\n";

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

// - - - M a I n - - - :3

void backRecorder() {
    // idon even want to talk about the gui... ill make this configurable later, might do a multiple tab system so dont want to commit to anything rn
    const int maxframes = 900; 
    int framecount = 0;
    int captureint = 1; // capture interval is 1 sec
    cv::Mat prevFrameGray;

    std::map<std::string, int> appDurations;
    int totalSecondsRecorded = 0;

    std::string currentPath = settings.storagePath;
    if (!fs::exists(currentPath)) fs::create_directories(currentPath);

    std::ofstream logFile(currentPath + "\\activity_log.txt", std::ios::app); //opens a log.txt file to write all window data.
    using steadyclock = std::chrono::steady_clock; //better timing system. not really usefull for anything more than 1 frame per sec but y not? 
    auto nextcap = steadyclock::now();

    CaptureContext screenCtx = InitCaptureContext();

    while (isRecording.load()) { // check kill switch, if false, abort.
        nextcap += std::chrono::seconds(captureint); //sets next capture time
        
        while (isRecording.load() && steadyclock::now() < nextcap) { //wait until its time for next capture, also check kill switch while waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep for a short duration to prevent busy-waiting
        }

        if (!isRecording.load()) break;

        if (isPaused.load()) {
            std::lock_guard<std::mutex> lock(windowMutex);
            activewindow = "!Paused!";
            continue;
        }

        if (framecount >= maxframes) {
            AdaptiveClean(currentPath, framecount, captureint);
        } 

        // Capture the screen
        cv::Mat screenshot = captureScreenMat(screenCtx);
        //Checks if screen was empty or failed to capture, idk if this actually woorks or notcuase i never got this error...
        if (screenshot.empty()) {
            std::cout <<"Error: me no capture screen!" << std::endl;
            continue;
        }

        cv::Mat finalImage = screenshot;

        int targetW = screenshot.cols;
        int targetH = screenshot.rows;

        if (settings.scaleInd == 0) targetH = 1080;
        else if (settings.scaleInd == 1) targetH = 1440;
        else if (settings.scaleInd == 2) targetH = 2160;

        //downscale only 
        if (screenshot.rows > targetH) {
            //calc scale factor for aspect ratio
            double scale = (double)targetH / screenshot.rows;
            int newW = (int)(screenshot.cols * scale);
            int newH = targetH;
            
            cv::resize(screenshot, finalImage, cv::Size(newW, newH), 0, 0, cv::INTER_AREA);
        } else {
            finalImage = screenshot;
        }

        // smart motion thing
        cv::Mat prevFrameGray;
        cv::Mat currFrameGray;
        
        //conv to grayscale for easier comparison
        cv::cvtColor(finalImage, currFrameGray, cv::COLOR_BGR2GRAY);

        if (!prevFrameGray.empty() && prevFrameGray.size() != currFrameGray.size()) {
            prevFrameGray = cv::Mat();
        }

        if (settings.enableSmartMotion && !prevFrameGray.empty()) {
            cv::Mat diff;
            //compare current pixels with the prev pixels
            cv::absdiff(prevFrameGray, currFrameGray, diff);
            
            int nonZero = cv::countNonZero(diff > 25);
            float changedRatio = (float)nonZero / (float)(currFrameGray.rows * currFrameGray.cols);

            // If less than 1% changed, 'tis a dupe
            if (changedRatio < 0.01f) {
                std::cout << "Duplicate frame skipped (Change: " << (changedRatio * 100) << "%)" << std::endl;
                
                std::string newTitle = activewindowtitle();
                appDurations[newTitle] += captureint;
                totalSecondsRecorded += captureint;
                sharedRecordingDuration.store(totalSecondsRecorded);
                
                continue; 
            }
        }

        prevFrameGray = currFrameGray.clone();

        std::string newTitle = activewindowtitle(); { // le spy. just gets the windows title, breaks sometimes but it somehow knows if ur on the search thing in windows??
            std::lock_guard<std::mutex> lock(windowMutex);
            activewindow = newTitle; 
        }

        appDurations[newTitle] += captureint;
        totalSecondsRecorded += captureint;
        sharedRecordingDuration.store(totalSecondsRecorded);

        std::string filename = currentPath + "\\frame_" + std::to_string(framecount) + ".jpg"; //constructs the filename, ill make options to save it with a jpg or png or anything else l8tr. UPDATE, supposdly jpg only uses rgb??? might cuase issues so png is default. updatex2, turn out screenshots dont save the alpha channel, so jpgs r just better.
        std::vector<int> compressionParams;
        compressionParams.push_back(cv::IMWRITE_JPEG_QUALITY);
        compressionParams.push_back(settings.jpegQuality);
        cv::imwrite(filename, finalImage, compressionParams); //writes compressed image to disk, blocking call but we dont care bcause this is images
        std::cout <<  "Saved : " << filename << std::endl; //says it saved the image, (debug tool)

        if (logFile.is_open()) {
            logFile << "Frame " << framecount << ": " << newTitle << std::endl; // writes window data to the log file with what frame and window name the user was on at the time.
        }

        framecount++; //increment frame count for next filename
        sharedframecount.store(framecount);
    }
    logFile.close(); //stops once done recording, closes log file to save it properly. 

    isProccesing.store(true);
    
    {
        std::lock_guard<std::mutex> lock(windowMutex);
        activewindow = "Rendering..."; 
    }
    
    bool exported = ExportVideo(currentPath, settings.outputPath, settings.exportFPS, framecount);
    {
        std::lock_guard<std::mutex> lock(windowMutex);
        activewindow = "Render Complete!"; 
    }


    if (exported) {
        std::string msg =
        "*ATLAS Render Complete*\n"
        "*Frames:* " + std::to_string(framecount) +
        "\n*Output:* " + settings.outputPath;
        SendSlackNotification(msg);
    }
    
   //Gen PAR report (file + Slack)
    if (totalSecondsRecorded > 0) {
    std::ostringstream reportStream;

    reportStream << "[TIMELAPSE SESSION REPORT]\n";

    int hrs = totalSecondsRecorded / 3600;
    int mins = (totalSecondsRecorded % 3600) / 60;
    int secs = totalSecondsRecorded % 60;

    reportStream << "Total Duration: " 
                 << std::setfill('0') << std::setw(2) << hrs << ":"
                 << std::setfill('0') << std::setw(2) << mins << ":"
                 << std::setfill('0') << std::setw(2) << secs << "\n";
    reportStream << "--------------------------------\n";

    std::vector<std::pair<std::string, int>> sortedApps(appDurations.begin(), appDurations.end());
    std::sort(sortedApps.begin(), sortedApps.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    int highestPercent = 0;

    for (const auto& pair : sortedApps) {
        int aHrs = pair.second / 3600;
        int aMins = (pair.second % 3600) / 60;
        int percent = (pair.second * 100) / totalSecondsRecorded;

        if (percent > highestPercent) highestPercent = percent;

        //Only show apps used for at least 1 minute or 1% of the time
        if (percent >= 1 || pair.second >= 60) {
            reportStream << pair.first.substr(0, 28) << " ";
            reportStream << aHrs << "h ";
            reportStream << std::setfill('0') << std::setw(2) 
                         << aMins << "m ";
            reportStream << "(" << percent << "%)\n";
        }
    }

    reportStream << "--------------------------------\n";

    std::string focusScore = "LOW (Multitasking)";
    if (highestPercent >= 60) focusScore = "HIGH (LOCKED IN)";
    else if (highestPercent >= 35) focusScore = "MEDIUM";

    reportStream << "Focus Score: " << focusScore << "\n";

    //save to file
    std::string reportPath = std::string(settings.outputPath) + "\\PAR_Report.txt";
    std::ofstream reportFile(reportPath);
    if (reportFile.is_open()) {
        reportFile << reportStream.str();
        reportFile.close();
        std::cout << "[PAR] saved to: " << reportPath << std::endl;
    }

    //Mail it to slack
    std::ostringstream slackStream;
    slackStream << "*ATLAS PAR Report*\n";
    slackStream << "```" << reportStream.str() << "```";
    SendSlackNotification(slackStream.str());
    }

    CleanupCaptureContext(screenCtx);
    isProccesing.store(false);
    isThreadFinished.store(true);
}