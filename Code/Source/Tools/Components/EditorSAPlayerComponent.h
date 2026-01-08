/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include <AzCore/Component/Component.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentAdapter.h>

#include "API/ComponentEntitySelectionBus.h"
#include "AzFramework/Entity/EntityDebugDisplayBus.h"
#include "Clients/Components/Configs/SAPlayerComponentConfig.h"
#include "Clients/Components/Controllers/SAPlayerComponentController.h"
#include "Clients/Components/SAPlayerComponent.h"

namespace TuSteamAudio
{
    class EditorSAPlayerComponent
        : public AzToolsFramework::Components::EditorComponentAdapter
            <SAPlayerComponentController, SAPlayerComponent, SAPlayerComponentConfig>
        , protected AzFramework::EntityDebugDisplayEventBus::Handler
    {
        using Super = AzToolsFramework::Components::EditorComponentAdapter<SAPlayerComponentController, SAPlayerComponent, SAPlayerComponentConfig>;
    public:
        AZ_EDITOR_COMPONENT(EditorSAPlayerComponent, "{7762F3C8-6085-4A8E-A57E-F877A9BA97BB}", Super);

        static void Reflect(AZ::ReflectContext* context);

        EditorSAPlayerComponent() = default;
        explicit EditorSAPlayerComponent(const SAPlayerComponentConfig& config);

        void Activate() override;
        void Deactivate() override;
        AZ::u32 OnConfigurationChanged() override;

        void DisplayEntityViewport(const AzFramework::ViewportInfo &, AzFramework::DebugDisplayRequests &) override;

        AZ::u32 OnDistanceModelChanged();
        AZ::u32 OnAttenuationSettingsChanged();
    };
} // TuSteamAudio