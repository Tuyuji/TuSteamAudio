/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#if !defined(Q_MOC_RUN)
#include <QWidget>

#include "TuSteamAudio/Types.h"
#endif

namespace TuSteamAudio
{
   class AttenuationGraphWidget : public QWidget
   {
       Q_OBJECT
   public:
       explicit AttenuationGraphWidget(QWidget* parent = nullptr);

       void UpdateAttenuation(const Attenuation::TuAttenuation& settings);
       void SetListenerDistance(float distance);

   protected:
       void paintEvent(QPaintEvent* event) override;
   private:
       Attenuation::TuAttenuation m_attenuation = {};
       float m_listenerDistance = 0.0f;
   };
}
