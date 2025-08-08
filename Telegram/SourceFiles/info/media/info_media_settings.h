#ifndef TELEGRAM_INFO_MEDIA_INFO_MEDIA_SETTINGS_H
#define TELEGRAM_INFO_MEDIA_INFO_MEDIA_SETTINGS_H

#include <QtCore/QString>

namespace Info::Media {

struct InfoMediaSettings {
  int mediaCountForSearch = 10;
};

const InfoMediaSettings &GetSettings();

} // namespace Info::Media

#endif // TELEGRAM_INFO_MEDIA_INFO_MEDIA_SETTINGS_H
