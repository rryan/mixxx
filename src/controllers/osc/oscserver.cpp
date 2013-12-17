#include <QtDebug>
#include <QRegExp>

#include "controlobjectthread.h"
#include "controllers/osc/oscserver.h"

void error(int num, const char *msg, const char *path) {
    qDebug() << "liblo server error" << num << "in path" << path << ":" << msg;
}

int control_set_handler(const char *path, const char *types, lo_arg** argv,
                        int argc, void *data, void *user_data) {
    QRegExp typePattern("^(sd)+$");

    QString typesStr(types);
    if (!typePattern.exactMatch(typesStr)) {
        qDebug() << "/control/set got unexpected message type" << types;
        return 1;
    }

    for (int i = 0; i + 1 < typesStr.length(); i += 2) {
        //qDebug() << path << types << argc << &argv[i]->s << argv[i+1]->d;
        QString keyStr(&argv[i]->s);
        ConfigKey key = ConfigKey::parseCommaSeparated(keyStr);
        ControlObjectThread cot(key);
        cot.set(argv[i+1]->d);
    }

    return 0;
}

int load_track_handler(const char* path, const char* types, lo_arg** argv,
                       int argc, void *data, void* user_data) {
    OscServer* pServer = static_cast<OscServer*>(user_data);
    QString location(&argv[0]->s);
    QString group(&argv[1]->s);
    pServer->doLoadLocationToPlayer(location, group);
    return 0;
};

int float_handler(const char* path, const char* types, lo_arg** argv,
                  int argc, void *data, void* user_data) {
    OscServer* pServer = static_cast<OscServer*>(user_data);
    qDebug() << "float_handler" << path << types << argc << argv[0]->f;
    QString pathStr(path);
    QRegExp adjCratePattern("^/autodj/play/([^/]+)$");
    if (argv[0]->f > 0 && adjCratePattern.exactMatch(pathStr)) {
        QString crate = adjCratePattern.cap(1);
        pServer->doPlayAutoDJCrate(crate);
        return 0;
    }
    return 1;
}

int double_handler(const char *path, const char *types, lo_arg** argv,
                   int argc, void *data, void *user_data) {
    qDebug() << "double_handler" << path << types << argc << argv[0]->d;
    QString pathStr(path);
    QRegExp controlPattern("^/control/([^/]+)/([^/]+)$");
    if (controlPattern.exactMatch(pathStr)) {
        QString group = controlPattern.cap(1);
        QString item = controlPattern.cap(2);

        ControlObjectThread cot(group, item);
        cot.set(argv[0]->d);
        return 0;
    }

    return 1;
}

int unknown_handler(const char *path, const char *types, lo_arg** argv,
                   int argc, void *data, void *user_data) {
    qDebug() << path << types << argc;
    return 1;
}

OscServer::OscServer(QObject* pParent, int port)
        : QObject(pParent) {
    QString portStr = QString::number(port);

    //m_server = lo_server_thread_new_from_url("osc.udp://:2448", error);
    m_server = lo_server_thread_new("2448", error);

    // add method that will match the path /foo/bar, with two numbers, coerced
    // to float and int
    lo_server_thread_add_method(m_server, "/control/set", NULL, control_set_handler, this);
    lo_server_thread_add_method(m_server, "/player/load", "ss", load_track_handler, this);
    lo_server_thread_add_method(m_server, NULL, "d", double_handler, this);
    lo_server_thread_add_method(m_server, NULL, "f", float_handler, this);
    lo_server_thread_add_method(m_server, NULL, NULL, unknown_handler, this);

    lo_server_thread_start(m_server);
}

OscServer::~OscServer() {
    lo_server_thread_free(m_server);
}
