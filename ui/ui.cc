#include "ui.hh"
#include "../globals.hh"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include <m_includes/filesystem/filesystem.hpp>
#include <Windows.h>
#include <cstdio>
#include "..\inject\injector.h"
using namespace API;
const char* dirX[] = { "x64", "x86" };
int current_part_idx = NULL;
void ui::render() {
    if (!globals.active) return;

    ImGui::SetNextWindowPos(ImVec2(window_pos.x, window_pos.y), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));

    ImGui::Begin(window_title, &globals.active, window_flags);
    {
        ImGui::InputText("Path", globals.path, IM_ARRAYSIZE(globals.path));
        ImGui::InputText("File", globals.file, IM_ARRAYSIZE(globals.file));
        ImGui::InputText("Game", globals.game, IM_ARRAYSIZE(globals.game));
/*
        if (ImGui::BeginListBox("##draw_list", ImVec2(300, 40))) {
            for (int n = 0; n < IM_ARRAYSIZE(dirX); ++n) {
                const bool is_selected = (current_part_idx == n);
                if (ImGui::Selectable(dirX[n], is_selected)) { current_part_idx = n; }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected) { ImGui::SetItemDefaultFocus(); }
            }
        }
        ImGui::EndListBox();
*/


        if (ImGui::Button("Inject")) {
            if (!process::get_instance().locked)
            {
                MessageBoxA(0, "Please Search for the game", "Inject", 0);
                return;
            }
            else
                injector::get_instance().inject(process::get_instance().pid, process::get_instance().cheat_path);
        }
        ImGui::SameLine();
        if (ImGui::Button("Search"))
        {
            injector::get_instance().find_game(globals.game);
            process::get_instance().cheat_path = globals.path;
            process::get_instance().cheat_path.append(globals.file);
            if (process::get_instance().locked)
            {
                system("cls");
                AllocConsole();
                HWND console = GetConsoleWindow();
                RECT r;
                GetWindowRect(console, &r); //stores the console's current dimensions

                MoveWindow(console, r.left, r.top, 700, 400, TRUE);

                FILE* fpstdin = stdin, * fpstdout = stdout, * fpstderr = stderr;
                freopen_s(&fpstdin, "CONIN$", "r", stdin);
                freopen_s(&fpstdout, "CONOUT$", "w", stdout);
                freopen_s(&fpstderr, "CONOUT$", "w", stderr);

                std::cout << process::get_instance().game << " was found" << std::endl;
                std::cout << "Game path: " << process::get_instance().file_path << std::endl;
                std::cout << "Game pid: " << process::get_instance().pid << std::endl;
                std::cout << "Game address: " << process::get_instance().get_address(process::get_instance().game) << std::endl;
            }
        }
    }
    ImGui::End();
}

void ui::init(LPDIRECT3DDEVICE9 device) {
    dev = device;
	
    // colors
    ImGui::StyleColorsDark();

	if (window_pos.x == 0) {
		RECT screen_rect{};
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		screen_res = ImVec2(float(screen_rect.right), float(screen_rect.bottom));
		window_pos = (screen_res - window_size) * 0.5f;

		// init images here
	}
}