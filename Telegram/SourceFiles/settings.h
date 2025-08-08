/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/integration.h"
#include "ui/style/style_core.h"

class DocumentData;
class UserData;

using RecentStickerPackOld = QList<QPair<DocumentData *, int16>>;
using RecentStickerPreload = QVector<QPair<uint64, ushort>>;
using RecentStickerPack = QVector<QPair<DocumentData *, ushort>>;
using RecentHashtagPack = QList<QPair<QString, ushort>>;
using RecentInlineBots = QVector<UserData *>;

enum LaunchMode {
  LaunchModeNormal = 0,
  LaunchModeAutoStart,
  LaunchModeFixPrevious,
  LaunchModeCleanup,
};

class SettingsManager {
public:
  static SettingsManager &instance();

  Qt::LayoutDirection gLangDir;
  bool gInstallBetaVersion;
  uint64 gAlphaVersion;
  uint64 gRealAlphaVersion;
  QByteArray gAlphaPrivateKey;

  bool gManyInstance;
  QString gKeyFile;
  QString gWorkingDir;

  QStringList gSendPaths;
  QString gStartUrl;

  QString gDialogLastPath;
  QString gDialogHelperPath;

  bool gStartMinimized;
  bool gStartInTray;
  bool gAutoStart;
  bool gSendToMenu;
  LaunchMode gLaunchMode;
  bool gSeenTrayTooltip;
  bool gRestartingUpdate;
  bool gRestarting;
  bool gRestartingToSettings;
  bool gWriteProtected;
  bool gQuit;
  int32 gLastUpdateCheck;
  bool gNoStartUpdate;
  bool gStartToSettings;
  bool gDebugMode;

  uint32 gConnectionsInSession;

  QByteArray gLocalSalt;
  int gScreenScale;
  int gConfigScale;

  RecentStickerPreload gRecentStickersPreload;
  RecentStickerPack gRecentStickers;

  RecentHashtagPack gRecentWriteHashtags;
  RecentHashtagPack gRecentSearchHashtags;

  RecentInlineBots gRecentInlineBots;

  bool gPasswordRecovered;
  int32 gPasscodeBadTries;
  crl::time gPasscodeLastTry;

  int gOtherOnline;

  int32 gAutoDownloadPhoto;
  int32 gAutoDownloadAudio;
  int32 gAutoDownloadGif;

private:
  SettingsManager();
};

#define DeclareReadSetting(Type, Name)                                         \
  inline const Type &c##Name() { return SettingsManager::instance().g##Name; }

#define DeclareSetting(Type, Name)                                             \
  DeclareReadSetting(Type, Name) inline void cSet##Name(const Type &Name) {    \
    SettingsManager::instance().g##Name = Name;                                \
  }

#define DeclareRefSetting(Type, Name)                                          \
  DeclareSetting(Type, Name) inline Type &cRef##Name() {                       \
    return SettingsManager::instance().g##Name;                                \
  }

DeclareSetting(Qt::LayoutDirection, LangDir);
inline bool rtl() { return style::RightToLeft(); }

DeclareSetting(bool, InstallBetaVersion);
DeclareSetting(uint64, AlphaVersion);
DeclareSetting(uint64, RealAlphaVersion);
DeclareSetting(QByteArray, AlphaPrivateKey);

DeclareSetting(bool, AutoStart);
DeclareSetting(bool, StartMinimized);
DeclareSetting(bool, StartInTray);
DeclareSetting(bool, SendToMenu);
DeclareSetting(LaunchMode, LaunchMode);
DeclareSetting(QString, KeyFile);
DeclareSetting(QString, WorkingDir);
inline void cForceWorkingDir(const QString &newDir) {
  cSetWorkingDir(newDir);
  if (!cWorkingDir().isEmpty()) {
    cSetWorkingDir(QDir(cWorkingDir()).absolutePath() + '/');
    QDir().mkpath(cWorkingDir());
    QFile::setPermissions(cWorkingDir(), QFileDevice::ReadUser |
                                             QFileDevice::WriteUser |
                                             QFileDevice::ExeUser);
  }
}
inline QString cExeName() {
  return base::Integration::Instance().executableName();
}
inline QString cExeDir() {
  return base::Integration::Instance().executableDir();
}
DeclareSetting(QString, DialogLastPath);
DeclareSetting(QString, DialogHelperPath);
inline QString cDialogHelperPathFinal() {
  return cDialogHelperPath().isEmpty() ? cExeDir() : cDialogHelperPath();
}

DeclareSetting(bool, SeenTrayTooltip);
DeclareSetting(bool, RestartingUpdate);
DeclareSetting(bool, Restarting);
DeclareSetting(bool, RestartingToSettings);
DeclareSetting(bool, WriteProtected);
DeclareSetting(int32, LastUpdateCheck);
DeclareSetting(bool, NoStartUpdate);
DeclareSetting(bool, StartToSettings);
DeclareSetting(bool, DebugMode);
DeclareSetting(bool, ManyInstance);
DeclareSetting(bool, Quit);

DeclareSetting(QByteArray, LocalSalt);
DeclareSetting(int, ScreenScale);
DeclareSetting(int, ConfigScale);
DeclareSetting(uint32, ConnectionsInSession);

DeclareSetting(RecentStickerPreload, RecentStickersPreload);
DeclareRefSetting(RecentStickerPack, RecentStickers);

DeclareRefSetting(RecentHashtagPack, RecentWriteHashtags);
DeclareSetting(RecentHashtagPack, RecentSearchHashtags);

DeclareRefSetting(RecentInlineBots, RecentInlineBots);

DeclareSetting(bool, PasswordRecovered);

DeclareSetting(int32, PasscodeBadTries);
DeclareSetting(crl::time, PasscodeLastTry);

DeclareSetting(QStringList, SendPaths);
DeclareSetting(QString, StartUrl);

DeclareSetting(int, OtherOnline);
DeclareSetting(int32, AutoDownloadPhoto);
DeclareSetting(int32, AutoDownloadAudio);
DeclareSetting(int32, AutoDownloadGif);

inline bool passcodeCanTry() {
  if (cPasscodeBadTries() < 3)
    return true;
  auto dt = crl::now() - cPasscodeLastTry();
  switch (cPasscodeBadTries()) {
  case 3:
    return dt >= 5000;
  case 4:
    return dt >= 10000;
  case 5:
    return dt >= 15000;
  case 6:
    return dt >= 20000;
  case 7:
    return dt >= 25000;
  }
  return dt >= 30000;
}

inline int cEvalScale(int scale) {
  return (scale == style::kScaleAuto) ? cScreenScale() : scale;
}

inline int cScale() { return style::Scale(); }

inline void SetScaleChecked(int scale) {
  cSetConfigScale(style::CheckScale(scale));
}

inline void ValidateScale() {
  SetScaleChecked(cConfigScale());
  style::SetScale(cEvalScale(cConfigScale()));
}
