/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include <AzCore/Component/Component.h>
#include <AzFramework/Components/ComponentAdapter.h>

#include "Configs/SAPlayerComponentConfig.h"
#include "Controllers/SAPlayerComponentController.h"

namespace TuSteamAudio
{
    class SAPlayerComponent
        : public AzFramework::Components::ComponentAdapter<SAPlayerComponentController, SAPlayerComponentConfig>
    {
        using Super = AzFramework::Components::ComponentAdapter<SAPlayerComponentController, SAPlayerComponentConfig>;
    public:
        AZ_COMPONENT(SAPlayerComponent, "{8F5CF256-4671-45E6-B47F-33BE1611A3B7}", Super);

        static void Reflect(AZ::ReflectContext* context);

        SAPlayerComponent();
        explicit SAPlayerComponent(const SAPlayerComponentConfig& config);

        void Activate() override;
    };
} // TuSteamAudio