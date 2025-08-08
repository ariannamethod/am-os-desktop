#include "settings_manager.h"

#include <QtCore/QDataStream>

SettingsManager &SettingsManager::instance() {
    static SettingsManager manager;
    return manager;
}

bool SettingsManager::autoUpdate() const {
    QMutexLocker lock(&_mutex);
    return _autoUpdate;
}

void SettingsManager::setAutoUpdate(bool value) {
    QMutexLocker lock(&_mutex);
    _autoUpdate = value;
}

QByteArray SettingsManager::serialize() const {
    QMutexLocker lock(&_mutex);
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream << _autoUpdate;
    return result;
}

void SettingsManager::deserialize(const QByteArray &data) {
    QMutexLocker lock(&_mutex);
    QDataStream stream(data);
    stream >> _autoUpdate;
}

