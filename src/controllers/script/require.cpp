#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtDebug>

#include "controllers/script/require.h"

const char* kModuleCacheName = "$MODULE_CACHE";
const char* kRequireName = "require";

// static
void ScriptRequire::initializeModule(QScriptContext* context, QScriptEngine* engine,
                                     QScriptValue* module) {
    module->setProperty(kModuleCacheName, engine->newObject());
    module->setProperty(kRequireName, engine->newFunction(require, 1));
}

QFile ScriptRequire::resolveModule(const QString& moduleIdentifier,
                                   const QStringList& searchPaths) {
    return QFile(moduleIdentifier);
}

// static
QScriptValue ScriptRequire::require(QScriptContext* context,
                                    QScriptEngine* engine) {
    if (context->argumentCount() != 1) {
        qDebug() << "ControllerEngine::require() requires exactly one argument.";
        context->throwError(QScriptContext::TypeError, "require() requires exactly one string argument");
        return QScriptValue();
    }

    const QString moduleIdentifier = context->argument(0).toString();
    qDebug() << "ControllerEngine::require()" << moduleIdentifier;

    // TODO(rryan): search paths.
    QStringList searchPaths;
    QFile file = resolveModule(moduleIdentifier, searchPaths);
    QFileInfo fileInfo(file);

    QSCriptValue moduleCache = engine->globalObject()->property(kModuleCacheName);
    QString cacheKey = fileInfo.canonicalFilePath();
    QScriptValue module = moduleCache.property(cacheKey);

    if (module.isValid()) {


    } else {
        module = engine->newObject();


    }
    moduleCache.property(moduleIdentifier);



    if (!file.open(QIODevice::ReadOnly)) {
        return context->throwValue(QString("Failed to open %1").arg(moduleIdentifier));
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString script = in.readAll();
    file.close();

    qDebug() << "Read" << moduleIdentifier << ":" << script;

    QScriptContext* newContext = engine->pushContext();

    // Insert an 'exports' object that the module can annotate with methods to
    // export.
    newContext->activationObject().setProperty("exports", engine->newObject());

    // module.setProperty("code", exports);
    // module.setProperty("moduleIdentifier", moduleIdentifier);
    // module.setProperty("timestamp", engine->newDate(fileInfo.lastModified()));
    // run the script
    engine->evaluate(script, moduleIdentifier);

    // The module may have overwritten our exports object with their own, so
    // fetch the latest value of 'exports' to get the module exports we will
    // return to the caller.
    QScriptValue exports = newContext->activationObject().property("exports");

    // TODO(rryan): cache exports
    //module.setProperty("code", );

    engine->popContext();
    if (engine->hasUncaughtException()) {
        return engine->uncaughtException();
    }
    qDebug() << "loaded" << moduleIdentifier;

    return exports;
}
