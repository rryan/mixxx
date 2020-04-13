#include <QtDebug>

#include "renderthread.h"
#include "util/performancetimer.h"
#include "util/event.h"
#include "util/counter.h"
#include "util/math.h"
#include "util/time.h"

static const QString renderTag("RenderThread render");

RenderThread::RenderThread(QObject* pParent)
        : QThread(pParent),
          m_bDoRendering(true),
          m_syncIntervalTimeMicros(33333),  // 30 FPS
          m_renderMode(ST_TIMER),
          m_droppedFrames(0) {
}

RenderThread::~RenderThread() {
    m_bDoRendering = false;
    m_semaRenderSlot.release(2); // Two slots
    wait();
}

void RenderThread::stop() {
    m_bDoRendering = false;
}


void RenderThread::run() {
    Counter droppedFrames("RenderThread real time error");
    QThread::currentThread()->setObjectName("RenderThread");

    m_timer.start();

    while (m_bDoRendering) {
        if (m_renderMode == ST_FREE) {
            // for benchmark only!

            Event::start(renderTag);
            // Request for the main thread to schedule a render.
            emit(render());
            m_semaRenderSlot.acquire();
            Event::end(renderTag);

            m_timer.restart();
            usleep(1000);
        } else { // if (m_renderMode == ST_TIMER) {
            m_timer.restart();

            Event::start(renderTag);

            // Request for the main thread to schedule a render.
            emit(render());

            // Wait until the main thread processed our rendering request. This
            // means the main thread scheduled a render, not that it has already
            // happened. Ultimately, Qt's compositor is in charge of when our
            // QOpenGLWidgets render and swap.
            m_semaRenderSlot.acquire();
            Event::end(renderTag);

            int elapsed = m_timer.restart().toIntegerMicros();
            int sleepTimeMicros = m_syncIntervalTimeMicros - elapsed;
            if (sleepTimeMicros > 100) {
                usleep(sleepTimeMicros);
            } else if (sleepTimeMicros < 0) {
                m_droppedFrames++;
            }
        }
    }
}

int RenderThread::elapsed() {
    return static_cast<int>(m_timer.elapsed().toIntegerMicros());
}

void RenderThread::setSyncIntervalTimeMicros(int syncTime) {
    m_syncIntervalTimeMicros = syncTime;
}

void RenderThread::setRenderType(int type) {
    if (type >= (int)RenderThread::ST_COUNT) {
        type = RenderThread::ST_TIMER;
    }
    m_renderMode = (enum RenderMode)type;
    m_droppedFrames = 0;
}

int RenderThread::droppedFrames() {
    return m_droppedFrames;
}

void RenderThread::renderSlotFinished() {
    m_semaRenderSlot.release();
}
