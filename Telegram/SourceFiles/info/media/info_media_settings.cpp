#include "info/media/info_media_settings.h"

#include <QtCore/QCoreApplication>

namespace Info::Media {
namespace {

InfoMediaSettings LoadSettings() {
  InfoMediaSettings settings;
  const auto args = QCoreApplication::arguments();
  const auto prefix = QStringLiteral("--media-search-batch-size=");
  for (const auto &arg : args) {
    if (arg.startsWith(prefix)) {
      bool ok = false;
      const auto value = arg.mid(prefix.size()).toInt(&ok);
      if (ok && value > 0) {
        settings.mediaCountForSearch = value;
      }
      break;
    }
  }
  return settings;
}

} // namespace

const InfoMediaSettings &GetSettings() {
  static const auto settings = LoadSettings();
  return settings;
}

} // namespace Info::Media
