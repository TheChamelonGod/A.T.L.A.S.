#include <iostream>
#include "exporter.h"
#include <filesystem>
#include <chrono>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

bool ExportVideo(const std::string& tempFolderPath, const std::string& outputFolderPath, int  fps, int totalFrames) {
    if (totalFrames <= 0) {
        std::cout << "{exporter} No frames to export" << std::endl;
        return false;
    }

    if (!fs::exists(outputFolderPath)) {
        fs::create_directories(outputFolderPath);
    }

    std::string firstFramePath = tempFolderPath + "\\frame_0.jpg";
    cv::Mat firstFrame = cv::imread(firstFramePath);
    if(firstFrame.empty()) {
        std::cerr << "{Exporter error} Could not read frame_0.jpg" << std::endl;
        return false;
    }

    auto now = std::chrono::system_clock::now(); // iconic camera pic names
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now_c);

    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", &timeinfo);

    std::string finalVideoPath = outputFolderPath + "\\ATLAS_" + std::string(timeStr) + ".mp4";

    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    cv::Size frameSize(firstFrame.cols, firstFrame.rows);

    cv::VideoWriter writer(finalVideoPath, fourcc, fps, frameSize, true);

    if (!writer.isOpened()) {
        std::cerr << "{Exporter error} Failed to open VideoWriter. Check codecs" << std::endl;
        return false;
    }

    std::cout << "{Exporter} Stitching " << totalFrames << " frames into " << finalVideoPath << "..." << std::endl;

    for (int i = 0; i < totalFrames; i++) {
        std::string framePath = tempFolderPath + "\\frame_" + std::to_string(i) + ".jpg";
        cv::Mat frame = cv::imread(framePath);

        if (!frame.empty()) {
            writer.write(frame);
        }
        else {
            std::cerr << "{Exporter Warning} Missing frame: " << i << std::endl;
        }
    }

    writer.release();
    std::cout << "{Exporter} Render Complete: " << finalVideoPath << std::endl;

    AutoUploadToCloud(finalVideoPath);

    return true;
}

void AutoUploadToCloud(const std::string& localFilePath) {

    std::vector<std::string> cloudPaths;

    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile) {
        cloudPaths.push_back(std::string(userProfile) + "\\OneDrive\\ATLAS_Backups");
        cloudPaths.push_back(std::string(userProfile) + "\\Google Drive\\ATLAS_Backups");
    }

    for (const auto& basePath : cloudPaths) {

        fs::path p(basePath);
        if (fs::exists(p.parent_path())) {

            if (!fs::exists(p)) fs::create_directories(p);
            
            std::string fileName = fs::path(localFilePath).filename().string();
            std::string destPath = basePath + "\\" + fileName;

            std::cout << "[CLOUD] Uploading to: " << destPath << std::endl;

            try {
                fs::copy_file(localFilePath, destPath, fs::copy_options::overwrite_existing);
                std::cout << "[CLOUD] Upload Finished!" << std::endl;
                return;
            }
            catch (const fs::filesystem_error& e) { 
                std::cerr << "[CLOUD ERROR] " << e.what() << std::endl;
            }
        }
    }
    std::cout << "[CLOUD] No active cloud drives found. No cloud backups for u." << std::endl;
}