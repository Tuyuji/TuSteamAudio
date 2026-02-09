/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "TuSteamAudioSystemComponent.h"

#include <TuSteamAudio/TuSteamAudioTypeIds.h>
#include <TuSteamAudio/Utils.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <phonon.h>
#include <Sune/SuneBus.h>

#include "Effects/SteamAudioHrtf.h"
#include "TuSteamAudio/Allocators.h"

namespace TuSteamAudio
{
    AZ_COMPONENT_IMPL(TuSteamAudioSystemComponent, "TuSteamAudioSystemComponent",
        TuSteamAudioSystemComponentTypeId);

    void TuSteamAudioSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        TuSteamAudio::Attenuation::TuAttenuation::Reflect(context);
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TuSteamAudioSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void TuSteamAudioSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TuSteamAudioService"));
    }

    void TuSteamAudioSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("TuSteamAudioService"));
    }

    void TuSteamAudioSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("SuneService"));
    }

    void TuSteamAudioSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    TuSteamAudioSystemComponent::TuSteamAudioSystemComponent()
    {
        if (TuSteamAudioInterface::Get() == nullptr)
        {
            TuSteamAudioInterface::Register(this);
        }
    }

    TuSteamAudioSystemComponent::~TuSteamAudioSystemComponent()
    {
        if (TuSteamAudioInterface::Get() == this)
        {
            TuSteamAudioInterface::Unregister(this);
        }
    }

    void TuSteamAudioSystemComponent::Init()
    {
    }

    static AZ::IAllocator* allocator = nullptr;

    [[maybe_unused]]static void* saAlloc(IPLsize size, IPLsize alignment)
    {
        return allocator->allocate(size, alignment);
    }

    [[maybe_unused]]static void saFree(void* ptr)
    {
        allocator->deallocate(ptr);
    }

    static void saLog(IPLLogLevel level, const char* message)
    {
        switch (level)
        {
        case IPL_LOGLEVEL_INFO:
        case IPL_LOGLEVEL_DEBUG:
            AZ_Info("TuSteamAudio", message);
            break;
        case IPL_LOGLEVEL_WARNING:
            AZ_Warning("TuSteamAudio", false, message);
            break;
        case IPL_LOGLEVEL_ERROR:
            AZ_Error("TuSteamAudio", false, message);
            break;
        default:
            break;
        }
    }

    void TuSteamAudioSystemComponent::Activate()
    {
        allocator = &AZ::AllocatorInstance<SteamAudioAllocator>::Get();
        IPLContextSettings contextSettings = {};
        contextSettings.version = STEAMAUDIO_VERSION;
        contextSettings.logCallback = &saLog;
        contextSettings.allocateCallback = &saAlloc;
        contextSettings.freeCallback = &saFree;
        contextSettings.flags = IPL_CONTEXTFLAGS_VALIDATION;

        IPLerror err = iplContextCreate(&contextSettings, &m_context);
        if (err != IPL_STATUS_SUCCESS)
        {
            AZ_Error("TuSteamAudio", false, "Failed to create Phonon context.");
            return;
        }

        m_audioSettings = {};
        m_audioSettings.frameSize = Sune::SuneInterface::Get()->GetPeriodSizeInFrames();
        m_audioSettings.samplingRate = Sune::SuneInterface::Get()->GetLabContext()->sampleRate();

        IPLHRTFSettings hrtfSettings = {};
        hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        hrtfSettings.volume = 1.0f;

        err = iplHRTFCreate(m_context, &m_audioSettings, &hrtfSettings, &m_hrtf);
        if (err != IPL_STATUS_SUCCESS)
        {
            AZ_Error("TuSteamAudio", false, "Failed to create Phonon HRTF.");
            return;
        }

        IPLSceneSettings sceneSettings = {};
        sceneSettings.type = IPL_SCENETYPE_DEFAULT;

        err = iplSceneCreate(m_context, &sceneSettings, &m_scene);
        if (err != IPL_STATUS_SUCCESS)
        {
            AZ_Error("TuSteamAudio", false, "Failed to create Phonon scene.");
            return;
        }

        IPLSimulationSettings simulationSettings = {};
        simulationSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;
        simulationSettings.sceneType = sceneSettings.type;
        simulationSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
        simulationSettings.maxNumOcclusionSamples = 32;
        simulationSettings.maxNumRays = 4096;
        simulationSettings.numDiffuseSamples = 32;
        simulationSettings.maxDuration = 2.0f;
        simulationSettings.maxOrder = 1;
        simulationSettings.maxNumSources = 24;
        simulationSettings.numThreads = 4;
        simulationSettings.numVisSamples = 32;
        simulationSettings.samplingRate = m_audioSettings.samplingRate;
        simulationSettings.frameSize = m_audioSettings.frameSize;

        err = iplSimulatorCreate(m_context, &simulationSettings, &m_simulator);
        if (err != IPL_STATUS_SUCCESS)
        {
            AZ_Error("TuSteamAudio", false, "Failed to create Phonon simulator.");
            return;
        }

        Sune::PlayerEffectFactoryBus::MultiHandler::BusConnect(SteamAudioHrtf::RegisterName);

        TuSteamAudioRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void TuSteamAudioSystemComponent::Deactivate()
    {
        Sune::PlayerEffectFactoryBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        TuSteamAudioRequestBus::Handler::BusDisconnect();

        iplSimulatorRelease(&m_simulator);
        m_simulator = nullptr;

        iplHRTFRelease(&m_hrtf);
        m_hrtf = nullptr;

        iplContextRelease(&m_context);
        m_context = nullptr;
    }

    void TuSteamAudioSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        iplSimulatorCommit(m_simulator);
        //get labsound ctx
        auto labContext = Sune::SuneInterface::Get()->GetLabContext();
        if (!labContext)
        {
            return;
        }

        auto listener = labContext->listener();
        if (!listener)
        {
            return;
        }

        IPLVector3 listenerPos = { listener->positionX()->value(), listener->positionY()->value(), listener->positionZ()->value() };
        IPLVector3 listenerForward = { listener->forwardX()->value(), listener->forwardY()->value(), listener->forwardZ()->value() };
        IPLVector3 listenerUp = { listener->upX()->value(), listener->upY()->value(), listener->upZ()->value() };

        // Compute right vector as cross product of ahead and up (left-handed coordinate system)
        IPLVector3 listenerRight;
        listenerRight.x = listenerForward.y * listenerUp.z - listenerForward.z * listenerUp.y;
        listenerRight.y = listenerForward.z * listenerUp.x - listenerForward.x * listenerUp.z;
        listenerRight.z = listenerForward.x * listenerUp.y - listenerForward.y * listenerUp.x;

        IPLCoordinateSpace3 listenerCoords = {};
        listenerCoords.right = listenerRight;
        listenerCoords.up = listenerUp;
        listenerCoords.ahead = listenerForward;
        listenerCoords.origin = listenerPos;

        IPLSimulationSharedInputs shared_inputs = {};
        shared_inputs.listener = listenerCoords;
        shared_inputs.numRays = 4096;
        shared_inputs.numBounces = 16;
        shared_inputs.duration = 2.0f;
        shared_inputs.order = 1;
        shared_inputs.irradianceMinDistance = 1.0f;

        /*iplSimulatorSetScene(m_simulator, m_scene);
        iplSimulatorSetSharedInputs(m_simulator, IPL_SIMULATIONFLAGS_REFLECTIONS, &shared_inputs);
        iplSimulatorRunReflections(m_simulator);*/
    }

    Sune::IPlayerAudioEffect* TuSteamAudioSystemComponent::CreateEffect(AZ::Crc32 id)
    {
        if (id == SteamAudioHrtf::RegisterName)
        {
            return aznew SteamAudioHrtf();
        }

        return nullptr;
    }
} // namespace TuSteamAudio
