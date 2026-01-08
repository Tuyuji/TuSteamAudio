/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <TuSteamAudio/TuSteamAudioTypeIds.h>
#include <TuSteamAudioModuleInterface.h>
#include "TuSteamAudioEditorSystemComponent.h"
#include "Components/EditorSAPlayerComponent.h"

namespace TuSteamAudio
{
    class TuSteamAudioEditorModule
        : public TuSteamAudioModuleInterface
    {
    public:
        AZ_RTTI(TuSteamAudioEditorModule, TuSteamAudioEditorModuleTypeId, TuSteamAudioModuleInterface);
        AZ_CLASS_ALLOCATOR(TuSteamAudioEditorModule, AZ::SystemAllocator);

        TuSteamAudioEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                TuSteamAudioEditorSystemComponent::CreateDescriptor(),
                EditorSAPlayerComponent::CreateDescriptor()
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<TuSteamAudioEditorSystemComponent>(),
            };
        }
    };
}// namespace TuSteamAudio

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), TuSteamAudio::TuSteamAudioEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_TuSteamAudio_Editor, TuSteamAudio::TuSteamAudioEditorModule)
#endif
