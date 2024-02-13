/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotopeview.h"

#include "isotopescene.h"
#include "prefs.h"

IsotopeView::IsotopeView(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new IsotopeScene(this, Prefs::isotopeTableMode());
    m_zoomLevel = 1.0;
    initialize();
}

IsotopeView::IsotopeView(QWidget *parent, int mode)
    : QGraphicsView(parent)
{
    m_scene = new IsotopeScene(this, mode);
    m_zoomLevel = 1.0;
    initialize();
}

void IsotopeView::initialize()
{
    setScene(m_scene);
    setSceneRect(m_scene->itemsBoundingRect());
    // Zoom in a bit
    setZoom(0.3);
    // Makes sure that you always zoom to the mouse if you use the scroll wheel
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    QPolygonF visibleSceneRect = mapToScene(viewport()->rect());
    Q_EMIT visibleSceneRectChanged(visibleSceneRect);
}

IsotopeView::~IsotopeView()
{
    delete scene();
}

void IsotopeView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    if (!isInteractive()) {
        qreal scale = qMin(qreal(viewport()->width()) / sceneRect().width(), qreal(viewport()->height()) / sceneRect().height());
        setTransform(QTransform::fromScale(scale, scale));
    }
}

void IsotopeView::mouseMoveEvent(QMouseEvent *event)
{
    QPolygonF visibleSceneRect = mapToScene(viewport()->rect());
    Q_EMIT visibleSceneRectChanged(visibleSceneRect);

    QGraphicsView::mouseMoveEvent(event);
}

void IsotopeView::wheelEvent(QWheelEvent *event)
{
    if (!isInteractive()) {
        event->accept();
        return;
    }

    double oldZoomLevel = m_zoomLevel;
    double factor = event->angleDelta().y() / 1000.0;
    m_zoomLevel = oldZoomLevel + oldZoomLevel * factor;

    if (m_zoomLevel < 0.3) {
        m_zoomLevel = 0.3;
    } else if (m_zoomLevel > 10.0) {
        m_zoomLevel = 10.0;
    }

    if (oldZoomLevel != m_zoomLevel) {
        factor = m_zoomLevel / oldZoomLevel;
        scale(factor, factor);
        Q_EMIT zoomLevelChanged(m_zoomLevel);
    }
    QPolygonF visibleSceneRect = mapToScene(viewport()->rect());
    Q_EMIT visibleSceneRectChanged(visibleSceneRect);

    event->accept();
}

void IsotopeView::setZoom(double zoom)
{
    double oldZoomLevel = m_zoomLevel;
    double factor;

    m_zoomLevel = zoom;

    if (m_zoomLevel < 0.3 && isInteractive()) {
        m_zoomLevel = 0.3;
    } else if (m_zoomLevel > 10.0 && isInteractive()) {
        m_zoomLevel = 10.0;
    }

    factor = m_zoomLevel / oldZoomLevel;
    scale(factor, factor);
    Q_EMIT zoomLevelChanged(m_zoomLevel);
    QPolygonF visibleSceneRect = mapToScene(viewport()->rect());
    Q_EMIT visibleSceneRectChanged(visibleSceneRect);
}

int IsotopeView::mode() const
{
    return m_scene->mode();
}

void IsotopeView::setMode(int mode)
{
    m_scene->setMode(mode);
    initialize();
}

#include "moc_isotopeview.cpp"
