#pragma once

#include <imgui.h>
#include <addons/ImGuizmo/ImGuizmo.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include "../../xrCore/xrCore.h"

void imgui_create_context();
void imgui_build_fonts();
void imgui_destroy_context();
