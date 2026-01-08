/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

namespace TuSteamAudio
{
    // System Component TypeIds
    inline constexpr const char* TuSteamAudioSystemComponentTypeId = "{79BD7E27-59A7-4A69-8518-77920F767161}";
    inline constexpr const char* TuSteamAudioEditorSystemComponentTypeId = "{868755E8-3BC1-407A-AF94-7E797646F53D}";

    // Module derived classes TypeIds
    inline constexpr const char* TuSteamAudioModuleInterfaceTypeId = "{BE85512E-C019-4D00-92D3-D7294EC63E9A}";
    inline constexpr const char* TuSteamAudioModuleTypeId = "{2DAD8806-A757-4CA9-8444-987FCA5B2378}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* TuSteamAudioEditorModuleTypeId = TuSteamAudioModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* TuSteamAudioRequestsTypeId = "{C5B90D4B-1F87-4A23-BA37-0E113DF93A50}";
} // namespace TuSteamAudio
