/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SAPlayerComponentConfig.h"
#include <AzCore/Serialization/SerializeContext.h>

using namespace TuSteamAudio;

void SAPlayerComponentConfig::Reflect(AZ::ReflectContext* context)
{
    auto sc = azrtti_cast<AZ::SerializeContext*>(context);
    if (!sc)
        return;

    sc->Class<SAPlayerComponentConfig>()
        ->Version(0)
        ->Field("distanceModel", &SAPlayerComponentConfig::m_distanceModel)
        ->Field("attenuation", &SAPlayerComponentConfig::m_attenuation);
}
