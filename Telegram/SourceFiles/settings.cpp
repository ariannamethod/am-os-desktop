/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings.h"

#include "ui/emoji_config.h"

SettingsManager &SettingsManager::instance() {
  static SettingsManager instance;
  return instance;
}

SettingsManager::SettingsManager()
    : gLangDir(Qt::LeftToRight), gInstallBetaVersion(AppBetaVersion),
      gAlphaVersion(AppAlphaVersion), gRealAlphaVersion(AppAlphaVersion),
      gManyInstance(false), gStartMinimized(false), gStartInTray(false),
      gAutoStart(false), gSendToMenu(false), gLaunchMode(LaunchModeNormal),
      gSeenTrayTooltip(false), gRestartingUpdate(false), gRestarting(false),
      gRestartingToSettings(false), gWriteProtected(false), gQuit(false),
      gLastUpdateCheck(0), gNoStartUpdate(false), gStartToSettings(false),
      gDebugMode(false), gConnectionsInSession(1),
      gScreenScale(style::kScaleAuto), gConfigScale(style::kScaleAuto),
      gPasswordRecovered(false), gPasscodeBadTries(0), gPasscodeLastTry(0),
      gOtherOnline(0), gAutoDownloadPhoto(0), gAutoDownloadAudio(0),
      gAutoDownloadGif(0) {}
