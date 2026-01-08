/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "TuSteamAudioModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <TuSteamAudio/TuSteamAudioTypeIds.h>

#include <Clients/TuSteamAudioSystemComponent.h>

#include "Clients/Components/SAPlayerComponent.h"

namespace TuSteamAudio
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(TuSteamAudioModuleInterface,
        "TuSteamAudioModuleInterface", TuSteamAudioModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(TuSteamAudioModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(TuSteamAudioModuleInterface, AZ::SystemAllocator);

    TuSteamAudioModuleInterface::TuSteamAudioModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            TuSteamAudioSystemComponent::CreateDescriptor(),
            SAPlayerComponent::CreateDescriptor()
            });
    }

    AZ::ComponentTypeList TuSteamAudioModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<TuSteamAudioSystemComponent>(),
        };
    }
} // namespace TuSteamAudio
