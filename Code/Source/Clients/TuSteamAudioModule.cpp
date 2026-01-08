/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <TuSteamAudio/TuSteamAudioTypeIds.h>
#include <TuSteamAudioModuleInterface.h>
#include "TuSteamAudioSystemComponent.h"

namespace TuSteamAudio
{
    class TuSteamAudioModule
        : public TuSteamAudioModuleInterface
    {
    public:
        AZ_RTTI(TuSteamAudioModule, TuSteamAudioModuleTypeId, TuSteamAudioModuleInterface);
        AZ_CLASS_ALLOCATOR(TuSteamAudioModule, AZ::SystemAllocator);
    };
}// namespace TuSteamAudio

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), TuSteamAudio::TuSteamAudioModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_TuSteamAudio, TuSteamAudio::TuSteamAudioModule)
#endif
