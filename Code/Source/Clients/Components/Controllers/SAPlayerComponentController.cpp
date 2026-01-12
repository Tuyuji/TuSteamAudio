/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SAPlayerComponentController.h"

#include "Clients/Effects/SteamAudioHrtf.h"
#include "Sune/AudioPlayerBus.h"

using namespace TuSteamAudio;

SAPlayerComponentController::SAPlayerComponentController(const SAPlayerComponentConfig& config)
    : m_config(config)
{
}

void SAPlayerComponentController::Reflect(AZ::ReflectContext* context)
{
    SAPlayerComponentConfig::Reflect(context);

    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<SAPlayerComponentController>()
            ->Version(0)
            ->Field("Config", &SAPlayerComponentController::m_config);
    }
}

void SAPlayerComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(Sune::AudioSpatializationEffectServiceName);
}

void SAPlayerComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(Sune::AudioSpatializationEffectServiceName);
}

void SAPlayerComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    required.push_back(Sune::AudioPlayerServiceName);
}

void SAPlayerComponentController::SetConfiguration(const SAPlayerComponentConfig& config)
{
    m_config = config;
}

const SAPlayerComponentConfig& SAPlayerComponentController::GetConfiguration() const
{
    return m_config;
}

void SAPlayerComponentController::Activate(const AZ::EntityComponentIdPair& entityComponentIdPair)
{
    m_entityComponentIdPair = entityComponentIdPair;
    Sune::AudioPlayerRequestBus::EventResult(m_playerId, m_entityComponentIdPair.GetEntityId(), &Sune::AudioPlayerRequestBus::Events::GetPlayerId);
    if (m_playerId == Sune::SoundPlayerId())
    {
        AZ_Error("Sune", false, "SAPlayerComponentController requires a Sune::AudioPlayerComponent to be present on the same entity.");
        return;
    }

    Sune::SoundPlayerRequestBus::EventResult(
        m_hrtfId,
        m_playerId,
        &Sune::SoundPlayerRequestBus::Events::AddEffect,
        SteamAudioHrtf::RegisterName);

    OnConfigurationUpdated();

    AZ::Transform transform = AZ::Transform::CreateIdentity();
    AZ::TransformBus::EventResult(transform, entityComponentIdPair.GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
    OnTransformChanged({}, transform);

    AZ::TransformNotificationBus::Handler::BusConnect(m_entityComponentIdPair.GetEntityId());
}

void SAPlayerComponentController::Deactivate()
{
    AZ::TransformNotificationBus::Handler::BusDisconnect();

    if (m_hrtfId != Sune::PlayerEffectId())
    {
        Sune::SoundPlayerRequestBus::Event(m_playerId, &Sune::SoundPlayerRequestBus::Events::RemoveEffect, m_hrtfId);
    }

    m_playerId = Sune::SoundPlayerId();
    m_hrtfId = Sune::PlayerEffectId();
}

void SAPlayerComponentController::OnConfigurationUpdated()
{
    SteamAudioEffectRequestBus::Event(m_hrtfId, &SteamAudioEffectRequestBus::Events::SetDistanceModel, m_config.m_distanceModel);
    SteamAudioEffectRequestBus::Event(m_hrtfId, &SteamAudioEffectRequestBus::Events::SetTuAttenuationSettings, m_config.m_attenuation);
}

void SAPlayerComponentController::OnTransformChanged(const AZ::Transform& _, const AZ::Transform& transform)
{
    Sune::PlayerEffectSpatializationRequestBus::Event(
        m_hrtfId,
        &Sune::PlayerEffectSpatializationRequestBus::Events::SetTransform,
        transform);
}
