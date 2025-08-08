/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>

namespace base {

class Integration {
public:
  static Integration &Instance() {
    static Integration instance;
    return instance;
  }

  QString executableName() const { return {}; }
  QString executableDir() const { return {}; }
};

} // namespace base
