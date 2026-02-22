#pragma once
#include <string>

// gives stitches to the images to make it an mp4
bool ExportVideo(const std::string& tempFolderPath, const std::string& outputFolderPath, int fps, int totalFrames);

// Finds OneDrive and copies the video there
void AutoUploadToCloud(const std::string& finalVideoPath);