#ifndef OSCSERVER_H
#define OSCSERVER_H

#include <QObject>

#include <lo/lo.h>

class OscServer : public QObject {
    Q_OBJECT
  public:
    OscServer(QObject* pParent, int port);
    virtual ~OscServer();

  private:
    lo_server_thread m_server;
};

#endif /* OSCSERVER_H */
