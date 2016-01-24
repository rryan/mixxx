#ifndef SKINLOADER_H
#define SKINLOADER_H

#include <QWidget>
#include <QList>
#include <QDir>

#include "preferences/usersettings.h"

class KeyboardEventFilter;
class PlayerManager;
class ControllerManager;
class Library;
class VinylControlManager;
class EffectsManager;
class RecordingManager;
class LaunchImage;

class SkinLoader {
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();


    QWidget* loadConfiguredSkin(QWidget* pParent,
                                KeyboardEventFilter* pKeyboard,
                                std::shared_ptr<PlayerManager> pPlayerManager,
                                std::shared_ptr<ControllerManager> pControllerManager,
                                std::shared_ptr<Library> pLibrary,
                                std::shared_ptr<VinylControlManager> pVCMan,
                                std::shared_ptr<EffectsManager> pEffectsManager,
                                std::shared_ptr<RecordingManager> pRecordingManager);

    LaunchImage* loadLaunchImage(QWidget* pParent);

    QString getSkinPath(const QString& skinName) const;
    QPixmap getSkinPreview(const QString& skinName) const;
    QString getConfiguredSkinPath() const;
    QString getDefaultSkinName() const;
    QList<QDir> getSkinSearchPaths() const;

  private:
    QString pickResizableSkin(QString oldSkin) const;

    UserSettingsPointer m_pConfig;
};


#endif /* SKINLOADER_H */
