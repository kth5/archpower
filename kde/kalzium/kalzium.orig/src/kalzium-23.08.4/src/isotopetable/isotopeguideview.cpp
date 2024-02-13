/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotopeguideview.h"

#include "isotopescene.h"
#include <QScrollBar>

IsotopeGuideView::IsotopeGuideView(QWidget *parent)
    : QGraphicsView(parent)
{
    m_guidedView = nullptr;
    m_scale = 1.0;

    setCursor(Qt::OpenHandCursor);
}

void IsotopeGuideView::setGuidedView(IsotopeView *guidedView)
{
    m_guidedView = guidedView;
    connect(m_guidedView, &IsotopeView::zoomLevelChanged, this, &IsotopeGuideView::setZoomLevel);
    connect(m_guidedView, &IsotopeView::visibleSceneRectChanged, this, &IsotopeGuideView::setVisibleSceneRect);

    updateScene();
}

void IsotopeGuideView::setVisibleSceneRect(const QPolygonF &sceneRect)
{
    m_visibleSceneRect = sceneRect;
    viewport()->update();
}

void IsotopeGuideView::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (m_guidedView && m_visibleSceneRect.boundingRect().intersects(rect)) {
        QRectF drawRect = m_visibleSceneRect.boundingRect().adjusted(0, 0, -1, -1);
        painter->setPen(QPen(Qt::yellow, 1.0 / m_scale));
        painter->fillRect(drawRect, QBrush(QColor(0, 0, 0, 100)));
        painter->drawRect(drawRect);
    }
}

void IsotopeGuideView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_scale = qMin(qreal(viewport()->width()) / sceneRect().width(), qreal(viewport()->height()) / sceneRect().height());
    setTransform(QTransform::fromScale(m_scale, m_scale));
}

void IsotopeGuideView::mousePressEvent(QMouseEvent *event)
{
    m_dragEvent = mapFromScene(m_visibleSceneRect).boundingRect().contains(event->pos());
    if (m_dragEvent && event->buttons() & Qt::LeftButton) {
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void IsotopeGuideView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_dragEvent = false;
    setCursor(Qt::OpenHandCursor);
}

void IsotopeGuideView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragEvent && event->buttons() & Qt::LeftButton) {
        const QPoint p1(m_guidedView->mapFromScene(mapToScene(m_lastMousePos)));
        const QPoint p2(m_guidedView->mapFromScene(mapToScene(event->pos())));
        m_guidedView->horizontalScrollBar()->setValue(m_guidedView->horizontalScrollBar()->value() + p2.x() - p1.x());
        m_guidedView->verticalScrollBar()->setValue(m_guidedView->verticalScrollBar()->value() + p2.y() - p1.y());
        m_lastMousePos = event->pos();

        m_visibleSceneRect = m_guidedView->visibleSceneRect();
        viewport()->update();
    }
}

void IsotopeGuideView::setZoomLevel(double zoomLevel)
{
    m_zoomLevel = zoomLevel;
}

void IsotopeGuideView::updateScene()
{
    m_zoomLevel = m_guidedView->zoomLevel();
    setScene(m_guidedView->scene());
    setSceneRect(scene()->itemsBoundingRect());
    ensureVisible(scene()->sceneRect());
    resizeEvent(nullptr);
}

#include "moc_isotopeguideview.cpp"
