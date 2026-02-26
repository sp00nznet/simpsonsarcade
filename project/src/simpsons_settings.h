// simpsons - Settings persistence
// Loads/saves user configuration from simpsons_settings.toml

#pragma once

#include <string>
#include <filesystem>

struct SimpsonsSettings {
    // [gfx]
    std::string render_path = "rov";  // "rov" or "rtv"
    int resolution_scale = 1;         // 1 or 2
    bool fullscreen = false;

    // [game]
    bool full_game = true;            // unlock all content (skip trial mode)
    bool unlock_cool_stuff = true;    // unlock "cool stuff" menu
    bool unlock_all = true;           // unlock all levels, ROMs, and cool stuff

    // [controls]
    std::string controller_1 = "auto";
    std::string controller_2 = "none";
    std::string controller_3 = "none";
    std::string controller_4 = "none";
    bool connected_2 = false;
    bool connected_3 = false;
    bool connected_4 = false;

    // [debug]
    bool show_fps = true;
    bool show_console = false;
};

// Per-slot sign-in state (defined in stubs.cpp, set from ApplySettings)
// Player 1 is always connected; slots 1-3 controlled by settings.
extern bool g_simpsons_user_connected[4];

// Unlock-all flag (defined in stubs.cpp, set from ApplySettings)
// When true, forces all achievements to "achieved" status, unlocking
// Cool Stuff menu, ROM versions, and all levels.
extern bool g_simpsons_unlock_all;

// Load settings from TOML file. Returns defaults if file doesn't exist.
SimpsonsSettings LoadSettings(const std::filesystem::path& path);

// Save settings to TOML file.
void SaveSettings(const std::filesystem::path& path, const SimpsonsSettings& settings);
