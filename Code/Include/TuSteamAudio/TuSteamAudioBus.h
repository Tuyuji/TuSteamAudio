/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <TuSteamAudio/TuSteamAudioTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <Sune/PlayerAudioEffect.h>

#include "phonon.h"
#include "Types.h"

namespace TuSteamAudio
{
    class TuSteamAudioRequests
    {
    public:
        AZ_RTTI(TuSteamAudioRequests, TuSteamAudioRequestsTypeId);
        virtual ~TuSteamAudioRequests() = default;

        virtual IPLHRTF GetHrtf() = 0;
        virtual IPLContext GetContext() = 0;
        virtual IPLAudioSettings GetAudioSettings() = 0;
        virtual IPLScene GetRootScene() = 0;
        virtual IPLSimulator GetSimulator() = 0;
    };

    class TuSteamAudioBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using TuSteamAudioRequestBus = AZ::EBus<TuSteamAudioRequests, TuSteamAudioBusTraits>;
    using TuSteamAudioInterface = AZ::Interface<TuSteamAudioRequests>;


    class SteamAudioEffectRequests
    {
    public:
        AZ_RTTI(SteamAudioEffectRequests, "{9BF64AEF-4445-4B77-868D-0A558A45CF73}");

        virtual void SetDistanceModel(DistanceModel model) = 0;
        virtual void SetTuAttenuationSettings(Attenuation::TuAttenuation settings) = 0;
    };

    using SteamAudioEffectRequestBus = AZ::EBus<SteamAudioEffectRequests, Sune::PlayerEffectBusTraits>;
} // namespace TuSteamAudio
