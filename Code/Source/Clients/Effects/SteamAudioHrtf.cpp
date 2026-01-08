/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SteamAudioHrtf.h"

#include <LabSound/core/AudioNodeInput.h>
#include <LabSound/core/AudioNodeOutput.h>
#include <LabSound/core/AudioBus.h>
#include <LabSound/core/AudioContext.h>
#include <TuSteamAudio/TuSteamAudioBus.h>
#include <TuLabSound/Utils.h>

#include <AzCore/Math/MathUtils.h>
#include <cmath>
#include <AzCore/Debug/Profiler.h>

#include "imgui/imgui.h"
#include "TuSteamAudio/Utils.h"

using namespace TuSteamAudio;

lab::AudioNodeDescriptor* SteamAudioHrtfNode::desc()
{
    static lab::AudioNodeDescriptor d = {nullptr, nullptr, 2};
    return &d;
}

void SteamAudioHrtfNode::initialize()
{
    if (isInitialized())
        return;

    m_context = iplContextRetain(TuSteamAudioInterface::Get()->GetContext());
    m_hrtf = iplHRTFRetain(TuSteamAudioInterface::Get()->GetHrtf());
    m_scene = iplSceneRetain(TuSteamAudioInterface::Get()->GetRootScene());
    m_simulator = iplSimulatorRetain(TuSteamAudioInterface::Get()->GetSimulator());

    IPLAudioSettings audioSettings = TuSteamAudioInterface::Get()->GetAudioSettings();

    // Create binaural effect
    IPLBinauralEffectSettings effectSettings{};
    effectSettings.hrtf = m_hrtf;

    IPLerror err = iplBinauralEffectCreate(m_context, &audioSettings, &effectSettings, &m_binauralEffect);
    if (err != IPL_STATUS_SUCCESS)
    {
        AZ_Error("SteamAudioHrtfNode", false, "Failed to create binaural effect");
        m_binauralEffect = nullptr;
    }

    IPLSourceSettings sourceSettings{};
    sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;

    err = iplSourceCreate(m_simulator, &sourceSettings, &m_source);
    if (err != IPL_STATUS_SUCCESS)
    {
        AZ_Error("SteamAudioHrtfNode", false, "Failed to create source");
        m_source = nullptr;
    }
    else
    {
        iplSourceAdd(m_source, m_simulator);
    }

    IPLReflectionEffectSettings refSettings = {};
    refSettings.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    refSettings.irSize = audioSettings.samplingRate * 2.0f;
    refSettings.numChannels = 2;

    err = iplReflectionEffectCreate(m_context, &audioSettings, &refSettings, &m_reflectionEffect);
    if (err != IPL_STATUS_SUCCESS)
    {
        AZ_Error("SteamAudioHrtfNode", false, "Failed to create reflection effect");
        m_reflectionEffect = nullptr;
    }

    AudioNode::initialize();
}

void SteamAudioHrtfNode::uninitialize()
{
    if (!isInitialized())
        return;

    if (m_source)
    {
        iplSourceRemove(m_source, m_simulator);
        iplSourceRelease(&m_source);
        m_source = nullptr;
    }

    if (m_directBuffer.numChannels > 0)
    {
        iplAudioBufferFree(m_context, &m_directBuffer);
        m_directBuffer.numChannels = 0;
    }

    if (m_reflectionEffect)
    {
        iplReflectionEffectRelease(&m_reflectionEffect);
        m_reflectionEffect = nullptr;
    }

    if (m_binauralEffect)
    {
        iplBinauralEffectRelease(&m_binauralEffect);
        m_binauralEffect = nullptr;
    }

    if (m_directEffect)
    {
        iplDirectEffectRelease(&m_directEffect);
        m_directEffect = nullptr;
    }

    m_lastInputChannelCount = 0;
    iplSimulatorRelease(&m_simulator);
    iplSceneRelease(&m_scene);
    iplHRTFRelease(&m_hrtf);
    iplContextRelease(&m_context);

    AudioNode::uninitialize();
}

SteamAudioHrtfNode::SteamAudioHrtfNode(lab::AudioContext& ac)
    : AudioNode(ac, *desc())
{
    addInput(std::unique_ptr<lab::AudioNodeInput>(new lab::AudioNodeInput(this)));

    initialize();
}

SteamAudioHrtfNode::~SteamAudioHrtfNode()
{
    uninitialize();
}

void SteamAudioHrtfNode::process(lab::ContextRenderLock& r, int bufferSize)
{
    AZ_PROFILE_FUNCTION(Audio);
    if (bufferSize != TuLabSound::TuLabSoundInterface::Get()->GetPeriodSizeInFrames())
    {
        return;
    }

    lab::AudioBus* outputBus = output(0)->bus(r);
    if (outputBus == nullptr)
        return;

    if (!isInitialized() || !input(0)->isConnected() || !m_binauralEffect)
    {
        outputBus->zero();
        return;
    }

    lab::AudioBus* inputBus = input(0)->bus(r);

    if (!inputBus)
    {
        outputBus->zero();
        return;
    }

    // Get listener position and orientation from AudioContext
    auto listener = r.context()->listener();

    // Calculate direction from listener to source
    // Convert O3DE position to LabSound/Steam Audio coords using the existing utility
    auto sourcePos = TuLabSound::ToLab(m_transform.GetTranslation());

    IPLVector3 sourceIPL = { sourcePos.x, sourcePos.y, sourcePos.z };
    IPLVector3 listenerIPL = { listener->positionX()->value(), listener->positionY()->value(), listener->positionZ()->value() };
    IPLVector3 forwardIPL = { listener->forwardX()->value(), listener->forwardY()->value(), listener->forwardZ()->value() };
    IPLVector3 upIPL = { listener->upX()->value(), listener->upY()->value(), listener->upZ()->value() };

    IPLVector3 direction = iplCalculateRelativeDirection(m_context, sourceIPL, listenerIPL, forwardIPL, upIPL);

    // Setup input buffer - LabSound already uses a deinterleaved format
    const float* inputChannels[16];
    for (int i = 0; i < inputBus->numberOfChannels(); ++i)
    {
        inputChannels[i] = inputBus->channel(i)->data();
    }

    float* outputChannels[16];
    for (int i = 0; i < outputBus->numberOfChannels(); ++i)
    {
        outputChannels[i] = outputBus->channel(i)->mutableData();
    }

    IPLAudioBuffer inBuffer{};
    inBuffer.numChannels = inputBus->numberOfChannels();
    inBuffer.numSamples = bufferSize;
    inBuffer.data = const_cast<float**>(inputChannels);

    IPLAudioBuffer outBuffer{};
    outBuffer.numChannels = outputBus->numberOfChannels();
    outBuffer.numSamples = bufferSize;
    outBuffer.data = outputChannels;

    EnsureDirectEffectInitialized(inputBus->numberOfChannels());

    UpdateBuffers(inputBus->numberOfChannels(), outputBus->numberOfChannels(), bufferSize);

    if (!m_directEffect)
    {
        outputBus->zero();
        return;
    }

    // IPLSimulationOutputs outputs = {};
    // iplSourceGetOutputs(m_source, IPL_SIMULATIONFLAGS_REFLECTIONS, &outputs);

    float distanceAttenuation = iplDistanceAttenuationCalculate(m_context, sourceIPL, listenerIPL, &m_distanceModel);

    // Modify spatial blend and distance attenuation to allow them to interact properly
    // This prevents audio from cutting out abruptly when sources get very far away
    // Formula from Unity's Steam Audio implementation
    float _distanceAttenuation = (1.0f - m_spatialBlend) + m_spatialBlend * distanceAttenuation;
    float _spatialBlend = (m_spatialBlend == 1.0f && distanceAttenuation == 0.0f) ? 1.0f :
                          m_spatialBlend * distanceAttenuation / _distanceAttenuation;

    IPLDirectEffectParams directParams{};
    directParams.flags = static_cast<IPLDirectEffectFlags>(IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION |
        IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION);
    directParams.distanceAttenuation = _distanceAttenuation;
    iplAirAbsorptionCalculate(m_context, sourceIPL, listenerIPL, &m_airAbsModel, directParams.airAbsorption);

    iplDirectEffectApply(m_directEffect, &directParams, &inBuffer, &m_directBuffer);

    IPLBinauralEffectParams params{};
    params.direction = direction;
    params.interpolation = m_interpolation;
    params.spatialBlend = _spatialBlend;
    params.hrtf = m_hrtf;
    params.peakDelays = nullptr;
    iplBinauralEffectApply(m_binauralEffect, &params, &m_directBuffer, &outBuffer);
}

void SteamAudioHrtfNode::reset(lab::ContextRenderLock&)
{
    if (m_binauralEffect)
    {
        iplBinauralEffectReset(m_binauralEffect);
    }
}

void SteamAudioHrtfNode::setTransform(const AZ::Transform& transform)
{
    m_transform = transform;

    IPLSimulationInputs inputs = {};
    inputs.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;
    inputs.source = ToIPL(m_transform);
    inputs.numOcclusionSamples = 32;
    inputs.numTransmissionRays = 32;

    if (m_distanceModel.type == IPL_DISTANCEATTENUATIONTYPE_CALLBACK)
    {
        inputs.occlusionRadius = m_attenuation.m_innerRadius;
    }else
    {
        inputs.occlusionRadius = m_distanceModel.minDistance;
    }

    inputs.distanceAttenuationModel = m_distanceModel;

    inputs.airAbsorptionModel = m_airAbsModel;

    iplSourceSetInputs(m_source, IPL_SIMULATIONFLAGS_REFLECTIONS, &inputs);
}

void SteamAudioHrtfNode::updateTuAttenuationSettings(const Attenuation::TuAttenuation& settings)
{
    m_attenuation = settings;
    m_distanceModel.dirty = IPL_TRUE;
    setTransform(m_transform);
}

void SteamAudioHrtfNode::EnsureDirectEffectInitialized(int numChannels)
{
    // If channel count changed or effect not created yet, recreate the direct effect
    if (m_lastInputChannelCount != numChannels || !m_directEffect)
    {
        IPLAudioSettings audioSettings = TuSteamAudioInterface::Get()->GetAudioSettings();

        // Release old effect if it exists
        if (m_directEffect)
        {
            iplDirectEffectRelease(&m_directEffect);
            m_directEffect = nullptr;
        }

        // Create new direct effect with correct channel count
        IPLDirectEffectSettings directEffectSettings{};
        directEffectSettings.numChannels = numChannels;

        IPLerror err = iplDirectEffectCreate(m_context, &audioSettings, &directEffectSettings, &m_directEffect);
        if (err != IPL_STATUS_SUCCESS)
        {
            AZ_Error("SteamAudioHrtfNode", false, "Failed to create direct effect with %d channels", numChannels);
            m_directEffect = nullptr;
        }
        else
        {
            m_lastInputChannelCount = numChannels;
        }
    }
}

void SteamAudioHrtfNode::UpdateBuffers(int inputChannelCount, int outputChannelCount, int sampleCount)
{
    if (m_directBuffer.numChannels != inputChannelCount || m_directBuffer.numSamples != sampleCount)
    {
        iplAudioBufferFree(m_context, &m_directBuffer);
        iplAudioBufferAllocate(m_context, inputChannelCount, sampleCount, &m_directBuffer);
    }
}

double SteamAudioHrtfNode::tailTime(lab::ContextRenderLock& r) const
{
    IPLint32 tailSamples = 0;
    if (m_binauralEffect)
    {
        tailSamples = iplBinauralEffectGetTailSize(m_binauralEffect);
    }
    if (m_directEffect)
    {
        tailSamples = AZStd::max(tailSamples, iplDirectEffectGetTailSize(m_directEffect));
    }

    return static_cast<double>(tailSamples) / r.context()->sampleRate();
}

float SteamAudioHrtfNode::DistanceAttenuationCallback(IPLfloat32 distance, void* userData)
{
    auto* settings = static_cast<Attenuation::TuAttenuation*>(userData);
    return settings->CalculateAttenuation(distance);
}

SteamAudioHrtf::~SteamAudioHrtf()
{

}

bool SteamAudioHrtf::Initialize(lab::AudioContext& ac)
{
    m_node = std::make_shared<SteamAudioHrtfNode>(ac);

    // Connect to spatialization bus
    TuLabSound::PlayerEffectSpatializationRequestBus::Handler::BusConnect(GetId());
    TuLabSound::PlayerEffectImGuiRequestBus::Handler::BusConnect(GetId());
    SteamAudioEffectRequestBus::Handler::BusConnect(GetId());

    return m_node != nullptr;
}

void SteamAudioHrtf::Shutdown()
{
    SteamAudioEffectRequestBus::Handler::BusDisconnect();
    TuLabSound::PlayerEffectImGuiRequestBus::Handler::BusDisconnect();
    TuLabSound::PlayerEffectSpatializationRequestBus::Handler::BusDisconnect();
    m_node = nullptr;
}

std::shared_ptr<lab::AudioNode> SteamAudioHrtf::GetInputNode()
{
    return m_node;
}

std::shared_ptr<lab::AudioNode> SteamAudioHrtf::GetOutputNode()
{
    return m_node;
}

void SteamAudioHrtf::SetTransform(const AZ::Transform& transform)
{
    if (m_node)
    {
        m_node->setTransform(transform);
    }
}

void SteamAudioHrtf::SetDistanceModel(DistanceModel model)
{
    if (model == DistanceModel::TuAttenuation)
    {
        m_node->useTuAttenuation();
        return;
    }

    if (model == DistanceModel::Default)
    {
        m_node->m_distanceModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
    }else
    {
        m_node->m_distanceModel.type = IPL_DISTANCEATTENUATIONTYPE_INVERSEDISTANCE;
    }
}

void SteamAudioHrtf::SetTuAttenuationSettings(Attenuation::TuAttenuation settings)
{
    m_node->updateTuAttenuationSettings(settings);
}

void SteamAudioHrtf::DrawGui()
{
    if (!m_node)
        return;

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "Steam Audio HRTF Spatialization:");

    // Minimap visualization
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Position Minimap (Top-Down):");

    // Get listener position from the audio context
    auto ac = TuLabSound::TuLabSoundInterface::Get()->GetLabContext();
    if (ac)
    {
        auto listener = ac->listener();
        IPLVector3 listenerIPL = {
            static_cast<float>(listener->positionX()->value()),
            static_cast<float>(listener->positionY()->value()),
            static_cast<float>(listener->positionZ()->value())
        };

        // Get listener orientation (forward direction for rotation)
        IPLVector3 forwardIPL = {
            static_cast<float>(listener->forwardX()->value()),
            static_cast<float>(listener->forwardY()->value()),
            static_cast<float>(listener->forwardZ()->value())
        };

        // Convert source position to IPL coords
        auto sourcePos = TuLabSound::ToLab(m_node->m_transform.GetTranslation());
        IPLVector3 sourceIPL = { sourcePos.x, sourcePos.y, sourcePos.z };

        // Calculate distance
        float dx = sourceIPL.x - listenerIPL.x;
        float dy = sourceIPL.y - listenerIPL.y;
        float dz = sourceIPL.z - listenerIPL.z;
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

        // Minimap settings
        const float mapSize = 200.0f;  // Size in pixels
        const float mapScale = 50.0f;  // Meters visible on map
        static float zoom = 1.0f;

        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size(mapSize, mapSize);
        ImVec2 canvas_center(canvas_pos.x + mapSize * 0.5f, canvas_pos.y + mapSize * 0.5f);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Draw background
        draw_list->AddRectFilled(canvas_pos,
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(20, 20, 25, 255));
        draw_list->AddRect(canvas_pos,
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(100, 100, 100, 255));

        // Draw distance circles (concentric rings)
        float ringSpacing = (mapScale / zoom) / 4.0f; // 4 rings
        for (int i = 1; i <= 4; i++)
        {
            float ringRadius = (mapSize * 0.5f) * (i / 4.0f);
            draw_list->AddCircle(canvas_center, ringRadius,
                IM_COL32(40, 40, 50, 100), 32, 1.0f);

            // Label the ring with distance
            char label[32];
            snprintf(label, sizeof(label), "%.0fm", ringSpacing * i);
            ImVec2 labelPos(canvas_center.x + ringRadius + 5.0f, canvas_center.y - 8.0f);
            draw_list->AddText(labelPos, IM_COL32(100, 100, 100, 150), label);
        }

        // Draw axes (X = red, Z = blue for top-down view)
        draw_list->AddLine(
            ImVec2(canvas_center.x - 30.0f, canvas_center.y),
            ImVec2(canvas_center.x + 30.0f, canvas_center.y),
            IM_COL32(255, 100, 100, 150), 1.5f); // X axis (horizontal)
        draw_list->AddLine(
            ImVec2(canvas_center.x, canvas_center.y - 30.0f),
            ImVec2(canvas_center.x, canvas_center.y + 30.0f),
            IM_COL32(100, 100, 255, 150), 1.5f); // Z axis (vertical)

        // Draw source at center (green circle)
        draw_list->AddCircleFilled(canvas_center, 8.0f, IM_COL32(50, 255, 100, 255), 16);
        draw_list->AddCircle(canvas_center, 8.0f, IM_COL32(255, 255, 255, 255), 16, 2.0f);
        draw_list->AddText(ImVec2(canvas_center.x - 25.0f, canvas_center.y - 25.0f),
            IM_COL32(50, 255, 100, 255), "Source");

        // Calculate listener position relative to source (top-down: X, Z)
        // In top-down view: X is horizontal, Z is vertical (depth)
        float relX = listenerIPL.x - sourceIPL.x;
        float relZ = listenerIPL.z - sourceIPL.z;

        // Scale to map coordinates
        float pixelX = (relX / (mapScale / zoom)) * (mapSize * 0.5f);
        float pixelZ = (relZ / (mapScale / zoom)) * (mapSize * 0.5f);

        ImVec2 listenerPos(canvas_center.x + pixelX, canvas_center.y + pixelZ);

        // Draw line from source to listener
        draw_list->AddLine(canvas_center, listenerPos,
            IM_COL32(255, 255, 0, 150), 1.5f);

        // Use forward vector directly to orient the triangle (top-down view uses X and Z)
        // Normalize the forward vector (X, Z components)
        float forwardLen = std::sqrt(forwardIPL.x * forwardIPL.x + forwardIPL.z * forwardIPL.z);
        float normX = forwardIPL.x / forwardLen;
        float normZ = forwardIPL.z / forwardLen;

        // Create perpendicular vector for the triangle base (rotate 90 degrees)
        float perpX = -normZ;
        float perpZ = normX;

        // Define listener triangle vertices using the forward and perpendicular vectors
        const float triSize = 8.0f;
        ImVec2 triPoints[3];

        // Front point: along forward direction
        triPoints[0] = ImVec2(listenerPos.x + normX * triSize,
                              listenerPos.y + normZ * triSize);

        // Back left: back along forward, left along perpendicular
        triPoints[1] = ImVec2(listenerPos.x - normX * 0.8f * triSize + perpX * 0.7f * triSize,
                              listenerPos.y - normZ * 0.8f * triSize + perpZ * 0.7f * triSize);

        // Back right: back along forward, right along perpendicular
        triPoints[2] = ImVec2(listenerPos.x - normX * 0.8f * triSize - perpX * 0.7f * triSize,
                              listenerPos.y - normZ * 0.8f * triSize - perpZ * 0.7f * triSize);

        // Draw listener triangle
        draw_list->AddTriangleFilled(triPoints[0], triPoints[1], triPoints[2],
            IM_COL32(255, 150, 50, 255));
        draw_list->AddTriangle(triPoints[0], triPoints[1], triPoints[2],
            IM_COL32(255, 255, 255, 255), 2.0f);

        draw_list->AddText(ImVec2(listenerPos.x + 12.0f, listenerPos.y - 8.0f),
            IM_COL32(255, 150, 50, 255), "Listener");

        // Reserve space for the canvas
        ImGui::Dummy(canvas_size);

        // Display info below map
        ImGui::Text("Distance: %.2f m", distance);
        ImGui::Text("Source: (%.1f, %.1f, %.1f)", sourceIPL.x, sourceIPL.y, sourceIPL.z);
        ImGui::Text("Listener: (%.1f, %.1f, %.1f)", listenerIPL.x, listenerIPL.y, listenerIPL.z);
        ImGui::SliderFloat("Zoom", &zoom, 0.5f, 5.0f);
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Audio context not available");
    }

    ImGui::Separator();

    // HRTF Interpolation
    int interpIndex = m_node->m_interpolation;
    const char* interpModes[] = { "Nearest", "Bilinear" };
    if (ImGui::Combo("HRTF Interpolation", &interpIndex, interpModes, IM_ARRAYSIZE(interpModes)))
    {
        m_node->setInterpolation(static_cast<IPLHRTFInterpolation>(interpIndex));
    }

    // Spatial Blend
    float spatialBlend = m_node->m_spatialBlend;
    if (ImGui::SliderFloat("Spatial Blend", &spatialBlend, 0.0f, 1.0f))
    {
        m_node->setSpatialBlend(spatialBlend);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("0 = Dry (no HRTF), 1 = Fully spatialized");
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.3f, 1.0f), "Distance Attenuation:");

    // Distance attenuation model
    int distModelIndex = m_node->m_distanceModel.type;
    const char* distModels[] = { "Default (Inverse)", "Inverse Distance", "TuAttenuation" };
    if (ImGui::Combo("Attenuation Model", &distModelIndex, distModels, IM_ARRAYSIZE(distModels)))
    {
        if (distModelIndex == 2)
        {
            m_node->useTuAttenuation();
        }else
        {
            m_node->m_distanceModel.type = static_cast<IPLDistanceAttenuationModelType>(distModelIndex);
        }
    }

    if (m_node->m_distanceModel.type == IPL_DISTANCEATTENUATIONTYPE_CALLBACK)
    {
        // Using Attenuation::TuAttenuation - expose all parameters
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.8f, 1.0f), "TuAttenuation Settings:");

        // Shape selector (currently only Sphere is available)
        int shapeIndex = static_cast<int>(m_node->m_attenuation.m_shape);
        const char* shapes[] = { "Sphere" };
        if (ImGui::Combo("Attenuation Shape", &shapeIndex, shapes, IM_ARRAYSIZE(shapes)))
        {
            m_node->m_attenuation.m_shape = static_cast<Attenuation::Shape>(shapeIndex);
            m_node->m_distanceModel.dirty = IPL_TRUE;
        }

        // Inner Radius
        float innerRadius = m_node->m_attenuation.m_innerRadius;
        if (ImGui::DragFloat("Inner Radius", &innerRadius, 0.1f, 0.0f, 1000.0f, "%.1f m"))
        {
            m_node->m_attenuation.m_innerRadius = innerRadius;
            m_node->m_distanceModel.dirty = IPL_TRUE;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("No attenuation applied within this radius");
        }

        // Falloff Distance
        float falloffDistance = m_node->m_attenuation.m_falloffDistance;
        if (ImGui::DragFloat("Falloff Distance", &falloffDistance, 1.0f, 0.1f, 10000.0f, "%.1f m"))
        {
            m_node->m_attenuation.m_falloffDistance = AZ::GetMax(falloffDistance, 0.1f);
            m_node->m_distanceModel.dirty = IPL_TRUE;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Distance over which attenuation occurs (from inner radius)");
        }

        // Curve Type
        int curveIndex = static_cast<int>(m_node->m_attenuation.m_curveType);
        const char* curveTypes[] = { "Linear", "Logarithmic", "Inverse", "Log Reverse", "Natural Sound (1/d²)" };
        if (ImGui::Combo("Attenuation Curve", &curveIndex, curveTypes, IM_ARRAYSIZE(curveTypes)))
        {
            m_node->m_attenuation.m_curveType = static_cast<Attenuation::TuAttenuation::CurveType>(curveIndex);
            m_node->m_distanceModel.dirty = IPL_TRUE;
        }
        if (ImGui::IsItemHovered())
        {
            const char* curveDescriptions[] = {
                "Linear: Constant falloff rate",
                "Logarithmic: Gradual then rapid falloff",
                "Inverse: 1/d falloff (classic distance attenuation)",
                "Log Reverse: Rapid then gradual falloff",
                "Natural Sound: 1/d² falloff (physically accurate inverse square law)"
            };
            ImGui::SetTooltip("%s", curveDescriptions[curveIndex]);
        }

        // Attenuation Curve Exponent (for future custom curve use)
        float exponent = m_node->m_attenuation.m_attenuationCurveExponent;
        if (ImGui::DragFloat("Curve Exponent", &exponent, 0.01f, 0.1f, 10.0f, "%.2f"))
        {
            m_node->m_attenuation.m_attenuationCurveExponent = AZ::GetClamp(exponent, 0.1f, 10.0f);
            m_node->m_distanceModel.dirty = IPL_TRUE;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Controls curve steepness (reserved for future use)");
        }

        // Visual preview of the attenuation curve
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Attenuation Preview:");

        // Draw attenuation curve graph
        const float graphWidth = 300.0f;
        const float graphHeight = 100.0f;
        ImVec2 graphPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Background
        drawList->AddRectFilled(graphPos,
            ImVec2(graphPos.x + graphWidth, graphPos.y + graphHeight),
            IM_COL32(25, 25, 30, 255));
        drawList->AddRect(graphPos,
            ImVec2(graphPos.x + graphWidth, graphPos.y + graphHeight),
            IM_COL32(100, 100, 100, 255));

        // Draw grid lines
        for (int i = 1; i < 4; ++i)
        {
            float x = graphPos.x + (graphWidth * i / 4.0f);
            drawList->AddLine(ImVec2(x, graphPos.y), ImVec2(x, graphPos.y + graphHeight),
                IM_COL32(50, 50, 50, 100));

            float y = graphPos.y + (graphHeight * i / 4.0f);
            drawList->AddLine(ImVec2(graphPos.x, y), ImVec2(graphPos.x + graphWidth, y),
                IM_COL32(50, 50, 50, 100));
        }

        // Draw attenuation curve
        const int numPoints = 100;
        ImVec2 prevPoint = graphPos;
        for (int i = 0; i <= numPoints; ++i)
        {
            float t = static_cast<float>(i) / numPoints;
            float distance = m_node->m_attenuation.m_innerRadius +
                           (m_node->m_attenuation.m_falloffDistance * t);

            // Calculate attenuation using the callback (simulated)
            float attenuation = 1.0f;
            if (distance > m_node->m_attenuation.m_innerRadius)
            {
                float effectiveDist = distance - m_node->m_attenuation.m_innerRadius;
                float normalizedDist = effectiveDist / m_node->m_attenuation.m_falloffDistance;

                if (normalizedDist >= 1.0f)
                {
                    attenuation = 0.0f;
                }
                else
                {
                    switch (m_node->m_attenuation.m_curveType)
                    {
                        case Attenuation::TuAttenuation::CurveType::Linear:
                            attenuation = 1.0f - normalizedDist;
                            break;
                        case Attenuation::TuAttenuation::CurveType::Logarithmic:
                            attenuation = 1.0f - (std::log(normalizedDist * 9.0f + 1.0f) / std::log(10.0f));
                            break;
                        case Attenuation::TuAttenuation::CurveType::Inverse:
                            attenuation = m_node->m_attenuation.m_innerRadius / distance;
                            break;
                        case Attenuation::TuAttenuation::CurveType::NaturalSound:
                            {
                                float ratio = m_node->m_attenuation.m_innerRadius / distance;
                                attenuation = ratio * ratio;
                            }
                            break;
                        case Attenuation::TuAttenuation::CurveType::LogReverse:
                            attenuation = std::log((1.0f - normalizedDist) * 9.0f + 1.0f) / std::log(10.0f);
                            break;
                    }
                }
            }

            float x = graphPos.x + (t * graphWidth);
            float y = graphPos.y + graphHeight - (attenuation * graphHeight);

            ImVec2 point(x, y);
            if (i > 0)
            {
                drawList->AddLine(prevPoint, point, IM_COL32(100, 255, 150, 255), 2.0f);
            }
            prevPoint = point;
        }

        // Labels
        drawList->AddText(ImVec2(graphPos.x + 5, graphPos.y + 5),
            IM_COL32(200, 200, 200, 255), "1.0");
        drawList->AddText(ImVec2(graphPos.x + 5, graphPos.y + graphHeight - 20),
            IM_COL32(200, 200, 200, 255), "0.0");

        char distLabel[64];
        snprintf(distLabel, sizeof(distLabel), "%.0fm", m_node->m_attenuation.m_innerRadius);
        drawList->AddText(ImVec2(graphPos.x + 5, graphPos.y + graphHeight + 5),
            IM_COL32(200, 200, 200, 255), distLabel);

        snprintf(distLabel, sizeof(distLabel), "%.0fm",
            m_node->m_attenuation.m_innerRadius + m_node->m_attenuation.m_falloffDistance);
        drawList->AddText(ImVec2(graphPos.x + graphWidth - 40, graphPos.y + graphHeight + 5),
            IM_COL32(200, 200, 200, 255), distLabel);

        ImGui::Dummy(ImVec2(graphWidth, graphHeight + 25));
    }
    else if (m_node->m_distanceModel.type == IPL_DISTANCEATTENUATIONTYPE_INVERSEDISTANCE)
    {
        // Min Distance for inverse distance model
        float minDistance = m_node->m_distanceModel.minDistance;
        if (ImGui::DragFloat("Min Distance", &minDistance, 0.1f, 0.1f, 100.0f))
        {
            if (m_node)
            {
                m_node->setDistanceAttenuation(minDistance);
            }
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("No attenuation applied within this distance");
        }
    }
}
