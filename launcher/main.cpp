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

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <toml++/toml.hpp>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "ole32.lib")

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

    {"watermark", "Port PC tidak resmi — tidak berafiliasi dengan Rockstar Games",
     "Unofficial PC port — not affiliated with Rockstar Games"},

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
            if (auto v = (*k)[r.id].value<std::string>()) g_cfg.keys[r.id] = *v;
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
    if (!t.contains("keys")) t.insert("keys", toml::table{});
    if (auto* k = t["keys"].as_table())
        for (auto& [id, v] : g_cfg.keys) k->insert_or_assign(id, v);
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

void GraphicsTab() {
    ImGui::TextUnformatted(T("graphics"));
    ImGui::Separator();
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::Combo(T("display_mode"), &g_cfg.display_mode, kDisplayModes, 3)) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::InputInt(T("width"), &g_cfg.width)) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::InputInt(T("height"), &g_cfg.height)) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::SliderInt(T("render_scale"), &g_cfg.render_percent, 30, 100)) g_dirty = true;
    ImGui::TextDisabled("%s", T("render_hint"));
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::InputInt(T("fps_cap"), &g_cfg.fps_cap)) g_dirty = true;
    if (ImGui::Checkbox(T("vsync"), &g_cfg.vsync)) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::Combo(T("aniso"), &g_cfg.aniso, kAnisoNames, 6)) g_dirty = true;
    if (ImGui::Checkbox(T("custom_tex"), &g_cfg.custom_textures)) g_dirty = true;
}

void ControllerTab() {
    ImGui::TextUnformatted(T("controller"));
    ImGui::Separator();
    ImGui::TextDisabled("%s", T("controller_note"));
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::SliderFloat(T("camera_speed"), &g_cfg.camera_speed, 0.2f, 3.0f, "%.2f")) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::SliderFloat(T("deadzone"), &g_cfg.deadzone, 0.0f, 0.8f, "%.2f")) g_dirty = true;
    if (ImGui::Checkbox(T("invert_x"), &g_cfg.invert_x)) g_dirty = true;
    if (ImGui::Checkbox(T("invert_y"), &g_cfg.invert_y)) g_dirty = true;
    if (ImGui::Checkbox(T("vibration"), &g_cfg.vibration)) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::SliderFloat(T("vibration_strength"), &g_cfg.vibration_strength, 0.0f, 1.0f, "%.2f")) g_dirty = true;

    ImGui::Spacing();
    ImGui::TextUnformatted(T("padmap"));
    ImGui::Separator();
    ImGui::BeginChild("padmap", ImVec2(0, 0), true);
    for (const Row& r : kPad) {
        std::string& cur = g_cfg.pad[r.id];
        if (cur.empty()) cur = r.id;
        int idx = 0;
        for (int i = 0; i < int(std::size(kPad)); i++)
            if (cur == kPad[i].id) idx = i;
        ImGui::SetNextItemWidth(-260.0f);
        if (ImGui::Combo(r.text, &idx, [](void* data, int i) { return static_cast<const Row*>(data)[i].text; },
                         (void*)kPad, int(std::size(kPad)))) {
            cur = kPad[idx].id;
            g_dirty = true;
        }
    }
    ImGui::EndChild();
}

void KeyboardTab() {
    ImGui::TextUnformatted(T("keyboard"));
    ImGui::Separator();
    if (ImGui::Checkbox(T("kb_enable"), &g_cfg.kb_enabled)) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::SliderFloat(T("sensitivity"), &g_cfg.sensitivity, 0.05f, 5.0f, "%.2f")) g_dirty = true;
    ImGui::SetNextItemWidth(-260.0f);
    if (ImGui::SliderFloat(T("smoothing"), &g_cfg.smoothing, 0.0f, 0.9f, "%.2f")) g_dirty = true;
    if (ImGui::Checkbox(T("invert_y_mouse"), &g_cfg.mouse_invert_y)) g_dirty = true;
    if (ImGui::Checkbox(T("menu_buttons"), &g_cfg.menu_buttons)) g_dirty = true;
    ImGui::TextDisabled("%s", T("key_names"));
    ImGui::TextDisabled("%s", T("key_pairs"));

    ImGui::Spacing();
    ImGui::TextUnformatted(T("bindings"));
    ImGui::Separator();
    ImGui::BeginChild("keys", ImVec2(0, 0), true);
    for (const Row& r : kKeys) Field(T(r.text), g_cfg.keys[r.id]);
    ImGui::EndChild();
}

void GameFilesTab() {
    ImGui::TextUnformatted(T("files"));
    ImGui::Separator();
    ImGui::TextWrapped("%s", T("files_note"));

    ImGui::Spacing();
    const bool custom = !g_cfg.game_files.empty();
    const char* state = custom ? GameFilesState(fs::u8path(g_cfg.game_files)) : nullptr;
    const bool ok = state && (std::strcmp(state, "files_ok_xbe") == 0 || std::strcmp(state, "files_ok_iso") == 0);
    ImGui::Text("%s : %s", T("data_folder"), custom ? g_cfg.game_files.c_str() : T("data_default"));
    if (custom)
        ImGui::TextColored(ok ? ImVec4(0.4f, 0.9f, 0.4f, 1) : ImVec4(0.95f, 0.5f, 0.4f, 1), "%s : %s", T("status"),
                           T(state));
    if (ImGui::Button(T("pick_folder"))) {
        std::string picked = PickFolder();
        if (!picked.empty()) {
            g_cfg.game_files = picked;
            g_dirty = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(T("clear_folder"))) {
        g_cfg.game_files.clear();
        g_dirty = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    fs::path exe = GameExe();
    const std::string exeText = exe.empty() ? std::string(T("exe_missing")) : Narrow(exe);
    ImGui::Text("%s : %s", T("exe_label"), exeText.c_str());
    if (ImGui::Button(T("pick_exe"))) {
        std::string picked = PickFolder();
        if (!picked.empty()) {
            g_cfg.game_exe = picked;
            g_dirty = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(T("detect"))) {
        g_cfg.game_exe.clear();
        g_dirty = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s : %s", T("settings_path"), g_settings.string().c_str());
    ImGui::TextDisabled("%s", T("settings_note"));
}

// a faint brand watermark over the panel: the title behind the content, and the credit line above the button row.
// Drawn into the panel window's own draw list just before End(), which is the one place its coordinates are known.
void WatermarkInPanel() {
    const ImVec2 size = ImGui::GetIO().DisplaySize;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* font = ImGui::GetFont();

    const char* big = "THE WARRIORS";
    const float big_size = 68.0f;
    const ImVec2 bsz = font->CalcTextSizeA(big_size, FLT_MAX, 0.0f, big);
    dl->AddText(font, big_size, ImVec2((size.x - bsz.x) * 0.5f, size.y * 0.46f), IM_COL32(255, 255, 255, 46), big);

    char line[256];
    std::snprintf(line, sizeof line, "%s  ·  v%s", T("watermark"), kVersion);
    const ImVec2 ssz = ImGui::CalcTextSize(line);
    const ImVec2 at(size.x - ssz.x - 14.0f, size.y - ssz.y - 46.0f);  // above the button row
    dl->AddRectFilled(ImVec2(at.x - 8, at.y - 4), ImVec2(at.x + ssz.x + 8, at.y + ssz.y + 4), IM_COL32(0, 0, 0, 110),
                      4.0f);
    dl->AddText(at, IM_COL32(255, 255, 255, 200), line);
}

// ---------------------------------------------------------------- D3D11 + ImGui

ID3D11Device* g_dev = nullptr;
ID3D11DeviceContext* g_ctx = nullptr;
IDXGISwapChain* g_swap = nullptr;
ID3D11RenderTargetView* g_rtv = nullptr;
bool g_occluded = false;

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

void DestroyTarget() {
    if (g_rtv) g_rtv->Release();
    g_rtv = nullptr;
}

void DestroyDevice() {
    DestroyTarget();
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
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"The Warriors — Launcher", WS_OVERLAPPEDWINDOW, 120, 80, 1000, 700,
                              nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) return 1;
    if (!CreateDevice(hwnd)) {
        MessageBoxW(hwnd, L"Launcher needs Direct3D 11. Update the GPU driver.", L"The Warriors", MB_ICONERROR);
        DestroyWindow(hwnd);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_dev, g_ctx);

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

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("##root", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

        ImGui::TextUnformatted(T("app_title"));
        ImGui::SameLine();
        ImGui::TextDisabled("%s  v%s", T("app_sub"), kVersion);

        // language picker, right aligned on the header row
        const char* langs[] = {"Indonesia", "English"};
        float combo_w = 130.0f;
        float x = ImGui::GetIO().DisplaySize.x - combo_w - 150.0f;
        ImGui::SameLine(x);
        ImGui::TextUnformatted(T("language"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(combo_w);
        int lang = int(g_lang);
        if (ImGui::Combo("##lang", &lang, langs, 2)) {
            g_lang = Lang(lang);
            g_dirty = true;  // written with the rest of the settings
        }
        ImGui::Separator();

        const float footer = 62.0f;
        ImGui::BeginChild("tabs", ImVec2(180, ImGui::GetContentRegionAvail().y - footer), true);
        static const char* tab_keys[] = {"tab_graphics", "tab_controller", "tab_keyboard", "tab_files"};
        for (int i = 0; i < 4; i++)
            if (ImGui::Selectable(T(tab_keys[i]), g_tab == i, 0, ImVec2(0, 28))) g_tab = i;
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("page", ImVec2(0, ImGui::GetContentRegionAvail().y - footer), true);
        switch (g_tab) {
            case 0: GraphicsTab(); break;
            case 1: ControllerTab(); break;
            case 2: KeyboardTab(); break;
            default: GameFilesTab(); break;
        }
        ImGui::EndChild();

        if (ImGui::Button(T("play"), ImVec2(150, 36))) Launch();
        ImGui::SameLine();
        if (ImGui::Button(T("save"), ImVec2(110, 36))) {
            Save();
            g_status = T("status_saved");
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s%s%s", g_dirty ? T("dirty") : "", g_dirty && !g_status.empty() ? " — " : "",
                            g_status.c_str());

        WatermarkInPanel();
        ImGui::End();

        ImGui::Render();
        const float clear[4] = {0.08f, 0.08f, 0.10f, 1.0f};
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
    // physical pixels: with display scaling the window and ImGui's coordinates would otherwise disagree
    SetProcessDPIAware();
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    fs::path exe = GameExe();
    g_settings = (exe.empty() ? ExeDir() : exe.parent_path()) / "settings.toml";
    Load();
    // --selftest: read settings.toml, write it back and leave, no window. Checks the round trip keeps every key.
    if (std::wstring(GetCommandLineW()).find(L"--selftest") != std::wstring::npos) {
        Save();
        CoUninitialize();
        return 0;
    }
    int code = Run();
    CoUninitialize();
    return code;
}
