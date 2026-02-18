#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <string>
#include <thread>
#include <chrono>
#include <system_error>
#include <fstream>
#include <atomic>

//fs::exists()
namespace fs= std::filesystem;

std::atomic<bool> isRecording(true); //Ded switch ;) flase = die, true = record

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
    bi.biXPelsPerMeter = 0; //x resolution, which we arent using (for now?) so 0 (thats why its still here other wise it would be deleted)
    bi.biYPelsPerMeter = 0; //y resolution, which we arent using (for now?) so 0 (thats why its still here other wise it would be deleted)

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
        std::string oldFile = folderPath + "\\frame_" + std::to_string(i) + ".png"; //constructs filename

        if (i % 2 != 0) { // If i is odd. (Im beyond stupid)
            if (fs::exists(oldFile)) {
                fs::remove(oldFile); //deletes file
                std::cout << "Ded: " << oldFile <<std::endl; //ded :( *FUTURE MELON, Get rid of this after done debugging!!!*
            }
        }
        //if i is even, rename
        else {
            std::string newFile = folderPath + "\\frame_" + std::to_string(newIndex) + ".png"; //constructs new filename
            //rename only if name changes, 0->0
            if (oldFile != newFile && fs::exists(oldFile)) {
                fs::rename(oldFile, newFile); //rename
            }
            newIndex++; //increment counter v2
        }
    }

    currentframecount = newIndex; //we now have 1/2 frames
    currentInterval = currentInterval*2; //x2 wait time

    std::cout << "[Melontenance] Complete. Frames reduced to: " << currentframecount << std::endl;
    std::cout << "[Melontenance] New Interval: " << currentInterval << "s\n" << std::endl;
}

    //config, ill actually make it configurable once gui is set up, not looking forward to that, gl future melon! wish u all the luck. ;)
    const int maxframes = 100; //testing purposes, ofc it wont be this short
    int framecount = 0;
    int captureint = 1; // capture interval is 1 sec

    using steadyclock = std::chrono::steady_clock; //better timing system. not really usefull for anything more than 1 frame per sec but y not? 
    auto nextcap = steadyclock::now(); //had to rename from clock cause it would break :(

// "Worker" Thread
void backRecorder(std::string storagepath) {
    std::ofstream logFile(storagepath + "\\activity_log.txt", std::ios::app); //opens a log.txt file to write all window data.
    while (isRecording.load()) { // check kill switch, if false, abort.
        nextcap += std::chrono::seconds(captureint); //sets next capture time
        
        while (isRecording.load() && steadyclock::now() < nextcap) { //wait until its time for next capture, also check kill switch while waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for a short duration to prevent busy-waiting
        }

    if (!isRecording.load()) {
        break; // If recording was stopped during the wait, exit the loop immediately
    }

        if (framecount >= maxframes) {
            AdaptiveClean(storagepath, framecount, captureint);
        } 

        // Capture the screen
        cv::Mat screenshot = captureScreenMat();
        //Checks if screen was empty or failed to capture
        if (screenshot.empty()) {
            std::cout <<"Error: me no capture screen!" << std::endl;
            continue;
        }

        std::string activewindow = activewindowtitle(); // le spy. just gets the windows title, breaks sometimes but it somehow knows if ur on the search thing in windows??

        std::string filename = storagepath + "\\frame_" + std::to_string(framecount) + ".png"; //constructs the filename, ill make options to save it with a jpg or png or anything else l8tr. UPDATE, supposdly jpg only uses rgb??? might cuase issues so png is default.
        cv::imwrite(filename, screenshot); //writes image to disk, blocking call but we dont care bcause this is images
        std::cout <<  "Saved : " << filename << std::endl; //says it saved the image, (debug tool)

        if (logFile.is_open()) {
            logFile << "Frame " << framecount << ": " << activewindow << std::endl; // writes window data to the log file with what frame and window name the user was on at the time.
        }

        framecount++; //increment frame count for next filename

        
        }
                
        logFile.close(); //stops once done recording, closes log file to save it properly.  
    }

// main :3
int main() {

    SetProcessDPIAware(); //THE F$CKING NEW THINGY DOESNT WORK, it took me ~2 hours of doomscrolling on stackoverflow to give up and just use the old one. Shouldn't be an issue tho. NOT FORESHADOWING!

    std::cout << "--- ATLAS ---" << std::endl;

    // This is where we will save the temp images, and maybe videos but i will make it so the user can choose the folder for vids l8tr.
    std::string storagepath = "C:\\ATLAS\\Temp"; //might change this to std::string(getenv("USERPROFILE")) + "\\ATLAS\\Temp"; becuase its possible it could not be able becuase of no acess. (not admin)
    std::error_code ec; //for error handling with filesystem, it will store the error if there is one when we try to create the folder, and then we can print it out.

    fs::create_directories(storagepath, ec);

    if(ec) {
        std::cerr << "Error creating directory: " << ec.message() << std::endl;
        return 1; //exit with error code
    }

    for(const auto& entry : fs::directory_iterator(storagepath)) { //cleans the folder on start, so we dont have old frames in there.
        fs::remove_all(entry.path());
    }

    std::cout << "Background recorder started" << std::endl;
    std::thread recorderThread(backRecorder, storagepath); //starts the background thread that does the recording, we pass the storage path to it so it knows where to save the images and logs.

    // main thread, will be used more when adding ui. ill add it today or tmmrw, depends on how much debugging ive got to do, best of luck melon.
    std::cout << "Recording in main thread. Press Enter to stop..." << std::endl;
    std::cin.get(); //waits for user to press enter, this is the "kill switch" for the recording, once the user presses enter, it will stop the recording and exit the program.
    isRecording.store(false); //sets the kill switch to false, so the background thread will stop.
    recorderThread.join(); //waits for the background thread to finish before exiting the program, so that all recources are cleaned up properly
    return 0;
}