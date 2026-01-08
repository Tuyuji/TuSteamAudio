/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "EditorSAPlayerComponent.h"

using namespace TuSteamAudio;

void EditorSAPlayerComponent::Reflect(AZ::ReflectContext* context)
{
    Super::Reflect(context);
    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<EditorSAPlayerComponent, Super>()
            ->Version(0);

        using namespace AZ::Edit;
        auto ec = sc->GetEditContext();
        if (!ec)
            return;

        ec->Class<EditorSAPlayerComponent>("Steam Audio Effect Component", "Adds Steam Audio processing to a TuLabSound Player")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
            ->Attribute(AZ::Edit::Attributes::Category, "TuSteamAudio")
            ->Attribute(Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
            ->Attribute(Attributes::AutoExpand, true)
            ->Attribute(Attributes::ChangeNotify, &EditorSAPlayerComponent::OnConfigurationChanged)
        ;

        ec->Class<SAPlayerComponentController>("Steam Audio Effect Controller", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
            ->Attribute(Attributes::AutoExpand, true)
            ->DataElement(Attributes::Visibility, &SAPlayerComponentController::m_config, "Configuration", "The configuration for the Steam Audio effect component.")
            ->Attribute(Attributes::Visibility, PropertyVisibility::ShowChildrenOnly);

        ec->Class<Attenuation::TuAttenuation>("TuAttenuation", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
            ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
            ->DataElement(UIHandlers::ComboBox, &Attenuation::TuAttenuation::m_shape, "Shape", "The shape of the attenuation curve.")
                ->EnumAttribute(Attenuation::Shape::Sphere, "Sphere")
            ->DataElement(UIHandlers::ComboBox, &Attenuation::TuAttenuation::m_curveType, "Curve Type", "The type of curve to use.")
                ->EnumAttribute(Attenuation::CurveType::Linear, "Linear")
                ->EnumAttribute(Attenuation::CurveType::Logarithmic, "Logarithmic")
                ->EnumAttribute(Attenuation::CurveType::Inverse, "Inverse")
                ->EnumAttribute(Attenuation::CurveType::LogReverse, "Log Reverse")
                ->EnumAttribute(Attenuation::CurveType::NaturalSound, "Natural Sound")
            ->DataElement(UIHandlers::Default, &Attenuation::TuAttenuation::m_innerRadius, "Inner Radius", "The inner radius of the attenuation curve.")
            ->DataElement(UIHandlers::Default, &Attenuation::TuAttenuation::m_falloffDistance, "Falloff Distance", "The falloff distance of the attenuation curve.")
            ->DataElement(UIHandlers::Default, &Attenuation::TuAttenuation::m_attenuationCurveExponent, "Attenuation Curve Exponent", "The exponent of the attenuation curve.")
            ;

        ec->Class<SAPlayerComponentConfig>("Steam Audio Effect Component Config", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
            ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
            ->Attribute(Attributes::Visibility, PropertyVisibility::Hide)
            ->DataElement(UIHandlers::ComboBox, &SAPlayerComponentConfig::m_distanceModel, "Distance Model", "The distance model to use.")
                ->EnumAttribute(DistanceModel::Default, "Default")
                ->EnumAttribute(DistanceModel::InverseDistance, "Inverse Distance")
                ->EnumAttribute(DistanceModel::TuAttenuation, "TuAttenuation")
            ->DataElement(UIHandlers::Default, &SAPlayerComponentConfig::m_attenuation, "Attenuation", "The attenuation settings to use")
        ;
    }
}

EditorSAPlayerComponent::EditorSAPlayerComponent(const SAPlayerComponentConfig& config)
    : Super(config)
{
}

void EditorSAPlayerComponent::Activate()
{
    Super::Activate();
    AzFramework::EntityDebugDisplayEventBus::Handler::BusConnect(GetEntityId());
}

void EditorSAPlayerComponent::Deactivate()
{
    AzFramework::EntityDebugDisplayEventBus::Handler::BusDisconnect();
    Super::Deactivate();
}

AZ::u32 EditorSAPlayerComponent::OnConfigurationChanged()
{
    m_controller.OnConfigurationUpdated();
    return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
}

void EditorSAPlayerComponent::DisplayEntityViewport(const AzFramework::ViewportInfo &viewport_info,
    AzFramework::DebugDisplayRequests &dbg)
{
    if (!IsSelected())
    {
        return;
    }

    AZ::Transform transform = {};
    AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

    dbg.PushMatrix(transform);

    if (m_controller.m_config.m_attenuation.m_shape == Attenuation::Shape::Sphere) {
        auto attenuation = m_controller.m_config.m_attenuation;

        auto innerRadius = attenuation.m_innerRadius;
        auto outerRadius = attenuation.m_falloffDistance;

        dbg.SetColor(AZ::Colors::Blue);
        dbg.DrawWireSphere({}, innerRadius);

        dbg.SetColor(AZ::Colors::LightBlue);
        dbg.DrawWireSphere({}, outerRadius);

        float dist = 0.0f;
        float lastDist = 0.0f;
        AZ::Vector4 lastColor = AZ::Colors::Green.GetAsVector4();

        const AZ::Vector4 positiveColor = AZ::Colors::Green.GetAsVector4();
        const AZ::Vector4 noAudioColor = AZ::Colors::Red.GetAsVector4();
        while (dist <= outerRadius)
        {
            dist += 0.5f;
            float attenuationValue = attenuation.CalculateAttenuation(dist);
            AZ::Vector4 color = AZ::Lerp(noAudioColor, positiveColor, attenuationValue);
            dbg.DrawLine({lastDist, 0, 0}, {dist, 0, 0}, lastColor, color);
            lastDist = dist;
            lastColor = color;
        }
    }

    dbg.PopMatrix();
}

AZ::u32 EditorSAPlayerComponent::OnDistanceModelChanged()
{
    return OnConfigurationChanged();
}

AZ::u32 EditorSAPlayerComponent::OnAttenuationSettingsChanged()
{
    return OnConfigurationChanged();
}