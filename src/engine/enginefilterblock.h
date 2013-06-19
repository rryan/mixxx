/***************************************************************************
                          enginefilterblock.h  -  description
                             -------------------
    begin                : Thu Apr 4 2002
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

#ifndef ENGINEFILTERBLOCK_H
#define ENGINEFILTERBLOCK_H

#include "engine/engineobject.h"

class CallbackControl;
class EngineState;

#define SIZE_NOISE_BUF 40
//#define NOISE_FACTOR 116.415321827e-12 // 1/4 bit of noise (99db SNR)
#define NOISE_FACTOR 0.25              // this is necessary to prevent denormals
// from consuming too much CPU resources
// and is well below being audible.
/**
  * Parallel processing of LP, BP and HP filters, and final mixing
  *
  *@author Tue and Ken Haste Andersen
  */

class EngineFilterBlock : public EngineObject {
  public:
    EngineFilterBlock(const char *group, EngineState* pEngineState);
    virtual ~EngineFilterBlock();

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut,
                 const int iBufferSize);

  private:

    void setFilters(bool forceSetting = false);

    CSAMPLE *m_pTemp1, *m_pTemp2, *m_pTemp3;
    EngineObject *low, *band, *high;
    CallbackControl* filterpotLow;
    CallbackControl* filterpotMid;
    CallbackControl* filterpotHigh;
    CallbackControl* filterKillLow;
    CallbackControl* filterKillMid;
    CallbackControl* filterKillHigh;

    CallbackControl* m_loEqFreq;
    CallbackControl* m_hiEqFreq;
    CallbackControl* m_lofiEq;

    int ilowFreq, ihighFreq;
    bool blofi;
};

#endif
