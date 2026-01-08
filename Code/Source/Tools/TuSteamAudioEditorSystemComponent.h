/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/TuSteamAudioSystemComponent.h>

namespace TuSteamAudio
{
    /// System component for TuSteamAudio editor
    class TuSteamAudioEditorSystemComponent
        : public TuSteamAudioSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = TuSteamAudioSystemComponent;
    public:
        AZ_COMPONENT_DECL(TuSteamAudioEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        TuSteamAudioEditorSystemComponent();
        ~TuSteamAudioEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        void NotifyRegisterViews() override;
    };
} // namespace TuSteamAudio
