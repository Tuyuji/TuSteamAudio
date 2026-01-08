/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#include "AttenuationGraphWidget.h"

#include <QPainter>
#include <QPainterPath>

using namespace TuSteamAudio;

AttenuationGraphWidget::AttenuationGraphWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void AttenuationGraphWidget::UpdateAttenuation(const Attenuation::TuAttenuation& settings)
{
    //check if we should bother to update
    if (settings.m_innerRadius == m_attenuation.m_innerRadius &&
        settings.m_falloffDistance == m_attenuation.m_falloffDistance &&
        settings.m_curveType == m_attenuation.m_curveType)
    {
        //We dont care for the other settings
        return;
    }
    m_attenuation = settings;

    update();
}

void AttenuationGraphWidget::SetListenerDistance(float distance)
{
    if (distance == m_listenerDistance)
    {
        return;
    }
    m_listenerDistance = distance;
    update();
}

void AttenuationGraphWidget::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF graphRect(10, 10, width() - 20, height() - 40);

    p.fillRect(graphRect, QColor(25, 25, 30));
    p.setPen(QPen(QColor(100, 100, 100)));
    p.drawRect(graphRect);

    //grid lines
    p.setPen(QPen(QColor(50, 50, 50, 100), 1));
    for (int i = 1; i < 4; ++i)
    {
        float x = graphRect.left() + (graphRect.width() * i / 4.0f);
        p.drawLine(QPointF(x, graphRect.top()),
                       QPointF(x, graphRect.bottom()));

        float y = graphRect.top() + (graphRect.height() * i / 4.0f);
        p.drawLine(QPointF(graphRect.left(), y),
                       QPointF(graphRect.right(), y));
    }

    //draw curve
    QPainterPath curvePath;
    const int numPoints = 256;

    for (int i = 0; i <= numPoints; ++i)
    {
        float t = static_cast<float>(i) / numPoints;
        float distance = m_attenuation.m_innerRadius + (m_attenuation.m_falloffDistance * t);

        float attenuation = m_attenuation.CalculateAttenuation(distance);

        float x = graphRect.left() + (t * graphRect.width());
        float y = graphRect.bottom() - (attenuation * graphRect.height());

        if (i == 0)
        {
            curvePath.moveTo(x, y);
        }
        else
        {
            curvePath.lineTo(x, y);
        }
    }

    p.setPen(QPen(QColor(150, 255, 150), 2));
    p.drawPath(curvePath);

    if (m_listenerDistance > 0.0f)
    {
        const float distanceFromInner = m_listenerDistance - m_attenuation.m_innerRadius;

        //calculate X position on graph (normalised)
        float t = 0.0f;
        if (m_attenuation.m_falloffDistance > 0.0f)
        {
            t = distanceFromInner / m_attenuation.m_falloffDistance;
            t = qBound(0.0f, t, 1.0f);
        }

        const float lineX = graphRect.left() + (t * graphRect.width());

        const float currentAttenuation = m_attenuation.CalculateAttenuation(m_listenerDistance);
        const float lineY = graphRect.bottom() - (currentAttenuation * graphRect.height());

        p.setPen(QPen(QColor(255, 200, 50), 2, Qt::DashLine));
        p.drawLine(QPointF(lineX, graphRect.top()),
            QPointF(lineX, graphRect.bottom()));

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 200, 50));
        p.drawEllipse(QPointF(lineX, lineY), 3, 3);
    }

    p.setPen(Qt::white);
    QFont font = p.font();
    font.setPointSize(8);
    p.setFont(font);

    p.drawText(graphRect.topLeft() + QPointF(5, 15), "1.0");
    p.drawText(graphRect.bottomLeft() + QPointF(5, -5), "0.0");

    QString innerLabel = QString("%1m").arg(m_attenuation.m_innerRadius, 0, 'f', 0);
    p.drawText(graphRect.bottomLeft() + QPointF(5, 20), innerLabel);

    QString outerLabel = QString("%1m").arg(
        m_attenuation.m_innerRadius + m_attenuation.m_falloffDistance, 0, 'f', 0);
    p.drawText(graphRect.bottomRight() + QPointF(-50, 20), outerLabel);
}

#include <moc_AttenuationGraphWidget.cpp>