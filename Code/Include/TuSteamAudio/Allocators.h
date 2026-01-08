/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Memory/ChildAllocatorSchema.h>

namespace TuSteamAudio
{
    AZ_CHILD_ALLOCATOR_WITH_NAME(SteamAudioAllocator, "SteamAudioAllocator", "{5D29062B-71AF-4DD4-828C-42476C7B2EA8}", AZ::SystemAllocator);
}