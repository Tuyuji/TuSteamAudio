/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SAPlayerComponent.h"

using namespace TuSteamAudio;

void SAPlayerComponent::Reflect(AZ::ReflectContext* context)
{
    Super::Reflect(context);
    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<SAPlayerComponent, Super>()
            ->Version(0);
    }
}

SAPlayerComponent::SAPlayerComponent() = default;

SAPlayerComponent::SAPlayerComponent(const SAPlayerComponentConfig& config)
    : Super(config)
{
}

void SAPlayerComponent::Activate()
{
    Super::Activate();
}
