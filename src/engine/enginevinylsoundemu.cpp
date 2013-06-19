/***************************************************************************
                          enginevinylsoundemu.cpp  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include <QtGlobal>

#include "engine/enginevinylsoundemu.h"

#include "configobject.h"
#include "controlobject.h"
#include "engine/enginestate.h"
#include "sampleutil.h"

/** This class emulates the response of a vinyl record's audio to changes
 *   in speed. In practice, it quiets the audio during very slow playback.
 *   Dithering also helps mask the aliasing due to interpolation that occurs at
 *   these slow speeds.
 */

EngineVinylSoundEmu::EngineVinylSoundEmu(const char* group,
                                         EngineState* pEngineState) {
    CallbackControlManager* pCallbackControlManager =
            pEngineState->getControlManager();
    m_pRateEngine = pCallbackControlManager->getControl(
        ConfigKey(group, "rateEngine"));
    m_fSpeed = m_fOldSpeed = 0.0f;
    m_fGainFactor = 1.0f;
    m_iNoisePos = 0;

    for (int i = 0; i < NOISE_BUFFER_SIZE; ++i) {
        m_fNoise[i] = static_cast<float>(rand() % RAND_MAX) / RAND_MAX - 0.5;
    }
}

EngineVinylSoundEmu::~EngineVinylSoundEmu() {
}

void EngineVinylSoundEmu::process(const CSAMPLE * pIn, const CSAMPLE * pOut,
                                  const int iBufferSize) {
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    m_fSpeed = static_cast<float>(m_pRateEngine->get());
    float rateFrac = 2 * (m_fSpeed - m_fOldSpeed) / static_cast<float>(iBufferSize);
    float curRate = m_fOldSpeed;

    // Scale volume if playback speed is below 7%.
    const float thresholdSpeed = 0.070f;
    // Dither if playback speed is below 85%.
    const float ditherSpeed = 0.85f;

    // iterate over old rate to new rate to prevent audible pops
    for (int i = 0; i < iBufferSize; i += 2) {
        float absCurRate = fabs(curRate);
        float dither = 0;
        if (absCurRate < ditherSpeed) {
            dither = m_fNoise[m_iNoisePos];
            m_iNoisePos = (m_iNoisePos + 1) % NOISE_BUFFER_SIZE;
        }

        if (absCurRate < thresholdSpeed) {
            float gainfrac = absCurRate / thresholdSpeed;
            pOutput[i] = gainfrac * static_cast<float>(pIn[i]) + dither;
            pOutput[i+1] = gainfrac * static_cast<float>(pIn[i+1]) + dither;
        } else {
           pOutput[i] = pIn[i] + dither;
           pOutput[i+1] = pIn[i+1] + dither;
        }

        curRate += rateFrac;
    }
    m_fOldSpeed = m_fSpeed;
}

