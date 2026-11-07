// The Warriors launcher: graphics, controller and keyboard settings written into the game's settings.toml, the folder
// of the game files, and PLAY. Indonesian and English. It starts TheWarriors.exe and nothing else: the game keeps its
// own F10 panel.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <shobjidl.h>
#include <wincodec.h>

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <toml++/toml.hpp>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "uuid.lib")

namespace fs = std::filesystem;

// the backend header keeps this declaration in a '#if 0' block, so that including it does not drag in <windows.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {

const char* kVersion = "0.1.0";
const char* kDisplayModes[] = {"fullscreen", "borderless", "windowed"};
const uint32_t kAnisoValues[] = {0, 1, 2, 4, 8, 16};
const char* kAnisoNames[] = {"Game default", "1x", "2x", "4x", "8x", "16x"};

// ---------------------------------------------------------------- language

enum Lang { kIndonesian, kEnglish };
Lang g_lang = kIndonesian;

struct Tr {
    const char* key;
    const char* id;
    const char* en;
};

const Tr kTr[] = {
    {"app_title", "THE WARRIORS", "THE WARRIORS"},
    {"app_sub", "launcher", "launcher"},
    {"language", "Bahasa", "Language"},
    {"desc_graphics", "Resolusi, render scale, dan kualitas gambar.", "Resolution, render scale and image quality."},
    {"desc_controller", "Deadzone, kecepatan kamera, dan getaran pad.", "Deadzone, camera speed and pad vibration."},
    {"desc_keyboard", "Sensitivitas mouse dan pemetaan tombol.", "Mouse sensitivity and key bindings."},
    {"desc_files", "Lokasi data game dan executable.", "Where the game data and executable live."},
    {"tab_graphics", "Grafik", "Graphics"},
    {"tab_controller", "Controller", "Controller"},
    {"tab_keyboard", "Keyboard & Mouse", "Keyboard & Mouse"},
    {"tab_files", "Folder game", "Game files"},
    {"play", "MAINKAN", "PLAY"},
    {"save", "Simpan", "Save"},
    {"dirty", "ada perubahan belum disimpan", "unsaved changes"},

    {"graphics", "Grafik", "Graphics"},
    {"display_mode", "Mode layar", "Display mode"},
    {"width", "Lebar (0 = desktop)", "Width (0 = desktop)"},
    {"height", "Tinggi (0 = desktop)", "Height (0 = desktop)"},
    {"render_scale", "Skala render %", "Render scale %"},
    {"render_hint", "Turunkan ini dulu kalau GPU lemah; game tetap 30 FPS.",
     "Lower this first on a weak GPU; the game stays at 30 FPS."},
    {"fps_cap", "Batas FPS (0 = tanpa batas)", "FPS cap (0 = uncapped)"},
    {"vsync", "V-Sync", "V-Sync"},
    {"aniso", "Anisotropic filtering", "Anisotropic filtering"},
    {"custom_tex", "Tekstur pengganti (textures\\replacement)", "Replacement textures (textures\\replacement)"},

    {"controller", "Controller (XInput)", "Controller (XInput)"},
    {"controller_note", "Dipakai untuk pad yang terpasang; keyboard tetap bisa dipakai bersamaan.",
     "Used by a connected pad; the keyboard keeps working alongside it."},
    {"camera_speed", "Kecepatan kamera", "Camera speed"},
    {"deadzone", "Deadzone stick", "Stick deadzone"},
    {"invert_x", "Invert X", "Invert X"},
    {"invert_y", "Invert Y", "Invert Y"},
    {"vibration", "Getaran (rumble)", "Vibration (rumble)"},
    {"vibration_strength", "Kekuatan getaran", "Vibration strength"},
    {"padmap", "Pemetaan tombol pad", "Pad button mapping"},

    {"keyboard", "Keyboard dan mouse", "Keyboard and mouse"},
    {"kb_enable", "Aktifkan keyboard + mouse", "Enable keyboard + mouse"},
    {"sensitivity", "Sensitivitas mouse", "Mouse sensitivity"},
    {"smoothing", "Smoothing mouse", "Mouse smoothing"},
    {"invert_y_mouse", "Invert Y mouse", "Invert mouse Y"},
    {"menu_buttons", "Mouse mengklik menu", "Mouse clicks the menus"},
    {"key_names", "Nama key: A..Z, 0..9, F1..F12, LShift, LAlt, Space, Enter, Esc, Mouse1..Mouse5, Up/Down/Left/Right.",
     "Key names: A..Z, 0..9, F1..F12, LShift, LAlt, Space, Enter, Esc, Mouse1..Mouse5, Up/Down/Left/Right."},
    {"key_pairs", "Dua key dipisah koma, mis. 'Mouse2,Enter'.", "Two keys are comma separated, e.g. 'Mouse2,Enter'."},
    {"bindings", "Bindings", "Bindings"},
    {"reset_keys", "Reset semua key", "Reset all keys"},

    {"files", "Folder data game", "Game files folder"},
    {"files_note",
     "Folder berisi data game: default.xbe + Main.wad*/Paks.wad*/System.wad0/XBoxWad.idx, atau satu file .iso disc Xbox.",
     "Folder with the game data: default.xbe + Main.wad*/Paks.wad*/System.wad0/XBoxWad.idx, or a single Xbox .iso."},
    {"data_folder", "Folder data", "Data folder"},
    {"data_default", "(assets di samping exe)", "(assets beside the exe)"},
    {"status", "Status", "Status"},
    {"pick_folder", "Pilih folder data...", "Choose data folder..."},
    {"clear_folder", "Kosongkan (pakai assets\\ di samping exe)", "Clear (use assets\\ beside the exe)"},
    {"exe_label", "Executable game", "Game executable"},
    {"exe_missing", "(tidak ditemukan)", "(not found)"},
    {"pick_exe", "Pilih TheWarriors.exe...", "Choose TheWarriors.exe..."},
    {"detect", "Deteksi ulang", "Detect again"},
    {"settings_path", "settings.toml", "settings.toml"},
    {"settings_note", "Launcher dan game memakai file yang sama; perubahan di sini berlaku saat game dijalankan.",
     "The launcher and the game share this file; changes apply the next time the game starts."},

    {"files_ok_xbe", "default.xbe + data", "default.xbe + data"},
    {"files_ok_iso", "disc image", "disc image"},
    {"files_bad", "tidak ada default.xbe atau .iso", "no default.xbe or .iso"},
    {"files_missing", "folder tidak ada", "folder does not exist"},

    {"status_exe_missing", "TheWarriors.exe tidak ditemukan. Isi 'Executable game' di tab Folder game.",
     "TheWarriors.exe was not found. Set it in the Game files tab."},
    {"status_launch_failed", "Gagal menjalankan game (error ", "Could not start the game (error "},
    {"status_launched", "Game dijalankan: ", "Game started: "},
    {"status_write_failed", "Tidak bisa menulis ", "Could not write "},
    {"status_parse_broken", "settings.toml rusak: ", "settings.toml is malformed: "},
    {"status_saved", "Pengaturan disimpan.", "Settings saved."},

    {"watermark", "Port PC tidak resmi â€” tidak berafiliasi dengan Rockstar Games",
     "Unofficial PC port â€” not affiliated with Rockstar Games"},

    {"k_forward", "Maju", "Move forward"},
    {"k_backward", "Mundur", "Move back"},
    {"k_left", "Ke kiri", "Strafe left"},
    {"k_right", "Ke kanan", "Strafe right"},
    {"k_walk", "Jalan pelan", "Walk"},
    {"k_x", "Serang ringan (X)", "Light attack (X)"},
    {"k_a", "Serang berat (A)", "Heavy attack (A)"},
    {"k_b", "Grab / lempar (B)", "Grab / throw (B)"},
    {"k_y", "Lompat, panjat, rampok (Y)", "Jump, climb, mug (Y)"},
    {"k_black", "Block (Black)", "Block (Black)"},
    {"k_white", "White", "White"},
    {"k_lt", "Sprint (LT)", "Sprint (LT)"},
    {"k_rt", "Target (RT)", "Target (RT)"},
    {"k_rage", "Rage (kedua trigger)", "Rage (both triggers)"},
    {"k_lthumb", "Perintah geng (L3)", "Gang commands (L3)"},
    {"k_rthumb", "Klik stick kanan", "Right stick click"},
    {"k_dpad_up", "D-pad atas", "D-pad up"},
    {"k_dpad_down", "D-pad bawah", "D-pad down"},
    {"k_dpad_left", "D-pad kiri", "D-pad left"},
    {"k_dpad_right", "D-pad kanan", "D-pad right"},
    {"k_start", "Pause (Start)", "Pause (Start)"},
    {"k_back", "Back", "Back"},
    {"k_look_up", "Lihat atas", "Look up"},
    {"k_look_down", "Lihat bawah", "Look down"},
    {"k_look_left", "Lihat kiri", "Look left"},
    {"k_look_right", "Lihat kanan", "Look right"},
};

const char* T(const char* key) {
    for (const Tr& t : kTr)
        if (std::strcmp(t.key, key) == 0) return g_lang == kIndonesian ? t.id : t.en;
    return key;
}

// every key bind the game's [keys] table holds, in the order the panel shows them
struct Row {
    const char* id;
    const char* text;  // key into kTr
};
const Row kKeys[] = {
    {"forward", "k_forward"},       {"backward", "k_backward"},    {"left", "k_left"},
    {"right", "k_right"},           {"walk", "k_walk"},            {"x", "k_x"},
    {"a", "k_a"},                   {"b", "k_b"},                  {"y", "k_y"},
    {"black", "k_black"},           {"white", "k_white"},          {"left_trigger", "k_lt"},
    {"right_trigger", "k_rt"},      {"rage", "k_rage"},            {"left_thumb", "k_lthumb"},
    {"right_thumb", "k_rthumb"},    {"dpad_up", "k_dpad_up"},      {"dpad_down", "k_dpad_down"},
    {"dpad_left", "k_dpad_left"},   {"dpad_right", "k_dpad_right"}, {"start", "k_start"},
    {"back", "k_back"},             {"look_up", "k_look_up"},      {"look_down", "k_look_down"},
    {"look_left", "k_look_left"},   {"look_right", "k_look_right"},
};

// the pad's own buttons, for the [controller.buttons] remap
const Row kPad[] = {
    {"a", "A"},           {"b", "B"},           {"x", "X"},           {"y", "Y"},
    {"black", "Black"},   {"white", "White"},   {"left_trigger", "LT"}, {"right_trigger", "RT"},
    {"dpad_up", "D-pad up"}, {"dpad_down", "D-pad down"}, {"dpad_left", "D-pad left"}, {"dpad_right", "D-pad right"},
    {"start", "Start"},   {"back", "Back"},     {"left_thumb", "L3"}, {"right_thumb", "R3"},
};

struct Cfg {
    int display_mode = 1;
    int width = 0, height = 0;
    int render_percent = 100;
    int fps_cap = 60;
    bool vsync = true;
    int aniso = 0;
    bool custom_textures = true;
    float camera_speed = 1.0f;
    float deadzone = 0.24f;
    bool invert_x = false, invert_y = false;
    bool vibration = true;
    float vibration_strength = 1.0f;
    std::map<std::string, std::string> pad;
    bool kb_enabled = true;
    bool menu_buttons = true;
    bool mouse_invert_y = false;
    float sensitivity = 1.0f;
    float smoothing = 0.0f;
    std::map<std::string, std::string> keys;
    std::string game_files;
    std::string game_exe;
};

Cfg g_cfg;
std::string g_status;
fs::path g_settings;
int g_tab = 0;
bool g_dirty = false;

std::string Narrow(const fs::path& p) {
    const std::u8string s = p.u8string();
    return std::string(reinterpret_cast<const char*>(s.data()), s.size());
}

fs::path ExeDir() {
    wchar_t buf[MAX_PATH * 4];
    DWORD n = GetModuleFileNameW(nullptr, buf, DWORD(std::size(buf)));
    return fs::path(std::wstring(buf, n)).parent_path();
}

fs::path GameExe() {
    std::vector<fs::path> tries;
    std::error_code ec;
    if (!g_cfg.game_exe.empty()) {
        fs::path p = fs::u8path(g_cfg.game_exe);
        tries.push_back(p.is_absolute() ? p : ExeDir() / p);
    }
    tries.push_back(ExeDir() / "TheWarriors.exe");
    tries.push_back(ExeDir() / "game" / "TheWarriors.exe");
    tries.push_back(ExeDir() / ".." / "out" / "build" / "release" / "TheWarriors.exe");
    for (const fs::path& p : tries)
        if (fs::is_regular_file(p, ec)) return fs::weakly_canonical(p, ec);
    return {};
}

std::string PickFolder() {
    std::string out;
    IFileOpenDialog* dlg = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dlg)))) return out;
    DWORD opts = 0;
    if (SUCCEEDED(dlg->GetOptions(&opts))) dlg->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    if (SUCCEEDED(dlg->Show(nullptr))) {
        IShellItem* item = nullptr;
        if (SUCCEEDED(dlg->GetResult(&item))) {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                out = Narrow(fs::path(path));
                CoTaskMemFree(path);
            }
            item->Release();
        }
    }
    dlg->Release();
    return out;
}

// what the chosen data folder holds: default.xbe, a disc image, or nothing usable (keys into kTr)
const char* GameFilesState(const fs::path& dir) {
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) return "files_missing";
    if (fs::is_regular_file(dir / "default.xbe", ec)) return "files_ok_xbe";
    for (auto& e : fs::directory_iterator(dir, ec)) {
        std::string ext = e.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return char(std::tolower(c)); });
        if (ext == ".iso") return "files_ok_iso";
    }
    return "files_bad";
}

void Load() {
    g_cfg = Cfg{};
    g_lang = kIndonesian;
    toml::table t;
    std::error_code ec;
    if (fs::exists(g_settings, ec)) {
        try {
            t = toml::parse_file(g_settings.string());
        } catch (const toml::parse_error& e) {
            g_status = std::string(T("status_parse_broken")) + e.description().data();
        }
    }
    auto str = [&](const char* k, const std::string& def) {
        if (auto v = t[k].value<std::string>()) return *v;
        return def;
    };
    auto num = [&](const char* k, int64_t def) {
        if (auto v = t[k].value<int64_t>()) return int(*v);
        return int(def);
    };
    if (str("launcher_language", "id") == "en") g_lang = kEnglish;
    std::string mode = str("display_mode", kDisplayModes[g_cfg.display_mode]);
    for (int i = 0; i < 3; i++)
        if (mode == kDisplayModes[i]) g_cfg.display_mode = i;
    g_cfg.width = num("width", g_cfg.width);
    g_cfg.height = num("height", g_cfg.height);
    g_cfg.render_percent = num("render_percent", g_cfg.render_percent);
    g_cfg.fps_cap = num("fps_cap", g_cfg.fps_cap);
    if (auto v = t["vsync"].value<bool>()) g_cfg.vsync = *v;
    int aniso = num("anisotropic_filtering", 0);
    for (int i = 0; i < 6; i++)
        if (aniso == int(kAnisoValues[i])) g_cfg.aniso = i;
    if (auto v = t["custom_textures"].value<bool>()) g_cfg.custom_textures = *v;
    g_cfg.game_files = str("game_files", "");
    g_cfg.game_exe = str("game_exe", "");

    if (auto* c = t["controller"].as_table()) {
        if (auto v = (*c)["camera_speed"].value<double>()) g_cfg.camera_speed = float(*v);
        if (auto v = (*c)["deadzone"].value<double>()) g_cfg.deadzone = float(*v);
        if (auto v = (*c)["invert_x"].value<bool>()) g_cfg.invert_x = *v;
        if (auto v = (*c)["invert_y"].value<bool>()) g_cfg.invert_y = *v;
        if (auto v = (*c)["vibration"].value<bool>()) g_cfg.vibration = *v;
        if (auto v = (*c)["vibration_strength"].value<double>()) g_cfg.vibration_strength = float(*v);
        if (auto* b = (*c)["buttons"].as_table())
            for (const Row& r : kPad)
                if (auto v = (*b)[r.id].value<std::string>()) g_cfg.pad[r.id] = *v;
    }
    if (auto* k = t["keys"].as_table())
        for (const Row& r : kKeys)
            if (auto v = (*k)[r.id].value<std::string>(); v && !v->empty()) g_cfg.keys[r.id] = *v;
    if (auto* m = t["mouse"].as_table()) {
        if (auto v = (*m)["keyboard_and_mouse"].value<bool>()) g_cfg.kb_enabled = *v;
        if (auto v = (*m)["menu_buttons"].value<bool>()) g_cfg.menu_buttons = *v;
        if (auto v = (*m)["invert_y"].value<bool>()) g_cfg.mouse_invert_y = *v;
        if (auto v = (*m)["sensitivity"].value<double>()) g_cfg.sensitivity = float(*v);
        if (auto v = (*m)["smoothing"].value<double>()) g_cfg.smoothing = float(*v);
    }
}

// a float setting as the file writes it, rounded so it reads back as the same short decimal
double Q(float v) { return std::round(double(v) * 10000.0) / 10000.0; }

void Save() {
    toml::table t;
    std::error_code ec;
    if (fs::exists(g_settings, ec)) {
        try {
            t = toml::parse_file(g_settings.string());  // keep what the game wrote and this panel does not show
        } catch (...) {
        }
    }
    t.insert_or_assign("launcher_language", g_lang == kEnglish ? "en" : "id");
    t.insert_or_assign("display_mode", kDisplayModes[g_cfg.display_mode]);
    t.insert_or_assign("width", (int64_t)g_cfg.width);
    t.insert_or_assign("height", (int64_t)g_cfg.height);
    t.insert_or_assign("render_percent", (int64_t)g_cfg.render_percent);
    t.insert_or_assign("fps_cap", (int64_t)g_cfg.fps_cap);
    t.insert_or_assign("vsync", g_cfg.vsync);
    t.insert_or_assign("anisotropic_filtering", (int64_t)kAnisoValues[g_cfg.aniso]);
    t.insert_or_assign("custom_textures", g_cfg.custom_textures);
    t.insert_or_assign("game_files", g_cfg.game_files);
    t.insert_or_assign("game_exe", g_cfg.game_exe);

    if (!t.contains("controller")) t.insert("controller", toml::table{});
    if (auto* c = t["controller"].as_table()) {
        c->insert_or_assign("camera_speed", Q(g_cfg.camera_speed));
        c->insert_or_assign("deadzone", Q(g_cfg.deadzone));
        c->insert_or_assign("invert_x", g_cfg.invert_x);
        c->insert_or_assign("invert_y", g_cfg.invert_y);
        c->insert_or_assign("vibration", g_cfg.vibration);
        c->insert_or_assign("vibration_strength", Q(g_cfg.vibration_strength));
        if (!c->contains("buttons")) c->insert("buttons", toml::table{});
        if (auto* b = (*c)["buttons"].as_table())
            for (auto& [id, v] : g_cfg.pad) b->insert_or_assign(id, v);
    }
    toml::table keys;
    for (auto& [id, v] : g_cfg.keys)
        if (!v.empty()) keys.insert_or_assign(id, v);
    t.insert_or_assign("keys", std::move(keys));
    if (!t.contains("mouse")) t.insert("mouse", toml::table{});
    if (auto* m = t["mouse"].as_table()) {
        m->insert_or_assign("keyboard_and_mouse", g_cfg.kb_enabled);
        m->insert_or_assign("menu_buttons", g_cfg.menu_buttons);
        m->insert_or_assign("invert_y", g_cfg.mouse_invert_y);
        m->insert_or_assign("sensitivity", Q(g_cfg.sensitivity));
        m->insert_or_assign("smoothing", Q(g_cfg.smoothing));
    }
    std::ofstream f(g_settings);
    if (!f) {
        g_status = std::string(T("status_write_failed")) + g_settings.string();
        return;
    }
    f << t << "\n";
    g_dirty = false;
}

HWND g_launcher_hwnd = nullptr;

// the launcher owns the foreground when Play is pressed: hand it to the game, or the keyboard keeps typing into a
// hidden launcher window and the game looks deaf
BOOL CALLBACK FocusGameProc(HWND w, LPARAM lp) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (pid != DWORD(lp) || !IsWindowVisible(w)) return TRUE;
    wchar_t cls[64] = {0};
    GetClassNameW(w, cls, 63);
    if (wcscmp(cls, L"ConsoleWindowClass") == 0) return TRUE;
    ShowWindow(w, SW_RESTORE);
    SetForegroundWindow(w);
    return FALSE;
}

void FocusGameWindow(DWORD pid) {
    if (g_launcher_hwnd) ShowWindow(g_launcher_hwnd, SW_MINIMIZE);
    for (int i = 0; i < 60; i++) {  // the game creates its window a moment later, or after its first-run compile
        if (!EnumWindows(FocusGameProc, LPARAM(pid))) return;
        Sleep(100);
    }
}
void Launch() {
    fs::path exe = GameExe();
    if (exe.empty()) {
        g_status = T("status_exe_missing");
        return;
    }
    g_cfg.game_exe = Narrow(exe);
    Save();
    std::wstring cmd = L"\"" + exe.wstring() + L"\"";
    std::vector<wchar_t> cmdline(cmd.begin(), cmd.end());
    cmdline.push_back(L'\0');
    STARTUPINFOW si{};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi{};
    std::wstring dir = exe.parent_path().wstring();
    if (!CreateProcessW(nullptr, cmdline.data(), nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &si, &pi)) {
        g_status = std::string(T("status_launch_failed")) + std::to_string(GetLastError()) + ")";
        return;
    }
    FocusGameWindow(pi.dwProcessId);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    g_status = std::string(T("status_launched")) + exe.filename().string();
}

bool Field(const char* label, std::string& value) {
    char buf[512];
    std::snprintf(buf, sizeof buf, "%s", value.c_str());
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::InputText(label, buf, sizeof buf)) {
        value = buf;
        g_dirty = true;
        return true;
    }
    return false;
}

// the game's own defaults (src/game/kbm.cpp kActions): shown when a binding is absent, and restored by the reset button
const char* KeyDefault(const char* id) {
    static const Row defaults[] = {
        {"a", "Mouse2,Enter"},      {"b", "E,Backspace"},        {"x", "Mouse1"},
        {"y", "Space"},             {"black", "F"},              {"white", "R"},
        {"left_trigger", "LShift"}, {"right_trigger", "Q"},      {"dpad_up", "Up,1"},
        {"dpad_down", "Down,2"},    {"dpad_left", "Left,3"},     {"dpad_right", "Right,4"},
        {"start", "Escape"},        {"back", "Tab"},             {"left_thumb", "C"},
        {"right_thumb", "Mouse3,V"},{"forward", "W"},            {"backward", "S"},
        {"left", "A"},              {"right", "D"},              {"walk", "LAlt"},
        {"look_up", ""},            {"look_down", ""},           {"look_left", ""},
        {"look_right", ""},         {"rage", "Z"},
    };
    for (const Row& r : defaults)
        if (std::strcmp(r.id, id) == 0) return r.text;
    return "";
}
// ---------------------------------------------------------------- look and feel

constexpr ImU32 kAccent = IM_COL32(226, 69, 43, 255);
constexpr ImU32 kAccentHot = IM_COL32(242, 84, 55, 255);
constexpr ImU32 kText = IM_COL32(236, 229, 219, 255);
constexpr ImU32 kMuted = IM_COL32(152, 141, 130, 255);
constexpr ImU32 kLine = IM_COL32(58, 47, 41, 255);
constexpr ImU32 kCard = IM_COL32(16, 12, 11, 150);

ImFont* g_font_body = nullptr;
ImFont* g_font_bold = nullptr;
ImFont* g_font_title = nullptr;

ImU32 Alpha(ImU32 c, float a) {
    return (c & 0x00FFFFFFu) | (ImU32(std::clamp(a, 0.0f, 1.0f) * 255.0f) << 24);
}

ImVec4 ToVec4(ImU32 c) {
    return ImVec4(((c >> 0) & 255) / 255.0f, ((c >> 8) & 255) / 255.0f, ((c >> 16) & 255) / 255.0f,
                  ((c >> 24) & 255) / 255.0f);
}

void ApplyTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding = ImVec2(0, 0);
    s.FramePadding = ImVec2(12, 7);
    s.ItemSpacing = ImVec2(10, 10);
    s.ItemInnerSpacing = ImVec2(8, 6);
    s.WindowRounding = 0.0f;
    s.ChildRounding = 12.0f;
    s.FrameRounding = 7.0f;
    s.PopupRounding = 8.0f;
    s.GrabRounding = 7.0f;
    s.ScrollbarRounding = 8.0f;
    s.WindowBorderSize = 0.0f;
    s.ChildBorderSize = 1.0f;
    s.FrameBorderSize = 0.0f;
    s.ScrollbarSize = 12.0f;

    s.Colors[ImGuiCol_Text] = ToVec4(kText);
    s.Colors[ImGuiCol_TextDisabled] = ToVec4(kMuted);
    s.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_PopupBg] = ToVec4(IM_COL32(20, 16, 14, 252));
    s.Colors[ImGuiCol_Border] = ToVec4(kLine);
    s.Colors[ImGuiCol_FrameBg] = ToVec4(IM_COL32(33, 26, 23, 255));
    s.Colors[ImGuiCol_FrameBgHovered] = ToVec4(IM_COL32(44, 34, 30, 255));
    s.Colors[ImGuiCol_FrameBgActive] = ToVec4(IM_COL32(54, 41, 36, 255));
    s.Colors[ImGuiCol_Button] = ToVec4(IM_COL32(35, 28, 25, 255));
    s.Colors[ImGuiCol_ButtonHovered] = ToVec4(IM_COL32(48, 37, 32, 255));
    s.Colors[ImGuiCol_ButtonActive] = ToVec4(IM_COL32(60, 46, 39, 255));
    s.Colors[ImGuiCol_Header] = ToVec4(Alpha(kAccent, 0.22f));
    s.Colors[ImGuiCol_HeaderHovered] = ToVec4(Alpha(kAccent, 0.30f));
    s.Colors[ImGuiCol_HeaderActive] = ToVec4(Alpha(kAccent, 0.38f));
    s.Colors[ImGuiCol_SliderGrab] = ToVec4(kAccent);
    s.Colors[ImGuiCol_SliderGrabActive] = ToVec4(kAccentHot);
    s.Colors[ImGuiCol_CheckMark] = ToVec4(kAccentHot);
    s.Colors[ImGuiCol_Separator] = ToVec4(kLine);
    s.Colors[ImGuiCol_ScrollbarBg] = ToVec4(IM_COL32(14, 11, 10, 140));
    s.Colors[ImGuiCol_ScrollbarGrab] = ToVec4(IM_COL32(60, 47, 41, 255));
    s.Colors[ImGuiCol_ScrollbarGrabHovered] = ToVec4(IM_COL32(78, 61, 52, 255));
    s.Colors[ImGuiCol_ScrollbarGrabActive] = ToVec4(kAccent);
}

ImFont* AddFontFile(const char* file, float px) {
    std::error_code ec;
    if (!fs::exists(file, ec)) return nullptr;
    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = true;
    return ImGui::GetIO().Fonts->AddFontFromFileTTF(file, px, &cfg);
}

// Segoe UI for text, Impact for the wordmark if it is installed, else the built-in font.
void LoadFonts() {
    const std::string dir = "C:/Windows/Fonts/";
    g_font_body = AddFontFile((dir + "segoeui.ttf").c_str(), 17.0f);
    g_font_bold = AddFontFile((dir + "seguisb.ttf").c_str(), 17.0f);
    if (!g_font_bold) g_font_bold = AddFontFile((dir + "segoeuib.ttf").c_str(), 17.0f);
    g_font_title = AddFontFile((dir + "impact.ttf").c_str(), 54.0f);
    if (g_font_title) ImGui::GetIO().Fonts->AddFontDefault();
    if (g_font_body) ImGui::GetIO().FontDefault = g_font_body;
}

void SectionTitle(const char* text) {
    ImGui::Dummy(ImVec2(0, 4));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    dl->AddRectFilled(ImVec2(p.x, p.y + 2), ImVec2(p.x + 3, p.y + 20), kAccent, 2.0f);
    ImFont* f = g_font_bold ? g_font_bold : ImGui::GetFont();
    dl->AddText(f, f->FontSize, ImVec2(p.x + 12, p.y), kText, text);
    ImGui::Dummy(ImVec2(0, f->FontSize + 4));
}

void Hint(const char* text) {
    ImGui::PushStyleColor(ImGuiCol_Text, ToVec4(kMuted));
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 2));
}

// a label on the left, its control right aligned; close with RowEnd()
void SettingRow(const char* label, float control_width = 250.0f) {
    ImGui::PushID(label);
    ImGui::AlignTextToFramePadding();
    ImGui::PushStyleColor(ImGuiCol_Text, ToVec4(kText));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    float x = ImGui::GetWindowContentRegionMax().x - control_width;
    if (x > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(x);
    ImGui::SetNextItemWidth(control_width);
}

inline void RowEnd() { ImGui::PopID(); }

bool AccentButton(const char* label, ImVec2 size) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool pressed = ImGui::InvisibleButton(label, size);
    bool hovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), held ? IM_COL32(188, 55, 34, 255)
                                                     : hovered ? kAccentHot
                                                               : kAccent,
                      9.0f);
    ImFont* f = g_font_bold ? g_font_bold : ImGui::GetFont();
    ImVec2 t = f->CalcTextSizeA(f->FontSize, FLT_MAX, 0.0f, label);
    dl->AddText(f, f->FontSize, ImVec2(p.x + (size.x - t.x) * 0.5f, p.y + (size.y - t.y) * 0.5f),
                IM_COL32(255, 246, 240, 255), label);
    return pressed;
}

bool GhostButton(const char* label, ImVec2 size) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool pressed = ImGui::InvisibleButton(label, size);
    bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y),
                      hovered ? IM_COL32(44, 35, 30, 255) : IM_COL32(28, 22, 19, 235), 9.0f);
    dl->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), kLine, 9.0f);
    ImFont* f = g_font_body ? g_font_body : ImGui::GetFont();
    ImVec2 t = f->CalcTextSizeA(f->FontSize, FLT_MAX, 0.0f, label);
    dl->AddText(f, f->FontSize, ImVec2(p.x + (size.x - t.x) * 0.5f, p.y + (size.y - t.y) * 0.5f), kText, label);
    return pressed;
}

bool NavItem(const char* id, const char* label, bool active) {
    ImGui::PushID(id);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    const float h = 44.0f;
    bool clicked = ImGui::InvisibleButton("##nav", ImVec2(w, h));
    bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (active) {
        dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), Alpha(kAccent, 0.16f), 9.0f);
        dl->AddRectFilled(ImVec2(p.x, p.y + 10), ImVec2(p.x + 3, p.y + h - 10), kAccent, 2.0f);
    } else if (hovered) {
        dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), IM_COL32(255, 255, 255, 12), 9.0f);
    }
    ImFont* f = active && g_font_bold ? g_font_bold : (g_font_body ? g_font_body : ImGui::GetFont());
    dl->AddText(f, f->FontSize, ImVec2(p.x + 18, p.y + (h - f->FontSize) * 0.5f), active ? kText : kMuted, label);
    ImGui::PopID();
    return clicked;
}

// the wordmark, drawn per glyph so it can be tracked out like a poster title
void DrawWordmark(ImDrawList* dl, ImVec2 pos, float size) {
    ImFont* f = g_font_title ? g_font_title : (g_font_bold ? g_font_bold : ImGui::GetFont());
    const char* text = "THE WARRIORS";
    float x = pos.x;
    const float tracking = size * 0.045f;
    for (const char* c = text; *c; ++c) {
        char ch[2] = {*c, 0};
        dl->AddText(f, size, ImVec2(x, pos.y), kText, ch);
        x += f->CalcTextSizeA(size, FLT_MAX, 0.0f, ch).x + tracking;
    }
    dl->AddRectFilled(ImVec2(pos.x, pos.y + size + 12), ImVec2(pos.x + 72, pos.y + size + 15), kAccent, 1.5f);
}

void DrawTracked(ImDrawList* dl, ImVec2 pos, const char* text, float size, ImU32 color, float tracking) {
    ImFont* f = g_font_body ? g_font_body : ImGui::GetFont();
    float x = pos.x;
    for (const char* c = text; *c; ++c) {
        char ch[2] = {*c, 0};
        dl->AddText(f, size, ImVec2(x, pos.y), color, ch);
        x += f->CalcTextSizeA(size, FLT_MAX, 0.0f, ch).x + tracking;
    }
}

// ---------------------------------------------------------------- pages

void GraphicsTab() {
    SectionTitle(T("graphics"));
    SettingRow(T("display_mode"));
    if (ImGui::Combo("##display_mode", &g_cfg.display_mode, kDisplayModes, 3)) g_dirty = true;
    RowEnd();
    SettingRow(T("width"));
    if (ImGui::InputInt("##width", &g_cfg.width)) g_dirty = true;
    RowEnd();
    SettingRow(T("height"));
    if (ImGui::InputInt("##height", &g_cfg.height)) g_dirty = true;
    RowEnd();
    SettingRow(T("render_scale"));
    if (ImGui::SliderInt("##render_scale", &g_cfg.render_percent, 30, 100)) g_dirty = true;
    RowEnd();
    Hint(T("render_hint"));
    SettingRow(T("fps_cap"));
    if (ImGui::InputInt("##fps_cap", &g_cfg.fps_cap)) g_dirty = true;
    RowEnd();
    SettingRow(T("vsync"));
    if (ImGui::Checkbox("##vsync", &g_cfg.vsync)) g_dirty = true;
    RowEnd();
    SettingRow(T("aniso"));
    if (ImGui::Combo("##aniso", &g_cfg.aniso, kAnisoNames, 6)) g_dirty = true;
    RowEnd();
    SettingRow(T("custom_tex"));
    if (ImGui::Checkbox("##custom_tex", &g_cfg.custom_textures)) g_dirty = true;
    RowEnd();
}

void ControllerTab() {
    SectionTitle(T("controller"));
    Hint(T("controller_note"));
    SettingRow(T("camera_speed"));
    if (ImGui::SliderFloat("##camera_speed", &g_cfg.camera_speed, 0.2f, 3.0f, "%.2f")) g_dirty = true;
    RowEnd();
    SettingRow(T("deadzone"));
    if (ImGui::SliderFloat("##deadzone", &g_cfg.deadzone, 0.0f, 0.8f, "%.2f")) g_dirty = true;
    RowEnd();
    SettingRow(T("invert_x"));
    if (ImGui::Checkbox("##invert_x", &g_cfg.invert_x)) g_dirty = true;
    RowEnd();
    SettingRow(T("invert_y"));
    if (ImGui::Checkbox("##invert_y", &g_cfg.invert_y)) g_dirty = true;
    RowEnd();
    SettingRow(T("vibration"));
    if (ImGui::Checkbox("##vibration", &g_cfg.vibration)) g_dirty = true;
    RowEnd();
    SettingRow(T("vibration_strength"));
    if (ImGui::SliderFloat("##vibration_strength", &g_cfg.vibration_strength, 0.0f, 1.0f, "%.2f")) g_dirty = true;
    RowEnd();

    ImGui::Dummy(ImVec2(0, 6));
    SectionTitle(T("padmap"));
    for (const Row& r : kPad) {
        std::string& cur = g_cfg.pad[r.id];
        if (cur.empty()) cur = r.id;
        int idx = 0;
        for (int i = 0; i < int(std::size(kPad)); i++)
            if (cur == kPad[i].id) idx = i;
        SettingRow(r.text);
        if (ImGui::Combo("##pad", &idx, [](void* data, int i) { return static_cast<const Row*>(data)[i].text; },
                         (void*)kPad, int(std::size(kPad)))) {
            cur = kPad[idx].id;
            g_dirty = true;
        }
        RowEnd();
    }
}

void KeyboardTab() {
    SectionTitle(T("keyboard"));
    SettingRow(T("kb_enable"));
    if (ImGui::Checkbox("##kb_enable", &g_cfg.kb_enabled)) g_dirty = true;
    RowEnd();
    SettingRow(T("sensitivity"));
    if (ImGui::SliderFloat("##sensitivity", &g_cfg.sensitivity, 0.05f, 5.0f, "%.2f")) g_dirty = true;
    RowEnd();
    SettingRow(T("smoothing"));
    if (ImGui::SliderFloat("##smoothing", &g_cfg.smoothing, 0.0f, 0.9f, "%.2f")) g_dirty = true;
    RowEnd();
    SettingRow(T("invert_y_mouse"));
    if (ImGui::Checkbox("##invert_y_mouse", &g_cfg.mouse_invert_y)) g_dirty = true;
    RowEnd();
    SettingRow(T("menu_buttons"));
    if (ImGui::Checkbox("##menu_buttons", &g_cfg.menu_buttons)) g_dirty = true;
    RowEnd();
    Hint(T("key_names"));
    Hint(T("key_pairs"));

    ImGui::Dummy(ImVec2(0, 6));
    SectionTitle(T("bindings"));
    if (GhostButton(T("reset_keys"), ImVec2(200, 36))) {
        g_cfg.keys.clear();
        g_dirty = true;
    }
    ImGui::Dummy(ImVec2(0, 6));
    for (const Row& r : kKeys) {
        auto it = g_cfg.keys.find(r.id);
        std::string shown = it != g_cfg.keys.end() ? it->second : KeyDefault(r.id);
        SettingRow(T(r.text));
        if (Field("##key", shown)) g_cfg.keys[r.id] = shown;
        RowEnd();
    }
}

void GameFilesTab() {
    SectionTitle(T("files"));
    Hint(T("files_note"));

    const bool custom = !g_cfg.game_files.empty();
    const char* state = custom ? GameFilesState(fs::u8path(g_cfg.game_files)) : nullptr;
    const bool ok = state && (std::strcmp(state, "files_ok_xbe") == 0 || std::strcmp(state, "files_ok_iso") == 0);
    ImGui::Dummy(ImVec2(0, 4));
    if (custom)
        ImGui::PushStyleColor(ImGuiCol_Text, ok ? ImVec4(0.45f, 0.85f, 0.45f, 1.0f) : ImVec4(0.95f, 0.52f, 0.40f, 1.0f));
    ImGui::TextWrapped("%s: %s", T("data_folder"), custom ? g_cfg.game_files.c_str() : T("data_default"));
    if (custom) ImGui::PopStyleColor();
    if (custom) ImGui::TextDisabled("%s: %s", T("status"), T(state));

    ImGui::Dummy(ImVec2(0, 6));
    if (GhostButton(T("pick_folder"), ImVec2(230, 38))) {
        std::string picked = PickFolder();
        if (!picked.empty()) {
            g_cfg.game_files = picked;
            g_dirty = true;
        }
    }
    ImGui::SameLine();
    if (GhostButton(T("clear_folder"), ImVec2(300, 38))) {
        g_cfg.game_files.clear();
        g_dirty = true;
    }

    ImGui::Dummy(ImVec2(0, 10));
    SectionTitle(T("exe_label"));
    fs::path exe = GameExe();
    const std::string exeText = exe.empty() ? std::string(T("exe_missing")) : Narrow(exe);
    ImGui::TextWrapped("%s", exeText.c_str());
    ImGui::Dummy(ImVec2(0, 6));
    if (GhostButton(T("pick_exe"), ImVec2(230, 38))) {
        std::string picked = PickFolder();
        if (!picked.empty()) {
            g_cfg.game_exe = picked;
            g_dirty = true;
        }
    }
    ImGui::SameLine();
    if (GhostButton(T("detect"), ImVec2(150, 38))) {
        g_cfg.game_exe.clear();
        g_dirty = true;
    }

    ImGui::Dummy(ImVec2(0, 10));
    SectionTitle(T("settings_path"));
    ImGui::TextWrapped("%s", g_settings.string().c_str());
    Hint(T("settings_note"));
}

// ---------------------------------------------------------------- D3D11 + ImGui

ID3D11Device* g_dev = nullptr;
ID3D11DeviceContext* g_ctx = nullptr;
IDXGISwapChain* g_swap = nullptr;
ID3D11RenderTargetView* g_rtv = nullptr;
bool g_occluded = false;

ID3D11ShaderResourceView* g_backdrop = nullptr;
int g_backdrop_w = 0, g_backdrop_h = 0;

struct Building {
    float x, w, h;
};
std::vector<Building> g_skyline;

void BuildSkyline() {
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    float x = 0.0f;
    while (x < 1.02f) {
        float w = 0.018f + u(rng) * 0.045f;
        float h = 0.10f + u(rng) * 0.34f;
        g_skyline.push_back({x, w, h});
        x += w + 0.005f;
    }
}

void CreateTarget() {
    ID3D11Texture2D* back = nullptr;
    if (SUCCEEDED(g_swap->GetBuffer(0, IID_PPV_ARGS(&back)))) {
        g_dev->CreateRenderTargetView(back, nullptr, &g_rtv);
        back->Release();
    }
}

bool CreateDevice(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL want[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL got;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, want, 2,
                                             D3D11_SDK_VERSION, &sd, &g_swap, &g_dev, &got, &g_ctx)))
        return false;
    CreateTarget();
    return g_rtv != nullptr;
}

// a PNG or JPG through WIC, for the player's own backdrop
ID3D11ShaderResourceView* LoadTexture(const fs::path& path, int* out_w, int* out_h) {
    IWICImagingFactory* factory = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))))
        return nullptr;
    IWICBitmapDecoder* dec = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* conv = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    if (SUCCEEDED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
                                                     &dec)) &&
        SUCCEEDED(dec->GetFrame(0, &frame)) && SUCCEEDED(factory->CreateFormatConverter(&conv)) &&
        SUCCEEDED(conv->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0,
                                   WICBitmapPaletteTypeCustom))) {
        UINT w = 0, h = 0;
        conv->GetSize(&w, &h);
        if (w && h) {
            std::vector<uint8_t> px(size_t(w) * h * 4);
            if (SUCCEEDED(conv->CopyPixels(nullptr, w * 4, UINT(px.size()), px.data()))) {
                D3D11_TEXTURE2D_DESC d{};
                d.Width = w;
                d.Height = h;
                d.MipLevels = 1;
                d.ArraySize = 1;
                d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                d.SampleDesc.Count = 1;
                d.Usage = D3D11_USAGE_IMMUTABLE;
                d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                D3D11_SUBRESOURCE_DATA sd{};
                sd.pSysMem = px.data();
                sd.SysMemPitch = w * 4;
                ID3D11Texture2D* tex = nullptr;
                if (SUCCEEDED(g_dev->CreateTexture2D(&d, &sd, &tex))) {
                    g_dev->CreateShaderResourceView(tex, nullptr, &srv);
                    tex->Release();
                    *out_w = int(w);
                    *out_h = int(h);
                }
            }
        }
    }
    if (conv) conv->Release();
    if (frame) frame->Release();
    if (dec) dec->Release();
    factory->Release();
    return srv;
}

// background.png / .jpg beside the executable: the player's own artwork. Falls back to a drawn backdrop.
void LoadBackdrop() {
    const char* names[] = {"background.png", "background.jpg", "background.jpeg", "bg.png", "bg.jpg",
                           "launcher-bg.png", "launcher-bg.jpg"};
    for (const char* n : names) {
        std::error_code ec;
        fs::path p = ExeDir() / n;
        if (!fs::exists(p, ec)) continue;
        g_backdrop = LoadTexture(p, &g_backdrop_w, &g_backdrop_h);
        if (g_backdrop) return;
    }
}

void DrawBackdrop(const ImVec2& size) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    if (g_backdrop && g_backdrop_w && g_backdrop_h) {
        const float image_aspect = float(g_backdrop_w) / float(g_backdrop_h);
        const float screen_aspect = size.x / size.y;
        ImVec2 uv0(0, 0), uv1(1, 1);
        if (image_aspect > screen_aspect) {
            const float f = screen_aspect / image_aspect;
            uv0.x = (1.0f - f) * 0.5f;
            uv1.x = 1.0f - uv0.x;
        } else {
            const float f = image_aspect / screen_aspect;
            uv0.y = (1.0f - f) * 0.5f;
            uv1.y = 1.0f - uv0.y;
        }
        dl->AddImage((ImTextureID)(intptr_t)g_backdrop, ImVec2(0, 0), size, uv0, uv1);
        dl->AddRectFilledMultiColor(ImVec2(0, 0), size, IM_COL32(10, 8, 7, 140), IM_COL32(10, 8, 7, 110),
                                    IM_COL32(10, 8, 7, 240), IM_COL32(10, 8, 7, 240));
        dl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(300, size.y), IM_COL32(10, 8, 7, 235), IM_COL32(10, 8, 7, 40),
                                    IM_COL32(10, 8, 7, 40), IM_COL32(10, 8, 7, 235));
        return;
    }

    dl->AddRectFilledMultiColor(ImVec2(0, 0), size, IM_COL32(26, 19, 16, 255), IM_COL32(22, 16, 15, 255),
                                IM_COL32(10, 8, 7, 255), IM_COL32(9, 7, 6, 255));
    const ImVec2 glow(size.x * 0.70f, size.y * 0.34f);
    for (int i = 14; i > 0; i--) {
        const float r = 30.0f + float(i) * 30.0f;
        dl->AddCircleFilled(glow, r, IM_COL32(226, 69, 43, int(2.0f + float(i) * 0.9f)), 64);
    }
    for (const Building& b : g_skyline) {
        const float x0 = b.x * size.x;
        const float x1 = (b.x + b.w) * size.x;
        const float top = size.y - b.h * size.y * 0.62f;
        dl->AddRectFilled(ImVec2(x0, top), ImVec2(x1, size.y), IM_COL32(6, 5, 5, 255));
        for (int wy = 0; wy < 5; wy++) {
            for (int wx = 0; wx < 3; wx++) {
                if (((wx * 7 + wy * 3 + int(b.x * 100)) % 5) != 0) continue;
                const float px = x0 + (b.w * size.x) * (0.18f + 0.28f * float(wx));
                const float py = top + 14.0f + float(wy) * 16.0f;
                dl->AddRectFilled(ImVec2(px, py), ImVec2(px + 2.0f, py + 3.0f), IM_COL32(226, 120, 60, 70));
            }
        }
    }
    for (int i = 0; i < 5; i++) {
        const float x = size.x * (0.15f + float(i) * 0.18f);
        dl->AddLine(ImVec2(x, size.y), ImVec2(x + size.x * 0.35f, 0), IM_COL32(255, 255, 255, 5), 22.0f);
    }
}

void DestroyTarget() {
    if (g_rtv) g_rtv->Release();
    g_rtv = nullptr;
}

void DestroyDevice() {
    DestroyTarget();
    if (g_backdrop) g_backdrop->Release();
    g_backdrop = nullptr;
    if (g_swap) g_swap->Release();
    if (g_ctx) g_ctx->Release();
    if (g_dev) g_dev->Release();
    g_swap = nullptr;
    g_ctx = nullptr;
    g_dev = nullptr;
}

LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp)) return true;
    switch (msg) {
        case WM_SIZE:
            if (g_dev && wp != SIZE_MINIMIZED) {
                DestroyTarget();
                g_swap->ResizeBuffers(0, UINT(LOWORD(lp)), UINT(HIWORD(lp)), DXGI_FORMAT_UNKNOWN, 0);
                CreateTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wp & 0xFFF0) == SC_KEYMENU) return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int Run() {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = L"TheWarriorsLauncher";
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"The Warriors - Launcher", WS_OVERLAPPEDWINDOW, 120, 80, 1120, 720,
                              nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) return 1;
    if (!CreateDevice(hwnd)) {
        MessageBoxW(hwnd, L"Launcher needs Direct3D 11. Update the GPU driver.", L"The Warriors", MB_ICONERROR);
        DestroyWindow(hwnd);
        return 1;
    }
    g_launcher_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ApplyTheme();
    LoadFonts();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_dev, g_ctx);
    LoadBackdrop();
    BuildSkyline();

    const ImVec2 sidebar_size(272.0f, 0);
    const float footer = 92.0f;
    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) running = false;
        }
        if (!running) break;
        if (g_occluded) {
            Sleep(30);
            continue;
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        const ImVec2 display = ImGui::GetIO().DisplaySize;
        DrawBackdrop(display);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(display);
        ImGui::Begin("##root", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ---- sidebar
        ImGui::SetCursorPos(ImVec2(24, 26));
        ImGui::BeginGroup();
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 origin = ImGui::GetCursorScreenPos();
            DrawWordmark(dl, ImVec2(origin.x, origin.y), 42.0f);
            DrawTracked(dl, ImVec2(origin.x, origin.y + 62), "UNOFFICIAL PC PORT", 12.0f, Alpha(kMuted, 0.95f), 2.4f);
        }
        ImGui::EndGroup();
        ImGui::Dummy(ImVec2(1, 88));

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6));
        static const char* tab_keys[] = {"tab_graphics", "tab_controller", "tab_keyboard", "tab_files"};
        for (int i = 0; i < 4; i++)
            if (NavItem(tab_keys[i], T(tab_keys[i]), g_tab == i)) g_tab = i;
        ImGui::PopStyleVar();

        // language + version, pinned to the bottom of the sidebar
        const float side_bottom = display.y - footer - 56.0f;
        ImGui::SetCursorPos(ImVec2(24, std::max(side_bottom, 380.0f)));
        ImGui::TextDisabled("%s", T("language"));
        ImGui::SetCursorPosX(24);
        ImGui::SetNextItemWidth(200.0f);
        const char* langs[] = {"Indonesia", "English"};
        int lang = int(g_lang);
        if (ImGui::Combo("##lang", &lang, langs, 2)) {
            g_lang = Lang(lang);
            g_dirty = true;
        }

        // ---- content
        ImGui::SetCursorPos(ImVec2(sidebar_size.x + 8, 24));
        ImGui::BeginChild("##content", ImVec2(display.x - sidebar_size.x - 32, display.y - footer - 24),
                          ImGuiChildFlags_None);
        {
            static const char* desc_keys[] = {"desc_graphics", "desc_controller", "desc_keyboard", "desc_files"};
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImFont* f = g_font_bold ? g_font_bold : ImGui::GetFont();
            ImVec2 p = ImGui::GetCursorScreenPos();
            dl->AddText(f, 26.0f, p, kText, T(tab_keys[g_tab]));
            ImGui::Dummy(ImVec2(0, 34));
            Hint(T(desc_keys[g_tab]));
            ImGui::Dummy(ImVec2(0, 4));

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ToVec4(kCard));
            ImGui::BeginChild("##card", ImVec2(0, 0), ImGuiChildFlags_Border);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18, 14));
            switch (g_tab) {
                case 0: GraphicsTab(); break;
                case 1: ControllerTab(); break;
                case 2: KeyboardTab(); break;
                default: GameFilesTab(); break;
            }
            ImGui::PopStyleVar();
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        // ---- footer
        ImGui::SetCursorPos(ImVec2(0, display.y - footer));
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 bar = ImGui::GetCursorScreenPos();
        dl->AddLine(ImVec2(bar.x + 24, bar.y), ImVec2(display.x - 24, bar.y), kLine, 1.0f);

        ImGui::SetCursorPos(ImVec2(sidebar_size.x + 8, display.y - footer + 24));
        if (AccentButton(T("play"), ImVec2(190, 46))) Launch();
        ImGui::SameLine(0, 12);
        if (GhostButton(T("save"), ImVec2(120, 46))) {
            Save();
            g_status = T("status_saved");
        }
        ImGui::SameLine(0, 20);
        ImGui::SetCursorPosY(display.y - footer + 38);
        if (g_dirty) ImGui::TextColored(ImVec4(0.95f, 0.62f, 0.35f, 1.0f), "%s", T("dirty"));
        else ImGui::TextDisabled("%s", g_status.c_str());

        char credit[256];
        std::snprintf(credit, sizeof credit, "%s  -  v%s", T("watermark"), kVersion);
        ImVec2 cs = ImGui::CalcTextSize(credit);
        ImGui::SetCursorPos(ImVec2(display.x - cs.x - 24, display.y - footer + 40));
        ImGui::TextDisabled("%s", credit);

        ImGui::End();

        ImGui::Render();
        const float clear[4] = {0.06f, 0.05f, 0.05f, 1.0f};
        g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_ctx->ClearRenderTargetView(g_rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swap->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    DestroyDevice();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

}  // namespace

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    SetProcessDPIAware();
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    fs::path exe = GameExe();
    g_settings = (exe.empty() ? ExeDir() : exe.parent_path()) / "settings.toml";
    Load();
    if (std::wstring(GetCommandLineW()).find(L"--selftest") != std::wstring::npos) {
        Save();
        CoUninitialize();
        return 0;
    }
    int code = Run();
    CoUninitialize();
    return code;
}
