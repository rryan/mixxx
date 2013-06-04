/***************************************************************************
                          enginepregain.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEPREGAIN_H
#define ENGINEPREGAIN_H

#include "engine/engineobject.h"
#include "util/performancetimer.h"

class CallbackControl;
class EngineState;

class EnginePregain : public EngineObject {
  public:
    EnginePregain(const char *group, EngineState* pEngineState);
    virtual ~EnginePregain();

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut,
                 const int iBufferSize);

  private:
    CallbackControl* potmeterPregain;
    CallbackControl* m_pTotalGain;
    CallbackControl* m_pControlReplayGain;
    CallbackControl* m_pPassthroughEnabled;
    CallbackControl* m_pReplayGainBoost;
    CallbackControl* m_pEnableReplayGain;
    bool m_bSmoothFade;
    PerformanceTimer m_timer;
};

#endif
