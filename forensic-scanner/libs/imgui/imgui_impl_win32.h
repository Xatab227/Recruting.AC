/**
 * @file imgui_impl_win32.h
 * @brief ImGui Win32 backend (заглушка)
 * 
 * ВАЖНО: Скачайте реальные файлы из https://github.com/ocornut/imgui
 */

#ifndef IMGUI_IMPL_WIN32_H
#define IMGUI_IMPL_WIN32_H

#include "imgui.h"

IMGUI_IMPL_API bool ImGui_ImplWin32_Init(void* hwnd);
IMGUI_IMPL_API void ImGui_ImplWin32_Shutdown();
IMGUI_IMPL_API void ImGui_ImplWin32_NewFrame();

// Handler for Win32 messages
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // IMGUI_IMPL_WIN32_H
