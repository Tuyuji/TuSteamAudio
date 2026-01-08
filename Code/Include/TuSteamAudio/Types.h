/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/RTTI/TypeInfo.h"

namespace TuSteamAudio
{
    enum class DistanceModel
    {
        Default,
        InverseDistance,
        TuAttenuation
    };

    namespace Attenuation
    {
        enum class Shape
        {
            Sphere
        };

        enum class CurveType
        {
            Linear,
            Logarithmic,
            Inverse,
            LogReverse,
            NaturalSound
        };

        class TuAttenuation final
        {
            AZ_RTTI(TuAttenuation, "{17FE545C-8BD3-4759-BB6A-81D7D8465276}");
        public:
            static void Reflect(AZ::ReflectContext* context);
            Shape m_shape = Shape::Sphere;

            float m_innerRadius = 1.0f;      // no attenuation within 1 meter
            float m_falloffDistance = 100.0f; // Falls off over 100 meters

            enum class CurveType {Linear, Logarithmic, Inverse, LogReverse, NaturalSound} m_curveType = CurveType::Linear;

            float m_attenuationCurveExponent = 1.0f;

            float CalculateAttenuation(float distance);
        };
    }
}

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(TuSteamAudio::DistanceModel, "{8394C253-DD9B-4EC3-86AB-E5DE589764B7}");
    AZ_TYPE_INFO_SPECIALIZE(TuSteamAudio::Attenuation::Shape, "{7F2AB445-1F17-458E-8F15-848F8E1879A0}");
    AZ_TYPE_INFO_SPECIALIZE(TuSteamAudio::Attenuation::CurveType, "{2A2EC65D-9918-42C8-8825-ED2088709941}");
}
