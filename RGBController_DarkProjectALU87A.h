#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <hidapi/hidapi.h>
#include "OpenRGBPluginInterface.h"

class DarkProjectALU87ADevice
{
public:
    DarkProjectALU87ADevice(hid_device* dev, const std::string& path, OpenRGBPluginAPIInterface* api);
    ~DarkProjectALU87ADevice();

    RGBControllerInterface* GetController() const { return controller; }

    void UpdateLEDs();
    void UpdateMode();

    static void StaticUpdateLEDs(void* obj);
    static void StaticUpdateZoneLEDs(void* obj, int zone);
    static void StaticUpdateSingleLED(void* obj, int led);
    static void StaticUpdateMode(void* obj);

private:
    hid_device*                dev;
    OpenRGBPluginAPIInterface* api;
    RGBControllerInterface*    controller;
    uint8_t                    active_profile;
    uint8_t                    custom_index;
    uint8_t                    profile_buf[256];
    std::vector<int>           mode_hid_ids;
    std::mutex                 hid_mutex;

    bool        SendFeature(const uint8_t* data, size_t len);
    bool        QueryFeature(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t* out_buf);
    void        ReadInitialState(RGBController_Setup& setup);
    void        ReadCustomColors();
    std::string ReadFirmwareVersion();
};
