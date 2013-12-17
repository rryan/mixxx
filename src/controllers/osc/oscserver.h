#ifndef OSCSERVER_H
#define OSCSERVER_H

#include <QObject>

#include <lo/lo.h>

class OscServer : public QObject {
    Q_OBJECT
  public:
    OscServer(QObject* pParent, int port);
    virtual ~OscServer();

    void doLoadLocationToPlayer(const QString& location, const QString& group) {
        emit(loadLocationToPlayer(location, group));
    }

    void doPlayAutoDJCrate(const QString& crate) {
        emit(playAutoDJCrate(crate));
    }

  signals:
    void loadLocationToPlayer(QString location, QString group);
    void playAutoDJCrate(QString crate);

  private:
    lo_server_thread m_server;
};

#endif /* OSCSERVER_H */
