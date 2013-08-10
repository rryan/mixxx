#include <QtDebug>
#include <QRegExp>

#include "controlobjectthread.h"
#include "controllers/osc/oscserver.h"

void error(int num, const char *msg, const char *path) {
    qDebug() << "liblo server error" << num << "in path" << path << ":" << msg;
}

int control_set_handler(const char *path, const char *types, lo_arg** argv,
                        int argc, void *data, void *user_data) {
    qDebug() << path << types << argc << &argv[0]->s << argv[1]->d;
    QString keyStr(&argv[0]->s);
    ConfigKey key = ConfigKey::parseCommaSeparated(keyStr);
    ControlObjectThread cot(key);
    cot.set(argv[1]->d);
    return 0;
}


int double_handler(const char *path, const char *types, lo_arg** argv,
                   int argc, void *data, void *user_data) {
    qDebug() << path << types << argc << argv[0]->d;
    QString pathStr(path);
    QRegExp controlPattern("^/control/([^/]+)/([^/]+)$");
    if (controlPattern.exactMatch(pathStr)) {
        QString group = controlPattern.cap(1);
        QString item = controlPattern.cap(2);

        ControlObjectThread cot(group, item);
        cot.set(argv[0]->d);
    }


    return 0;
}

int unknown_handler(const char *path, const char *types, lo_arg** argv,
                   int argc, void *data, void *user_data) {
    qDebug() << path << types << argc;
    return 1;
}

OscServer::OscServer(QObject* pParent, int port)
        : QObject(pParent) {
    QString portStr = QString::number(port);

    m_server = lo_server_thread_new_from_url("osc.udp://:2448", error);

    // add method that will match the path /foo/bar, with two numbers, coerced
    // to float and int
    lo_server_thread_add_method(m_server, "/control/set", "sd", control_set_handler, this);
    lo_server_thread_add_method(m_server, NULL, "d", double_handler, this);
    lo_server_thread_add_method(m_server, NULL, NULL, unknown_handler, this);

    lo_server_thread_start(m_server);
}

OscServer::~OscServer() {
    lo_server_thread_free(m_server);
}
