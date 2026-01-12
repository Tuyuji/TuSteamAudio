/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "AzCore/Component/Component.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "Clients/Components/Configs/SAPlayerComponentConfig.h"
#include "Sune/SuneBus.h"
#include "AzCore/Component/TransformBus.h"

namespace TuSteamAudio
{
    class SAPlayerComponentController
        : protected AZ::TransformNotificationBus::Handler
    {
    public:
        AZ_RTTI(SAPlayerComponentController, "{4346D1F8-8BBF-4E38-843F-F00B1E0068BD}");

        SAPlayerComponentController() = default;
        explicit SAPlayerComponentController(const SAPlayerComponentConfig& config);
        virtual ~SAPlayerComponentController() = default;

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        void SetConfiguration(const SAPlayerComponentConfig& config);
        const SAPlayerComponentConfig& GetConfiguration() const;

        void Activate(const AZ::EntityComponentIdPair& entityComponentIdPair);
        void Deactivate();
        void OnConfigurationUpdated();

        void OnTransformChanged(const AZ::Transform& _, const AZ::Transform& transform) override;
    private:
        friend class EditorSAPlayerComponent;
        friend class SAPlayerComponent;
        AZ::EntityComponentIdPair m_entityComponentIdPair;
        SAPlayerComponentConfig m_config;

        Sune::SoundPlayerId m_playerId;
        Sune::PlayerEffectId m_hrtfId = Sune::PlayerEffectId();
    };
} // TuSteamAudio