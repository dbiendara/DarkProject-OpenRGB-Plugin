#pragma once

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <vector>
#include "OpenRGBPluginInterface.h"

class DarkProjectALU87ADevice;

class DarkProjectPlugin : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID FILE "DarkProjectPlugin.json")
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    DarkProjectPlugin();
    ~DarkProjectPlugin() override;

    OpenRGBPluginInfo GetPluginInfo() override;
    unsigned int      GetPluginAPIVersion() override;

    void              Load(OpenRGBPluginAPIInterface* plugin_api_ptr) override;
    QWidget*          GetWidget() override;
    QMenu*            GetTrayMenu() override;
    void              Unload() override;
    void              OnProfileAboutToLoad() override {}
    void              OnProfileLoad(nlohmann::json /*profile_data*/) override {}
    nlohmann::json    OnProfileSave() override { return nlohmann::json(); }
    unsigned char*    OnSDKCommand(unsigned int /*pkt_id*/, unsigned char* /*pkt_data*/, unsigned int* /*pkt_size*/) override { return nullptr; }

    void              ProfileManagerUpdated(unsigned int /*update_reason*/) override {}
    void              ResourceManagerUpdated(unsigned int /*update_reason*/) override {}
    void              SettingsManagerUpdated(unsigned int /*update_reason*/) override {}

private:
    OpenRGBPluginAPIInterface*              plugin_api = nullptr;
    std::vector<DarkProjectALU87ADevice*>   devices;
    QLabel*                                 status_label = nullptr;

    void DetectDevices();
    void ClearDevices();
};
