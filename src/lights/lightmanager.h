
#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include <QObject>

class LightManager : public QObject {
    Q_OBJECT
  public:
    LightManager(QObject* pParent=NULL);
    virtual ~LightManager();

    virtual void sync() = 0;
};


#endif /* LIGHTMANAGER_H */
