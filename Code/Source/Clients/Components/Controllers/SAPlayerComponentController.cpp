/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SAPlayerComponentController.h"

#include "Clients/Effects/SteamAudioHrtf.h"
#include "TuLabSound/AudioPlayerBus.h"

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
    provided.push_back(TuLabSound::AudioSpatializationEffectServiceName);
}

void SAPlayerComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(TuLabSound::AudioSpatializationEffectServiceName);
}

void SAPlayerComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    required.push_back(TuLabSound::AudioPlayerServiceName);
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
    TuLabSound::AudioPlayerRequestBus::EventResult(m_playerId, m_entityComponentIdPair.GetEntityId(), &TuLabSound::AudioPlayerRequestBus::Events::GetPlayerId);
    if (m_playerId == TuLabSound::SoundPlayerId())
    {
        AZ_Error("TuLabSound", false, "SAPlayerComponentController requires a TuLabSound::AudioPlayerComponent to be present on the same entity.");
        return;
    }

    TuLabSound::TuSoundPlayerRequestBus::EventResult(
        m_hrtfId,
        m_playerId,
        &TuLabSound::TuSoundPlayerRequestBus::Events::AddEffect,
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

    if (m_hrtfId != TuLabSound::PlayerEffectId())
    {
        TuLabSound::TuSoundPlayerRequestBus::Event(m_playerId, &TuLabSound::TuSoundPlayerRequestBus::Events::RemoveEffect, m_hrtfId);
    }

    m_playerId = TuLabSound::SoundPlayerId();
    m_hrtfId = TuLabSound::PlayerEffectId();
}

void SAPlayerComponentController::OnConfigurationUpdated()
{
    SteamAudioEffectRequestBus::Event(m_hrtfId, &SteamAudioEffectRequestBus::Events::SetDistanceModel, m_config.m_distanceModel);
    SteamAudioEffectRequestBus::Event(m_hrtfId, &SteamAudioEffectRequestBus::Events::SetTuAttenuationSettings, m_config.m_attenuation);
}

void SAPlayerComponentController::OnTransformChanged(const AZ::Transform& _, const AZ::Transform& transform)
{
    TuLabSound::PlayerEffectSpatializationRequestBus::Event(
        m_hrtfId,
        &TuLabSound::PlayerEffectSpatializationRequestBus::Events::SetTransform,
        transform);
}
