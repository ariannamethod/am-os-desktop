#pragma once

#include <QtCore/QMutex>
#include <QtCore/QByteArray>

class SettingsManager {
public:
    static SettingsManager &instance();

    bool autoUpdate() const;
    void setAutoUpdate(bool value);

    QByteArray serialize() const;
    void deserialize(const QByteArray &data);

private:
    SettingsManager() = default;

    mutable QMutex _mutex;
    bool _autoUpdate = true;
};

