#include "ToolsJosep.h"
#include <fstream>
#include <windows.h>
#include <wingdi.h>

#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert

// encoding function
std::string utf8_encode(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

//----------------------------------------------------------------------------------------------------------

void save_bmp1bpp(const cv::Mat& img, const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file) return;

    // save bitmap file headers
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    RGBQUAD bl = { 0,0,0,0 };  //black color
    RGBQUAD wh = { 0xff,0xff,0xff,0xff }; // white color

    fileHeader.bfType = 0x4d42;
    fileHeader.bfSize = 0;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + (sizeof(BITMAPINFOHEADER)) + 2 * sizeof(RGBQUAD);

    infoHeader.biSize = (sizeof(BITMAPINFOHEADER));
    infoHeader.biWidth = img.cols;
    infoHeader.biHeight = img.rows;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 1;
    infoHeader.biCompression = BI_RGB; //no compression needed
    infoHeader.biSizeImage = 0;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 2;
    infoHeader.biClrImportant = 2;

    file.write((char*)&fileHeader, sizeof(fileHeader)); //write bitmapfileheader
    file.write((char*)&infoHeader, (sizeof(BITMAPINFOHEADER))); //write bitmapinfoheader
    file.write((char*)&bl, sizeof(bl)); //write RGBQUAD for black
    file.write((char*)&wh, sizeof(wh)); //write RGBQUAD for white

    // convert the bits into bytes and write the file
    int numBytes = ((img.cols + 31) / 32) * 4;
    byte* bytes = (byte*)cv::fastMalloc(numBytes * sizeof(byte));

    for (int y = img.rows - 1; y >= 0; --y) {
        //offset = y * img.width();
        memset(bytes, 0, (numBytes * sizeof(byte)));
        for (int x = 0; x < img.cols; ++x) {
            byte b = img.at<uint8_t>(y, x);
            if (b & 0x01) {
                bytes[x / 8] |= 1 << (7 - x % 8);
            };
        }
        file.write((const char*)bytes, numBytes);
    };
    cv::fastFree(bytes);
    file.close();
}

//----------------------------------------------------------------------------------------------------------

