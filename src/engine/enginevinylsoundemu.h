/***************************************************************************
                          enginevinylsoundemu.h  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
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

#ifndef ENGINEVINYLSOUNDEMU_H
#define ENGINEVINYLSOUNDEMU_H

#include "engineobject.h"

#define NOISE_BUFFER_SIZE 1000

class CallbackControl;
class EngineState;

class EngineVinylSoundEmu : public EngineObject {
  public:
    EngineVinylSoundEmu(const char *group, EngineState* pEngineState);
    virtual ~EngineVinylSoundEmu();

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut,
                 const int iBufferSize);

  private:
    CallbackControl* m_pRateEngine;
    float m_fSpeed, m_fOldSpeed;
    float m_fGainFactor;

    float m_fNoise[NOISE_BUFFER_SIZE];
    int m_iNoisePos;
};

#endif
