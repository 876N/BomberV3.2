#ifndef DRAWING_H
#define DRAWING_H

#include <thread>
#include <chrono>
#include "../../pch.h"

class Drawing
{
private:
    static LPCSTR lpWindowName;
    static ImVec2 vWindowSize;
    static ImGuiWindowFlags WindowFlags;
    static bool bDraw;

public:
    static void Active();
    static bool isActive();
    static void Draw();
};

#endif
