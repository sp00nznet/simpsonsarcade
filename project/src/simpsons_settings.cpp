// simpsons - Settings persistence implementation

#include "simpsons_settings.h"

#include <toml++/toml.hpp>
#include <fstream>

bool g_simpsons_user_connected[4] = {true, false, false, false};

SimpsonsSettings LoadSettings(const std::filesystem::path& path) {
    SimpsonsSettings s;
    if (!std::filesystem::exists(path)) return s;

    try {
        auto tbl = toml::parse_file(path.string());

        // [gfx]
        s.render_path = tbl["gfx"]["render_path"].value_or(s.render_path);
        s.resolution_scale = tbl["gfx"]["resolution_scale"].value_or(s.resolution_scale);
        s.fullscreen = tbl["gfx"]["fullscreen"].value_or(s.fullscreen);

        // [game]
        s.full_game = tbl["game"]["full_game"].value_or(s.full_game);
        s.unlock_cool_stuff = tbl["game"]["unlock_cool_stuff"].value_or(s.unlock_cool_stuff);
        s.unlock_all = tbl["game"]["unlock_all"].value_or(s.unlock_all);

        // [controls]
        s.controller_1 = tbl["controls"]["controller_1"].value_or(s.controller_1);
        s.controller_2 = tbl["controls"]["controller_2"].value_or(s.controller_2);
        s.controller_3 = tbl["controls"]["controller_3"].value_or(s.controller_3);
        s.controller_4 = tbl["controls"]["controller_4"].value_or(s.controller_4);
        s.connected_2 = tbl["controls"]["connected_2"].value_or(s.connected_2);
        s.connected_3 = tbl["controls"]["connected_3"].value_or(s.connected_3);
        s.connected_4 = tbl["controls"]["connected_4"].value_or(s.connected_4);

        // [debug]
        s.show_fps = tbl["debug"]["show_fps"].value_or(s.show_fps);
        s.show_console = tbl["debug"]["show_console"].value_or(s.show_console);
    } catch (const toml::parse_error&) {
        // Parse error: return defaults
    }

    return s;
}

void SaveSettings(const std::filesystem::path& path, const SimpsonsSettings& s) {
    std::ofstream f(path);
    if (!f) return;

    f << "[gfx]\n";
    f << "render_path = " << toml::value<std::string>(s.render_path) << "\n";
    f << "resolution_scale = " << s.resolution_scale << "\n";
    f << "fullscreen = " << (s.fullscreen ? "true" : "false") << "\n";
    f << "\n";

    f << "[game]\n";
    f << "full_game = " << (s.full_game ? "true" : "false") << "\n";
    f << "unlock_cool_stuff = " << (s.unlock_cool_stuff ? "true" : "false") << "\n";
    f << "unlock_all = " << (s.unlock_all ? "true" : "false") << "\n";
    f << "\n";

    f << "[controls]\n";
    f << "controller_1 = " << toml::value<std::string>(s.controller_1) << "\n";
    f << "controller_2 = " << toml::value<std::string>(s.controller_2) << "\n";
    f << "controller_3 = " << toml::value<std::string>(s.controller_3) << "\n";
    f << "controller_4 = " << toml::value<std::string>(s.controller_4) << "\n";
    f << "connected_2 = " << (s.connected_2 ? "true" : "false") << "\n";
    f << "connected_3 = " << (s.connected_3 ? "true" : "false") << "\n";
    f << "connected_4 = " << (s.connected_4 ? "true" : "false") << "\n";
    f << "\n";

    f << "[debug]\n";
    f << "show_fps = " << (s.show_fps ? "true" : "false") << "\n";
    f << "show_console = " << (s.show_console ? "true" : "false") << "\n";
}
