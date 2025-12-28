/**
 * @file imgui_impl_dx11.h
 * @brief ImGui DirectX 11 backend (заглушка)
 * 
 * ВАЖНО: Скачайте реальные файлы из https://github.com/ocornut/imgui
 */

#ifndef IMGUI_IMPL_DX11_H
#define IMGUI_IMPL_DX11_H

#include "imgui.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

IMGUI_IMPL_API bool ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
IMGUI_IMPL_API void ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

// Called by ImGui_ImplDX11_NewFrame when fonts need to be rebuilt
IMGUI_IMPL_API void ImGui_ImplDX11_InvalidateDeviceObjects();
IMGUI_IMPL_API bool ImGui_ImplDX11_CreateDeviceObjects();

#endif // IMGUI_IMPL_DX11_H
