#include "RGBController_DarkProjectALU87A.h"
#include "RGBControllerKeyNames.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

#define NA  0xFFFFFFFF

namespace
{
struct KeyDef
{
    const char* name;
    uint8_t     hw_idx; // Index in Matrix_LEDCode (0..95)
};

// All 88 keys of DPKB_MIDNIGHT_87_ISO (0x342D:0xE40F)
static const KeyDef alu87a_iso_keys[] =
{
    /* Row 0: F-Row (16 keys) */
    { KEY_EN_ESCAPE,        0  },
    { KEY_EN_F1,            6  },
    { KEY_EN_F2,            12 },
    { KEY_EN_F3,            18 },
    { KEY_EN_F4,            24 },
    { KEY_EN_F5,            30 },
    { KEY_EN_F6,            36 },
    { KEY_EN_F7,            42 },
    { KEY_EN_F8,            48 },
    { KEY_EN_F9,            54 },
    { KEY_EN_F10,           60 },
    { KEY_EN_F11,           66 },
    { KEY_EN_F12,           72 },
    { KEY_EN_PRINT_SCREEN,  78 },
    { KEY_EN_SCROLL_LOCK,   84 },
    { KEY_EN_PAUSE_BREAK,   90 },

    /* Row 1: Number Row (17 keys) */
    { KEY_EN_BACK_TICK,     1  },
    { KEY_EN_1,             7  },
    { KEY_EN_2,             13 },
    { KEY_EN_3,             19 },
    { KEY_EN_4,             25 },
    { KEY_EN_5,             31 },
    { KEY_EN_6,             37 },
    { KEY_EN_7,             43 },
    { KEY_EN_8,             49 },
    { KEY_EN_9,             55 },
    { KEY_EN_0,             61 },
    { KEY_EN_MINUS,         67 },
    { KEY_EN_EQUALS,        73 },
    { KEY_EN_BACKSPACE,     79 },
    { KEY_EN_INSERT,        85 },
    { KEY_EN_HOME,          91 },
    { KEY_EN_PAGE_UP,       87 },

    /* Row 2: QWERTY Row (17 keys) */
    { KEY_EN_TAB,           2  },
    { KEY_EN_Q,             8  },
    { KEY_EN_W,             14 },
    { KEY_EN_E,             20 },
    { KEY_EN_R,             26 },
    { KEY_EN_T,             32 },
    { KEY_EN_Y,             38 },
    { KEY_EN_U,             44 },
    { KEY_EN_I,             50 },
    { KEY_EN_O,             56 },
    { KEY_EN_P,             62 },
    { KEY_EN_LEFT_BRACKET,  68 },
    { KEY_EN_RIGHT_BRACKET, 74 },
    { KEY_EN_ISO_ENTER,     75 },
    { KEY_EN_DELETE,        86 },
    { KEY_EN_END,           92 },
    { KEY_EN_PAGE_DOWN,     93 },

    /* Row 3: Home Row (13 keys, Enter is shared with Row 2 at idx 75) */
    { KEY_EN_CAPS_LOCK,     3  },
    { KEY_EN_A,             9  },
    { KEY_EN_S,             15 },
    { KEY_EN_D,             21 },
    { KEY_EN_F,             27 },
    { KEY_EN_G,             33 },
    { KEY_EN_H,             39 },
    { KEY_EN_J,             45 },
    { KEY_EN_K,             51 },
    { KEY_EN_L,             57 },
    { KEY_EN_SEMICOLON,     63 },
    { KEY_EN_QUOTE,         69 },
    { KEY_EN_POUND,         80 },

    /* Row 4: Bottom Alpha Row (14 keys) */
    { KEY_EN_LEFT_SHIFT,    5  },
    { KEY_EN_ISO_BACK_SLASH,4  },
    { KEY_EN_Z,             10 },
    { KEY_EN_X,             16 },
    { KEY_EN_C,             22 },
    { KEY_EN_V,             28 },
    { KEY_EN_B,             34 },
    { KEY_EN_N,             40 },
    { KEY_EN_M,             46 },
    { KEY_EN_COMMA,         52 },
    { KEY_EN_PERIOD,        58 },
    { KEY_EN_FORWARD_SLASH, 64 },
    { KEY_EN_RIGHT_SHIFT,   70 },
    { KEY_EN_UP_ARROW,      76 },

    /* Row 5: Bottom Modifier Row (11 keys) */
    { KEY_EN_LEFT_CONTROL,  11 },
    { KEY_EN_LEFT_WINDOWS,  17 },
    { KEY_EN_LEFT_ALT,      23 },
    { KEY_EN_SPACE,         29 },
    { KEY_EN_RIGHT_ALT,     65 },
    { KEY_EN_RIGHT_FUNCTION,71 },
    { KEY_EN_MENU,          77 },
    { KEY_EN_RIGHT_CONTROL, 83 },
    { KEY_EN_LEFT_ARROW,    89 },
    { KEY_EN_DOWN_ARROW,    95 },
    { KEY_EN_RIGHT_ARROW,   82 },
};

static constexpr unsigned int MATRIX_HEIGHT = 6;
static constexpr unsigned int MATRIX_WIDTH  = 17;

static unsigned int alu87a_iso_matrix[6][17] =
{
    {  0, NA,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
    { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 },
    { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49 },
    { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 46, NA, NA, NA },
    { 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, NA, 75, NA, 76, NA },
    { 77, 78, 79, NA, NA, NA, 80, NA, NA, NA, 81, 82, 83, 84, 85, 86, 87 },
};

static mode MakeMode(const char* name, int hid_effect_id, unsigned int flags,
                     unsigned int color_mode, unsigned int colors_min = 0, unsigned int colors_max = 0)
{
    mode m;
    m.name           = name;
    m.value          = hid_effect_id;
    m.flags          = flags;
    m.color_mode     = color_mode;
    m.brightness_min = 0;
    m.brightness_max = 4;
    m.brightness     = 4;
    m.speed_min      = 1;
    m.speed_max      = 5;
    m.speed          = 3;
    m.direction      = MODE_DIRECTION_RIGHT;
    m.colors_min     = colors_min;
    m.colors_max     = colors_max;
    if(colors_max > 0)
    {
        m.colors.resize(colors_max, ToRGBColor(255, 0, 255));
    }
    return m;
}
} // namespace

DarkProjectALU87ADevice::DarkProjectALU87ADevice(hid_device* dev, const std::string& path, OpenRGBPluginAPIInterface* api)
    : dev(dev), api(api), controller(nullptr), active_profile(1), custom_index(0)
{
    RGBController_Setup setup{};
    setup.name        = "Dark Project ALU87A (Midnight ISO)";
    setup.vendor      = "Dark Project / GSKY";
    setup.type        = DEVICE_TYPE_KEYBOARD;
    setup.description = "Dark Project ALU87A TKL ISO Keyboard";
    setup.location    = "HID: " + path;
    setup.serial      = "342D:E40F";
    setup.version     = ReadFirmwareVersion();
    setup.active_mode = 0;
    setup.flags       = CONTROLLER_FLAG_VIRTUAL;

    const unsigned int BASE_SPD_BRI = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;

    setup.modes.push_back(MakeMode("Direct", 14,
                             MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE,
                             MODE_COLORS_PER_LED));
    setup.modes.push_back(MakeMode("Static", 5,
                             MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("Wave", 0,
                             BASE_SPD_BRI | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD |
                             MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_RANDOM, 1, 1));
    setup.modes.push_back(MakeMode("Spiral Wave", 6,
                             BASE_SPD_BRI | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_RANDOM));
    setup.modes.push_back(MakeMode("Spectrum Cycle", 1, BASE_SPD_BRI, MODE_COLORS_NONE));
    setup.modes.push_back(MakeMode("Breathing", 3,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("Random", 2, BASE_SPD_BRI, MODE_COLORS_NONE));
    setup.modes.push_back(MakeMode("Star", 7, BASE_SPD_BRI, MODE_COLORS_NONE));
    setup.modes.push_back(MakeMode("Footprint", 4,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("River", 8,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("Ripples", 9,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("Trigger", 10,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("Color Dischange", 11,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("Sine Wave", 12,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));
    setup.modes.push_back(MakeMode("Rain", 13,
                             BASE_SPD_BRI | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR,
                             MODE_COLORS_MODE_SPECIFIC, 1, 1));

    for(const auto& m : setup.modes)
    {
        mode_hid_ids.push_back(m.value);
    }

    zone kb_zone;
    kb_zone.name       = ZONE_EN_KEYBOARD;
    kb_zone.type       = ZONE_TYPE_MATRIX;
    kb_zone.leds_min   = 88;
    kb_zone.leds_max   = 88;
    kb_zone.leds_count = 88;
    kb_zone.matrix_map = matrix_map_type(MATRIX_HEIGHT, MATRIX_WIDTH, (unsigned int*)&alu87a_iso_matrix);
    setup.zones.push_back(kb_zone);

    for(unsigned int i = 0; i < 88; i++)
    {
        led new_led;
        new_led.name  = alu87a_iso_keys[i].name;
        new_led.value = alu87a_iso_keys[i].hw_idx;
        setup.leds.push_back(new_led);
    }

    ReadInitialState(setup);

    setup.object_ptr            = this;
    setup.DeviceUpdateLEDs      = &DarkProjectALU87ADevice::StaticUpdateLEDs;
    setup.DeviceUpdateZoneLEDs  = &DarkProjectALU87ADevice::StaticUpdateZoneLEDs;
    setup.DeviceUpdateSingleLED = &DarkProjectALU87ADevice::StaticUpdateSingleLED;
    setup.DeviceUpdateMode      = &DarkProjectALU87ADevice::StaticUpdateMode;

    controller = api->CreateVirtualRGBController(&setup);
    if(controller)
    {
        ReadCustomColors();
        api->RegisterVirtualRGBController(controller);
    }
}

DarkProjectALU87ADevice::~DarkProjectALU87ADevice()
{
    if(controller && api)
    {
        api->UnregisterVirtualRGBController(controller);
        api->DeleteVirtualRGBController(controller);
        controller = nullptr;
    }

    std::lock_guard<std::mutex> lock(hid_mutex);
    if(dev)
    {
        hid_close(dev);
        dev = nullptr;
    }
}

void DarkProjectALU87ADevice::StaticUpdateLEDs(void* obj)
{
    if(obj) static_cast<DarkProjectALU87ADevice*>(obj)->UpdateLEDs();
}

void DarkProjectALU87ADevice::StaticUpdateZoneLEDs(void* obj, int /*zone*/)
{
    if(obj) static_cast<DarkProjectALU87ADevice*>(obj)->UpdateLEDs();
}

void DarkProjectALU87ADevice::StaticUpdateSingleLED(void* obj, int /*led*/)
{
    if(obj) static_cast<DarkProjectALU87ADevice*>(obj)->UpdateLEDs();
}

void DarkProjectALU87ADevice::StaticUpdateMode(void* obj)
{
    if(obj) static_cast<DarkProjectALU87ADevice*>(obj)->UpdateMode();
}

bool DarkProjectALU87ADevice::SendFeature(const uint8_t* data, size_t len)
{
    if(!dev) return false;
    uint8_t buf[256];
    std::memset(buf, 0, sizeof(buf));
    std::memcpy(buf, data, std::min(len, sizeof(buf)));
    buf[0] = 0x07;
    return hid_send_feature_report(dev, buf, sizeof(buf)) >= 0;
}

bool DarkProjectALU87ADevice::QueryFeature(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t* out_buf)
{
    if(!dev) return false;
    uint8_t buf[256];
    std::memset(buf, 0, sizeof(buf));
    buf[0] = 0x07;
    buf[1] = cmd;
    buf[2] = arg1;
    buf[3] = arg2;
    buf[4] = arg3;
    if(hid_send_feature_report(dev, buf, sizeof(buf)) < 0)
    {
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    std::memset(out_buf, 0, 256);
    out_buf[0] = 0x07;
    return hid_get_feature_report(dev, out_buf, 256) >= 0;
}

std::string DarkProjectALU87ADevice::ReadFirmwareVersion()
{
    std::lock_guard<std::mutex> lock(hid_mutex);
    uint8_t rbuf[256];
    if(QueryFeature(136 /*0x88*/, 0, 0, 0, rbuf))
    {
        unsigned int fw = ((rbuf[14] & 0x0F) << 8) | rbuf[15];
        char ver[16];
        std::snprintf(ver, sizeof(ver), "v%x", fw);
        return std::string(ver);
    }
    return "Unknown";
}

void DarkProjectALU87ADevice::ReadInitialState(RGBController_Setup& setup)
{
    std::lock_guard<std::mutex> lock(hid_mutex);
    uint8_t rbuf[256];
    if(QueryFeature(129 /*0x81*/, 0, 0, 0, rbuf))
    {
        if(rbuf[2] >= 1 && rbuf[2] <= 3)
        {
            active_profile = rbuf[2];
        }
    }

    if(QueryFeature(130 /*0x82*/, active_profile, 0, 0, rbuf))
    {
        std::memcpy(profile_buf, rbuf, sizeof(profile_buf));
        uint8_t hid_effect = rbuf[9];
        for(unsigned int i = 0; i < setup.modes.size(); i++)
        {
            if(setup.modes[i].value == hid_effect)
            {
                setup.active_mode = (int)i;
                setup.modes[i].brightness = std::min<unsigned int>(4, rbuf[10 + hid_effect]);
                int spd = 6 - (int)rbuf[24 + hid_effect];
                setup.modes[i].speed = (unsigned int)std::clamp(spd, 1, 5);

                if(setup.modes[i].flags & MODE_FLAG_HAS_RANDOM_COLOR)
                {
                    if(rbuf[38 + hid_effect] == 8)
                    {
                        setup.modes[i].color_mode = MODE_COLORS_RANDOM;
                    }
                    else if(setup.modes[i].flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
                    {
                        setup.modes[i].color_mode = MODE_COLORS_MODE_SPECIFIC;
                    }
                }

                if(hid_effect == 6)
                {
                    setup.modes[i].direction = (rbuf[84] == 1) ? MODE_DIRECTION_LEFT : MODE_DIRECTION_RIGHT;
                }
                else if(hid_effect == 0)
                {
                    switch(rbuf[52])
                    {
                        case 0: setup.modes[i].direction = MODE_DIRECTION_RIGHT; break;
                        case 1: setup.modes[i].direction = MODE_DIRECTION_LEFT;  break;
                        case 2: setup.modes[i].direction = MODE_DIRECTION_DOWN;  break;
                        case 3: setup.modes[i].direction = MODE_DIRECTION_UP;    break;
                    }
                }

                if(!setup.modes[i].colors.empty())
                {
                    setup.modes[i].colors[0] = ToRGBColor(rbuf[59], rbuf[67], rbuf[75]);
                }
                break;
            }
        }
    }
}

void DarkProjectALU87ADevice::ReadCustomColors()
{
    if(!controller) return;
    std::lock_guard<std::mutex> lock(hid_mutex);

    uint8_t P[384];
    std::memset(P, 0, sizeof(P));

    uint8_t rbuf[256];
    for(uint8_t part = 0; part < 2; part++)
    {
        if(QueryFeature(138 /*0x8A*/, active_profile, custom_index + 1, part + 1, rbuf))
        {
            std::memcpy(P + (part * 192), rbuf + 8, 192);
        }
    }

    for(unsigned int i = 0; i < 88; i++)
    {
        unsigned int hw = alu87a_iso_keys[i].hw_idx;
        if(hw < 96)
        {
            controller->SetColor(i, ToRGBColor(P[hw], P[96 + hw], P[192 + hw]));
        }
    }
}

void DarkProjectALU87ADevice::UpdateLEDs()
{
    if(!controller) return;
    std::lock_guard<std::mutex> lock(hid_mutex);

    RGBColor* cols = controller->GetColorsPointer();
    if(!cols) return;

    uint8_t P[384];
    std::memset(P, 0, sizeof(P));

    for(unsigned int i = 0; i < 88; i++)
    {
        unsigned int hw = alu87a_iso_keys[i].hw_idx;
        if(hw < 96)
        {
            P[hw]       = RGBGetRValue(cols[i]);
            P[96 + hw]  = RGBGetGValue(cols[i]);
            P[192 + hw] = RGBGetBValue(cols[i]);
        }
    }

    uint8_t buf[256];
    for(uint8_t part = 0; part < 2; part++)
    {
        std::memset(buf, 0, sizeof(buf));
        buf[0] = 0x07;
        buf[1] = 10; // 0x0A: Write Custom Lighting
        buf[2] = active_profile;
        buf[3] = custom_index + 1;
        buf[4] = part + 1;
        std::memcpy(buf + 8, P + (part * 192), 192);
        SendFeature(buf, sizeof(buf));
    }
}

void DarkProjectALU87ADevice::UpdateMode()
{
    if(!controller) return;
    bool is_direct = false;

    {
        std::lock_guard<std::mutex> lock(hid_mutex);
        int m_idx = controller->GetActiveMode();
        if(m_idx < 0 || (size_t)m_idx >= mode_hid_ids.size()) return;

        uint8_t S          = (uint8_t)mode_hid_ids[m_idx];
        unsigned int bri   = controller->GetModeBrightness(m_idx);
        unsigned int spd   = controller->GetModeSpeed(m_idx);
        unsigned int c_mod = controller->GetModeColorMode(m_idx);
        unsigned int dir   = controller->GetModeDirection(m_idx);

        uint8_t buf[256];
        std::memcpy(buf, profile_buf, sizeof(buf));
        buf[0] = 0x07;
        buf[1] = 0x02; // Write Profile
        buf[2] = active_profile;
        buf[9] = S;
        buf[10 + S] = (uint8_t)std::clamp<unsigned int>(bri, 0, 4);
        buf[24 + S] = (uint8_t)std::clamp<int>(6 - (int)spd, 1, 5);
        buf[38 + S] = (c_mod == MODE_COLORS_RANDOM) ? 8 : 0;

        if(S == 6)
        {
            buf[84] = (dir == MODE_DIRECTION_LEFT) ? 1 : 0;
        }
        else if(S == 0)
        {
            uint8_t hw_dir = 0;
            switch(dir)
            {
                case MODE_DIRECTION_RIGHT: hw_dir = 0; break;
                case MODE_DIRECTION_LEFT:  hw_dir = 1; break;
                case MODE_DIRECTION_DOWN:  hw_dir = 2; break;
                case MODE_DIRECTION_UP:    hw_dir = 3; break;
                default:                   hw_dir = 0; break;
            }
            buf[52] = hw_dir;
        }

        buf[53] = (S == 14) ? (custom_index + 1) : 0;
        buf[83] = (S == 3) ? 0 : 8;

        if(c_mod == MODE_COLORS_MODE_SPECIFIC && controller->GetModeColorsCount(m_idx) > 0)
        {
            RGBColor c = controller->GetModeColor(m_idx, 0);
            buf[59] = RGBGetRValue(c);
            buf[67] = RGBGetGValue(c);
            buf[75] = RGBGetBValue(c);
        }

        std::memcpy(profile_buf, buf, sizeof(profile_buf));
        SendFeature(buf, sizeof(buf));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        is_direct = (S == 14);
    }

    if(is_direct)
    {
        UpdateLEDs();
    }
}
