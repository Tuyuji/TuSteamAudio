/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#if !defined(Q_MOC_RUN)
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include "TuSteamAudio/Types.h"
#include "Tools/AttenuationGraphWidget.h"
#endif

namespace TuSteamAudio
{
    class SACompInfoWindow
        : public QWidget
        , private AzToolsFramework::EditorEvents::Bus::Handler
        , private AzToolsFramework::ToolsApplicationEvents::Bus::Handler
    {
        Q_OBJECT
    public:
        explicit SACompInfoWindow(QWidget* parent = nullptr);
        ~SACompInfoWindow() override;
    private:
        void NotifyRegisterViews() override;

        void AfterEntitySelectionChanged(const AzToolsFramework::EntityIdList&, const AzToolsFramework::EntityIdList&) override;

        void UpdateFromSelection();
        void UpdateGraph();
        void ClearGraph();

        AttenuationGraphWidget* m_graph = nullptr;
        QLabel* m_statusLabel = nullptr;
        QTimer* m_updateTimer = nullptr;

        AZ::EntityId m_currentEntityId = AZ::EntityId();
        AZ::ComponentId m_currentComponentId = AZ::ComponentId();
    };
}