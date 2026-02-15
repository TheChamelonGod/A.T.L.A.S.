#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <string>
#include <thread>
#include <chrono>
#include <system_error>

//fs::exists()
namespace fs= std::filesystem;



// Screenshot func
// Bassically it asks Windows for the screens pickles(pixels) and makes them an OpenCV image.
cv::Mat captureScreenMat() { //cvmat is poencv main image container, its like a 2d array of pickles but with more stuff, it has greyscale, rgba, kows the size, type, and had more funcs that i will use.
    int width = GetSystemMetrics(SM_CXSCREEN); //Gets the x size of the sceen
    int height = GetSystemMetrics(SM_CYSCREEN); //Gets the y size of the screen I FORGOT THE WORDS FOR THIS, TF IS IT CALLED???

    HDC hwindowDC = GetDC(NULL); //DC is device context, Null because we dont want specific window (yet, ill do that once ive got the core done, prob gonna have to recode ts, gl future melon ;), we just want the whole screen.
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC); //Creates a compatible DC which we got so we can copy pickles to it. (drawing)

    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height); //makes blamnk bitmap with same size of screen
    SelectObject(hwindowCompatibleDC, hbwindow); //selects bitmap into the compatible DC,  so  allthings that go to DC goes to bitmap

    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR); //sets how the images r scaled, coloron is the exact same quality as the screen(exactly like a screenshot) which is a bit much but idc as cause its taking a picture like every 10 sec.
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, width, height, SRCCOPY); //copies pickles from screenDC --> bitmap DC

    //defines bitmap format for opencv, (the width, heigh, and 32bitcolor top to bottom)
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; //neg cause thats how it works, kinda how mirors work. Fun fact, our eyes are kind of like this! We see the world "upside down" with our eyes but the brain acutally flips it from years of evolution and converts it right side up, i have no idea why i still know this as i learned about it in ~2019.
    bi.biPlanes = 1;
    bi.biBitCount = 32; //32bits is kinda necessary as most screens use it wether or not there is an alpha channel. Alpha channel is transparency, (RGBA) thats why each pickel is 4 bytes. so cant use 24 bit (RGB)
    bi.biCompression= BI_RGB;
    bi.biSizeImage = 0; //uncrompressed so 0
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

int main() {
    std::cout << "--- ATLAS STARTING ---" << std::endl;

    // This is where we will save the temp images, and maybe videos but i will make it so the user can choose the folder for vids l8tr.
    std::string storagepath = "C:\\ATLAS\\Temp"; //might change this to std::string(getenv("USERPROFILE")) + "\\ATLAS\\Temp"; becuase its possible it could not be able becuase of no acess. (not admin)
    std::error_code ec; //for error handling with filesystem, it will store the error if there is one when we try to create the folder, and then we can print it out.

    if (!fs::exists(storagepath)) { //this checks if there is a folder, if not make it.
        if (fs::create_directories(storagepath, ec)) { //creates folder, if there is an error it will be stored in ec
            std::cout << "Created folder: " << storagepath << std::endl;
        }else {
            std::cerr <<"Error: " << ec.message() << std::endl; //if an error in creating the folder, prints that there is an error and the windows error message. (Usually no acess or smth like that)
            return 1; //exit with error code
        }
    }

    int framecount = 0;

    std::cout << "Recording - Press Ctrl+C to stop" << std::endl;

    while (true) {
        // Capture the screen
        cv::Mat screenshot = captureScreenMat();
        //Checks if screen was empty or failed to capture
        if (screenshot.empty()) {
            std::cout <<"Error: me no capture screen!" << std::endl;
            continue;
        }

        std::string filename = storagepath + "\\frame_" + std::to_string(framecount) + ".jpg"; //constructs the filename, ill make options to save it with a jpg or png or anything else l8tr.

        cv::imwrite(filename, screenshot); //writes image to disk, blocking call but we dont care bcause this is images
        std::cout <<  "Saved : " << filename << std::endl; //says it saved the image, (debug tool)

        framecount++; //increment frame count for next filename
        std::this_thread::sleep_for(std::chrono::seconds(1)); //wait a sec before taking next screenshot. Will make this adaptive at some point, not looking forward to that, gl future melon! wish u all the luck. ;)

    }

    return 0;
}

