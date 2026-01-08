/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include <TuSteamAudio/Types.h>
#include <AzCore/Serialization/SerializeContext.h>

using namespace TuSteamAudio;

void Attenuation::TuAttenuation::Reflect(AZ::ReflectContext* context)
{
    auto sc = azrtti_cast<AZ::SerializeContext*>(context);
    if (!sc)
        return;

    sc->Enum<DistanceModel>()
        ->Value("Default", DistanceModel::Default)
        ->Value("InverseDistance", DistanceModel::InverseDistance)
        ->Value("TuAttenuation", DistanceModel::TuAttenuation);

    sc->Enum<Attenuation::Shape>()
        ->Value("Linear", Attenuation::Shape::Sphere);

    sc->Enum<Attenuation::CurveType>()
        ->Value("Linear", Attenuation::CurveType::Linear)
        ->Value("Logarithmic", Attenuation::CurveType::Logarithmic)
        ->Value("Inverse", Attenuation::CurveType::Inverse)
        ->Value("LogReverse", Attenuation::CurveType::LogReverse)
        ->Value("NaturalSound", Attenuation::CurveType::NaturalSound);

    sc->Class<Attenuation::TuAttenuation>()
        ->Version(0)
        ->Field("shape", &Attenuation::TuAttenuation::m_shape)
        ->Field("innerRadius", &Attenuation::TuAttenuation::m_innerRadius)
        ->Field("falloffDistance", &Attenuation::TuAttenuation::m_falloffDistance)
        ->Field("curveType", &Attenuation::TuAttenuation::m_curveType)
        ->Field("attenuationCurveExponent", &Attenuation::TuAttenuation::m_attenuationCurveExponent);
}

float Attenuation::TuAttenuation::CalculateAttenuation(float distance) {

    if (distance <= m_innerRadius)
    {
        return 1.0f;
    }

    float effectiveDistance = distance - m_innerRadius;
    if (effectiveDistance >= m_falloffDistance)
    {
        return 0.0f;
    }

    float normalizedDistance = effectiveDistance / m_falloffDistance;

    float attenuation = 1.0f;
    switch (m_curveType)
    {
        case Attenuation::TuAttenuation::CurveType::Linear:
            attenuation = 1.0f - normalizedDistance;
            break;

        case Attenuation::TuAttenuation::CurveType::Logarithmic:
            // Log base 10: log10(x) = log(x) / log(10)
            attenuation = 1.0f - (std::log(normalizedDistance * 9.0f + 1.0f) / std::log(10.0f));
            break;

        case Attenuation::TuAttenuation::CurveType::Inverse:
            // Inverse distance: 1/d falloff
            attenuation = m_innerRadius / distance;
            break;

        case Attenuation::TuAttenuation::CurveType::NaturalSound:
            // Inverse square law: 1/d² (physically accurate for sound)
        {
            float ratio = m_innerRadius / distance;
            attenuation = ratio * ratio;
        }
            break;

        case Attenuation::TuAttenuation::CurveType::LogReverse:
            // Reverse logarithmic curve
            attenuation = std::log((1.0f - normalizedDistance) * 9.0f + 1.0f) / std::log(10.0f);
            break;
    }

    return AZ::GetClamp(attenuation, 0.0f, 1.0f);
}