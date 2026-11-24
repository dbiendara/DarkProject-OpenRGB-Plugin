#include "DarkProjectPlugin.h"
#include "RGBController_DarkProjectALU87A.h"
#include <QVBoxLayout>
#include <hidapi/hidapi.h>

DarkProjectPlugin::DarkProjectPlugin() = default;

DarkProjectPlugin::~DarkProjectPlugin()
{
    ClearDevices();
}

OpenRGBPluginInfo DarkProjectPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;
    info.Name        = "Dark Project ALU87A";
    info.Description = "WebHID-reverse-engineered driver for Dark Project ALU87A (GSKY 342D:E40F) keyboards.";
    info.Version     = "1.0.0";
    info.Commit      = "";
    info.URL         = "https://software.darkproject.eu/";
    info.Location    = OPENRGB_PLUGIN_LOCATION_SETTINGS;
    info.Label       = "Dark Project ALU87A";
    return info;
}

unsigned int DarkProjectPlugin::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

void DarkProjectPlugin::Load(OpenRGBPluginAPIInterface* plugin_api_ptr)
{
    plugin_api = plugin_api_ptr;
    hid_init();
    DetectDevices();
}

void DarkProjectPlugin::Unload()
{
    ClearDevices();
    hid_exit();
}

QWidget* DarkProjectPlugin::GetWidget()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);

    status_label = new QLabel(
        QString("Dark Project ALU87A Plugin loaded. Active devices: %1").arg(devices.size()),
        widget);
    QPushButton* rescan_btn = new QPushButton("Rescan Dark Project Keyboards", widget);
    QObject::connect(rescan_btn, &QPushButton::clicked, [this]() {
        ClearDevices();
        DetectDevices();
        if(status_label)
        {
            status_label->setText(
                QString("Dark Project ALU87A Plugin loaded. Active devices: %1").arg(devices.size()));
        }
    });

    layout->addWidget(status_label);
    layout->addWidget(rescan_btn);
    layout->addStretch();
    return widget;
}

QMenu* DarkProjectPlugin::GetTrayMenu()
{
    return nullptr;
}

void DarkProjectPlugin::DetectDevices()
{
    if(!plugin_api) return;

    struct hid_device_info* devs = hid_enumerate(0x342D, 0xE40F);
    struct hid_device_info* cur  = devs;

    while(cur)
    {
        bool match = (cur->usage_page == 0xFF01 && cur->usage == 0x01) ||
                     (cur->usage_page == 0 && cur->interface_number == 2);
        if(match && cur->path)
        {
            hid_device* handle = hid_open_path(cur->path);
            if(handle)
            {
                auto* dev = new DarkProjectALU87ADevice(handle, cur->path, plugin_api);
                devices.push_back(dev);
                break;
            }
        }
        cur = cur->next;
    }

    hid_free_enumeration(devs);
}

void DarkProjectPlugin::ClearDevices()
{
    for(auto* dev : devices)
    {
        delete dev;
    }
    devices.clear();
}
