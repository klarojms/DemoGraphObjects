#pragma once
#ifndef TOOLS_JOSEP_H
#define TOOLS_JOSEP_H

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <windows.h>
#include <wingdi.h>

// encoding function
std::string utf8_encode(const std::wstring& wstr);

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str);

void save_bmp1bpp(const cv::Mat& img, const std::string& filename);

#endif