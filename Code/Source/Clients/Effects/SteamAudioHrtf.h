/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include "TuLabSound/PlayerAudioEffect.h"
#include "TuSteamAudio/Types.h"
#include "phonon.h"
#include <AzCore/Math/Vector3.h>

#include "AzCore/Math/Transform.h"
#include "TuSteamAudio/TuSteamAudioBus.h"


namespace TuSteamAudio
{
    class SteamAudioHrtfNode : public lab::AudioNode
    {
    public:
        SteamAudioHrtfNode(lab::AudioContext& ac);
        virtual ~SteamAudioHrtfNode();

        static const char* static_name() { return "SteamAudioHrtf"; }
        const char* name() const override{ return static_name(); }
        static lab::AudioNodeDescriptor* desc();

        void initialize() override;
        void uninitialize() override;

        void process(lab::ContextRenderLock&, int bufferSize) override;
        void reset(lab::ContextRenderLock&) override;

        void setTransform(const AZ::Transform& transform);
        void setSpatialBlend(float blend) { m_spatialBlend = blend; }
        void setInterpolation(IPLHRTFInterpolation interp) { m_interpolation = interp; }
        void updateTuAttenuationSettings(const Attenuation::TuAttenuation& settings);

        // Distance attenuation settings
        void setDistanceAttenuation(float minDistance)
        {
            m_distanceModel.type = IPL_DISTANCEATTENUATIONTYPE_INVERSEDISTANCE;
            m_distanceModel.minDistance = minDistance;
        }

        void useTuAttenuation()
        {
            m_distanceModel.type = IPL_DISTANCEATTENUATIONTYPE_CALLBACK;
            m_distanceModel.callback = DistanceAttenuationCallback;
            m_distanceModel.userData = &m_attenuation;
            m_distanceModel.dirty = IPL_TRUE;
        }

    protected:
        void UpdateBuffers(int inputChannelCount, int outputChannelCount, int sampleCount);
        void EnsureDirectEffectInitialized(int numChannels);
        double tailTime(lab::ContextRenderLock& r) const override;
        double latencyTime(lab::ContextRenderLock& r) const override { return 0; }

        static float IPLCALL DistanceAttenuationCallback(IPLfloat32 distance, void* userData);

    private:
        friend class SteamAudioHrtf;
        //Settings
        AZ::Transform m_transform = AZ::Transform::Identity();
        IPLHRTFInterpolation m_interpolation = IPL_HRTFINTERPOLATION_BILINEAR;
        Attenuation::TuAttenuation m_attenuation = {};
        float m_spatialBlend = 1.0f;

        //Globals retained
        IPLContext m_context = nullptr;
        IPLHRTF m_hrtf = nullptr;
        IPLScene m_scene = nullptr;
        IPLSimulator m_simulator = nullptr;

        //Per instance handles
        IPLSource m_source = {};
        IPLAudioBuffer m_directBuffer = {};

        //Effects
        IPLDirectEffect m_directEffect = {};
        IPLBinauralEffect m_binauralEffect = {};
        IPLReflectionEffect m_reflectionEffect = {};

        int m_lastInputChannelCount = 0;

        IPLDistanceAttenuationModel m_distanceModel = {
            IPL_DISTANCEATTENUATIONTYPE_INVERSEDISTANCE,
            1.0f, // minDistance
            nullptr, // callback
            nullptr, // userData
            IPL_FALSE // dirty
        };
        IPLAirAbsorptionModel m_airAbsModel = {
            IPL_AIRABSORPTIONTYPE_DEFAULT,
            {0.9f, 0.7f, 0.5f},
            nullptr,
            nullptr,
            IPL_FALSE
        };

    };

    class SteamAudioHrtf
        : public TuLabSound::IPlayerAudioEffect
        , public TuLabSound::PlayerEffectSpatializationRequestBus::Handler
        , public TuLabSound::PlayerEffectImGuiRequestBus::Handler
        , public SteamAudioEffectRequestBus::Handler
    {
    public:
        constexpr static const char* RegisterName = "SteamAudioHrtf";
        ~SteamAudioHrtf() override;
        virtual bool Initialize(lab::AudioContext& ac) override;
        virtual void Shutdown() override;

        std::shared_ptr<lab::AudioNode> GetInputNode() override;
        std::shared_ptr<lab::AudioNode> GetOutputNode() override;

        const char* GetEffectName() const override
        {
            return "SteamAudioHrtf";
        }
        TuLabSound::PlayerEffectOrder GetProcessingOrder() const override
        {
            return TuLabSound::PlayerEffectOrder::Spatializer;
        }

        // PlayerEffectSpatializationRequestBus
        void SetTransform(const AZ::Transform& transform) override;

        void SetDistanceModel(DistanceModel model) override;
        void SetTuAttenuationSettings(Attenuation::TuAttenuation settings) override;

        void DrawGui() override;

    private:
        std::shared_ptr<SteamAudioHrtfNode> m_node = {};
    };
} // TuSteamAudio