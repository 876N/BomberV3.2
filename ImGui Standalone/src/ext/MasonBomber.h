#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <urlmon.h>
#include <wininet.h>
#include <shellapi.h>
#include <commdlg.h>
#include <shlobj.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "user32.lib")

class MasonBomber {
public:
    std::wstring wFilePath;
    std::string filePath;
    std::string generatedOneLiner;

    std::wstring GenerateRandomFilename(const std::wstring& extension = L".bat");
    std::string UploadToCatbox(const std::wstring& filePath);
    std::string RemoveHTTPS(const std::string& url);
    void ShowTopmostMessage(const std::string& title, const std::string& message);
    void BrowseFile();
    void HandleUpload();
};
