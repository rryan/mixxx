#ifndef CONTROLLERS_SCRIPT_REQUIRE_H
#define CONTROLLERS_SCRIPT_REQUIRE_H

#include <QtScript>
#include <QString>

class ScriptRequire {
  public:
    static void initializeModule(QScriptContext* context, QScriptEngine* engine,
                                 QScriptValue* module);

    static QScriptValue require(QScriptContext* context, QScriptEngine* engine);

  private:
    static QFile resolveModule(const QString& moduleName,
                               const QStringList& searchPaths);

};


#endif /* CONTROLLERS_SCRIPT_REQUIRE_H */
