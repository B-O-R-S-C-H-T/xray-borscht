#pragma once

#include <imgui.h>
#include <addons/ImGuizmo/ImGuizmo.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

// создаёт основной контекст (но не инициализирует бэкенды!)
void imgui_create_context();
// подготавливает текстурный атлас шрифтов _до_ первого рендера
void imgui_build_fonts();
// завершает платформенный бекэнд и уничтожает основной контекст
void imgui_destroy_context();
