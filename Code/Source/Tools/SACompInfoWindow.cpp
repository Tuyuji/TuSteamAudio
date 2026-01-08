/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "SACompInfoWindow.h"

#include <QVBoxLayout>

#include "AzCore/Component/EntityBus.h"
#include "AzFramework/Components/CameraBus.h"
#include "Components/EditorSAPlayerComponent.h"

using namespace TuSteamAudio;

SACompInfoWindow::SACompInfoWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Steam Audio Component Info");
    setMinimumSize(100, 100);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);

    m_statusLabel = new QLabel("Select an entity with a Steam Audio component", this);
    m_statusLabel->setStyleSheet("QLabel { padding: 4px; color: palette(light); }");
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    m_graph = new AttenuationGraphWidget(this);
    mainLayout->addWidget(m_graph, 1);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16);
    m_updateTimer->setSingleShot(false);
    connect(m_updateTimer, &QTimer::timeout, this, &SACompInfoWindow::UpdateGraph);
    m_updateTimer->start();

    AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    AzToolsFramework::ToolsApplicationEvents::Bus::Handler::BusConnect();

    UpdateFromSelection();
}

SACompInfoWindow::~SACompInfoWindow()
{
    AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
    AzToolsFramework::ToolsApplicationEvents::Bus::Handler::BusDisconnect();
}

void SACompInfoWindow::NotifyRegisterViews()
{
}

void SACompInfoWindow::AfterEntitySelectionChanged(const AzToolsFramework::EntityIdList& entity_id_list,
                                                   const AzToolsFramework::EntityIdList& entity_ids)
{
    UpdateFromSelection();
}

void SACompInfoWindow::UpdateFromSelection()
{
    AzToolsFramework::EntityIdList selectedEntities;
    AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(selectedEntities, &AzToolsFramework::ToolsApplicationRequests::GetSelectedEntities);

    if (selectedEntities.empty())
    {
        ClearGraph();
        return;
    }

    AZ::EntityId foundEntityId = AZ::EntityId();
    AZ::ComponentId foundComponentId = AZ::InvalidComponentId;
    for (const auto& entityId : selectedEntities)
    {
        AZ::Entity* ptr;
        AZ::ComponentApplicationBus::BroadcastResult(ptr, &AZ::ComponentApplicationBus::Events::FindEntity, entityId);
        if (!ptr)
        {
            continue;
        }

        auto components = ptr->GetComponents();
        for (const auto component : components)
        {
            if (component->GetUnderlyingComponentType() == azrtti_typeid<EditorSAPlayerComponent>())
            {
                foundEntityId = entityId;
                foundComponentId = component->GetId();
                break;
            }
        }
        if (foundComponentId != AZ::InvalidComponentId)
        {
            break;
        }
    }

    if (foundEntityId.IsValid() && foundComponentId != AZ::InvalidComponentId)
    {
        m_currentEntityId = foundEntityId;
        m_currentComponentId = foundComponentId;

        AZStd::string entityName;
        AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationBus::Events::GetEntityName, foundEntityId);

        UpdateGraph();
    }else
    {
        ClearGraph();
    }
}

void SACompInfoWindow::UpdateGraph()
{
    if (!m_currentEntityId.IsValid() || m_currentComponentId == AZ::InvalidComponentId)
    {
        ClearGraph();
        return;
    }

    AZ::Entity* entity = nullptr;
    AZ::ComponentApplicationBus::BroadcastResult(entity, &AZ::ComponentApplicationBus::Events::FindEntity, m_currentEntityId);
    if (!entity)
    {
        ClearGraph();
        return;
    }

    EditorSAPlayerComponent* component = entity->FindComponent<EditorSAPlayerComponent>(m_currentComponentId);
    if (!component)
    {
        ClearGraph();
        return;
    }

    SAPlayerComponentConfig config;
    if (!component->GetConfiguration(config))
    {
        ClearGraph();
        return;
    }

    AZ::Transform cameraTrans = AZ::Transform::CreateIdentity();
    Camera::ActiveCameraRequestBus::BroadcastResult(cameraTrans, &Camera::ActiveCameraRequestBus::Events::GetActiveCameraTransform);
    float distance = (cameraTrans.GetTranslation() - entity->GetTransform()->GetWorldTranslation()).GetLength();
    m_graph->SetListenerDistance(distance);

    m_graph->UpdateAttenuation(config.m_attenuation);
    m_graph->setVisible(true);
    m_statusLabel->setVisible(false);
}

void SACompInfoWindow::ClearGraph()
{
    m_currentEntityId = AZ::EntityId();
    m_currentComponentId = AZ::InvalidComponentId;
    m_graph->setVisible(false);
    m_statusLabel->setVisible(true);
    m_statusLabel->setText("Select an entity with a Steam Audio component");
}

#include <moc_SACompInfoWindow.cpp>
