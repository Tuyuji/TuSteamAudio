/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <AzCore/Serialization/SerializeContext.h>
#include "TuSteamAudioEditorSystemComponent.h"

#include <TuSteamAudio/TuSteamAudioTypeIds.h>

#include "SACompInfoWindow.h"
#include "API/ViewPaneOptions.h"

namespace TuSteamAudio
{
    AZ_COMPONENT_IMPL(TuSteamAudioEditorSystemComponent, "TuSteamAudioEditorSystemComponent",
        TuSteamAudioEditorSystemComponentTypeId, BaseSystemComponent);

    void TuSteamAudioEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TuSteamAudioEditorSystemComponent, TuSteamAudioSystemComponent>()
                ->Version(0);
        }
    }

    TuSteamAudioEditorSystemComponent::TuSteamAudioEditorSystemComponent() = default;

    TuSteamAudioEditorSystemComponent::~TuSteamAudioEditorSystemComponent() = default;

    void TuSteamAudioEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("TuSteamAudioEditorService"));
    }

    void TuSteamAudioEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("TuSteamAudioEditorService"));
    }

    void TuSteamAudioEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void TuSteamAudioEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void TuSteamAudioEditorSystemComponent::Activate()
    {
        TuSteamAudioSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void TuSteamAudioEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        TuSteamAudioSystemComponent::Deactivate();
    }

    void TuSteamAudioEditorSystemComponent::NotifyRegisterViews()
    {
        AzToolsFramework::ViewPaneOptions options;
        options.paneRect = QRect(100, 100, 500, 400);
        options.showInMenu = true;
        options.canHaveMultipleInstances = false;
        options.showOnToolsToolbar = false;

        AzToolsFramework::RegisterViewPane<SACompInfoWindow>("Steam Audio Attenuation Graph", "TuSteamAudio", options);
    }
} // namespace TuSteamAudio
