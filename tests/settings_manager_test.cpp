#include "../Telegram/SourceFiles/settings/settings_manager.h"
#include <cassert>

int main() {
    auto &manager = SettingsManager::instance();
    manager.setAutoUpdate(true);
    auto data = manager.serialize();
    manager.setAutoUpdate(false);
    manager.deserialize(data);
    assert(manager.autoUpdate() == true);
    return 0;
}
