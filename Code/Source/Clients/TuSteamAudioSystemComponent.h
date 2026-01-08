/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <TuSteamAudio/TuSteamAudioBus.h>

#include "phonon.h"


namespace TuSteamAudio
{
    class TuSteamAudioSystemComponent
        : public AZ::Component
        , protected TuSteamAudioRequestBus::Handler
        , protected AZ::TickBus::Handler
        , protected TuLabSound::PlayerEffectFactoryBus::MultiHandler
    {
    public:
        AZ_COMPONENT_DECL(TuSteamAudioSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        TuSteamAudioSystemComponent();
        ~TuSteamAudioSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // TuSteamAudioRequestBus interface implementation
        IPLHRTF GetHrtf() override
        {
            return m_hrtf;
        }

        IPLContext GetContext() override
        {
            return m_context;
        }

        IPLAudioSettings GetAudioSettings() override
        {
            return m_audioSettings;
        }

        IPLScene GetRootScene() override
        {
            return m_scene;
        }

        IPLSimulator GetSimulator() override
        {
            return m_simulator;
        }

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override
        {
            return AZ::TICK_LAST;
        }
        ////////////////////////////////////////////////////////////////////////

        TuLabSound::IPlayerAudioEffect* CreateEffect(const AZStd::string& id) override;

    private:
        IPLContext m_context;
        IPLAudioSettings m_audioSettings;
        IPLHRTF m_hrtf;

        IPLScene m_scene = nullptr;
        IPLSimulator m_simulator = nullptr;
    };

} // namespace TuSteamAudio
