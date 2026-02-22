#include "recorder.h"
#include "globals.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>

std::string activewindowtitle() {
    char buffer[256];
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        GetWindowTextA(hwnd, buffer, 256);
        return std::string(buffer);
    }
    return "Idk";
}

// Screenshot func
// Bassically it asks Windows for the screens pickles(pixels) and makes them an OpenCV image.
cv::Mat captureScreenMat() { //cvmat is opencv main image container, its like a 2d array of pickles but with more stuff, it has greyscale, rgba, knows the size, type, and had more funcs that i will use.
    int width = GetSystemMetrics(SM_CXSCREEN); //Gets the width size of the sceen 
    int height = GetSystemMetrics(SM_CYSCREEN); //Gets the height of the screen. i have no idea how i forgot the words.

    HDC hwindowDC = GetDC(NULL); //DC is device context, Null because we dont want specific window (yet, ill do that once ive got the core done, prob gonna have to recode ts, gl future melon ;), we just want the whole screen.
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC); //Creates a compatible DC which we got so we can copy pickles to it. (drawing)

    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height); //makes blank bitmap with same size of screen
    SelectObject(hwindowCompatibleDC, hbwindow); //selects bitmap into the compatible DC,  so  all things that go to DC goes to bitmap

    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR); //sets how the images r scaled, coloron is the exact same quality as the screen(exactly like a screenshot) which is a bit much but idc as cause its taking a picture like every 10 sec.
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, width, height, SRCCOPY); //copies pickles from screenDC --> bitmap DC (no needfor x and y, was just for testing purposes)

    //defines bitmap format for opencv, (the width, heigh, and 32bitcolor top to bottom)
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; //neg cause thats how it works, kinda how mirors work. Fun fact, our eyes are kind of like this! We see the world "upside down" with our eyes but the brain acutally flips it from years of evolution and converts it right side up, i have no idea why i still know this as i learned about it in ~2019.
    bi.biPlanes = 1;
    bi.biBitCount = 32; //32bits is kinda necessary as most screens use it wether or not there is an alpha channel. Alpha channel is transparency, (RGBA) thats why each pickel is 4 bytes. so cant use 24 bit (RGB)
    bi.biCompression= BI_RGB;

    cv::Mat src(height, width, CV_8UC4); // 8u is 8 bit per channel, c4 is  4 channel per pickle, there r 16.7million colors + alpha in rgba
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS); //copies pickles from bitmap DC to opencv mat

    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(NULL, hwindowDC);
//optimization note: i can probably just make the bitmap and compatible DC once and then just keep copying to it, instead of deleting and remaking it every time, but this is fine for now as its only gonna be like every 10 sec or so, and i want to get the core done first before optimizing.
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

// - - - M a I n - - - :3

void backRecorder() {
    // idon even want to talk about the gui... ill make this configurable later, might do a multiple tab system so dont want to commit to anything rn
    const int maxframes = 100; //testing purposes, ofc it wont be this short
    int framecount = 0;
    int captureint = 1; // capture interval is 1 sec

    std::string currentPath = settings.storagePath;
    if (!fs::exists(currentPath)) fs::create_directories(currentPath);

    std::ofstream logFile(currentPath + "\\activity_log.txt", std::ios::app); //opens a log.txt file to write all window data.
    using steadyclock = std::chrono::steady_clock; //better timing system. not really usefull for anything more than 1 frame per sec but y not? 
    auto nextcap = steadyclock::now();

    while (isRecording.load()) { // check kill switch, if false, abort.
        nextcap += std::chrono::seconds(captureint); //sets next capture time
        
        while (isRecording.load() && steadyclock::now() < nextcap) { //wait until its time for next capture, also check kill switch while waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep for a short duration to prevent busy-waiting
        }

        if (!isRecording.load()) break;

        if (framecount >= maxframes) {
            AdaptiveClean(currentPath, framecount, captureint);
        } 

        // Capture the screen
        cv::Mat screenshot = captureScreenMat();
        //Checks if screen was empty or failed to capture, idk if this actually woorks or notcuase i never got this error...
        if (screenshot.empty()) {
            std::cout <<"Error: me no capture screen!" << std::endl;
            continue;
        }

        std::string newTitle = activewindowtitle(); { // le spy. just gets the windows title, breaks sometimes but it somehow knows if ur on the search thing in windows??
            std::lock_guard<std::mutex> lock(windowMutex);
            activewindow = newTitle; 
        }

        std::string filename = currentPath + "\\frame_" + std::to_string(framecount) + ".jpg"; //constructs the filename, ill make options to save it with a jpg or png or anything else l8tr. UPDATE, supposdly jpg only uses rgb??? might cuase issues so png is default. updatex2, turn out screenshots dont save the alpha channel, so jpgs r just better.
        cv::imwrite(filename, screenshot); //writes image to disk, blocking call but we dont care bcause this is images
        std::cout <<  "Saved : " << filename << std::endl; //says it saved the image, (debug tool)

        if (logFile.is_open()) {
            logFile << "Frame " << framecount << ": " << newTitle << std::endl; // writes window data to the log file with what frame and window name the user was on at the time.
        }

        framecount++; //increment frame count for next filename
        sharedframecount.store(framecount);
    }
    logFile.close(); //stops once done recording, closes log file to save it properly. 
} 
