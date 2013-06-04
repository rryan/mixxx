#ifndef SYNCWORKER_H
#define SYNCWORKER_H

#include <QObject>

#include "engine/engineworker.h"

class EngineWorkerScheduler;
class CallbackControlManager;

class SyncWorker : public EngineWorker {
    Q_OBJECT
  public:
    explicit SyncWorker(EngineWorkerScheduler* pScheduler,
                        CallbackControlManager* callbackControlManager);
    virtual ~SyncWorker();

    void run();
    void schedule();
  private:
    CallbackControlManager* m_pCallbackControlManager;
};

#endif /* SYNCWORKER_H */
