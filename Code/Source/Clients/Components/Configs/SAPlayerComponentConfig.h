/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "AzCore/Component/ComponentBus.h"
#include "TuSteamAudio/Types.h"

namespace TuSteamAudio
{
    class SAPlayerComponentConfig : public AZ::ComponentConfig
    {
        AZ_RTTI(SAPlayerComponentConfig, "{224102F4-404E-493C-BF3E-39D88C16E1EC}");
    public:
        static void Reflect(AZ::ReflectContext* context);

        DistanceModel m_distanceModel = DistanceModel::TuAttenuation;
        Attenuation::TuAttenuation m_attenuation = {};
    };
} // TuSteamAudio