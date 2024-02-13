/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISOTOPEGUIDEVIEW_H
#define ISOTOPEGUIDEVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QResizeEvent>

#include "isotopeview.h"

class IsotopeGuideView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit IsotopeGuideView(QWidget *parent = nullptr);
    void setGuidedView(IsotopeView *guidedView);

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    IsotopeView *m_guidedView = nullptr;
    double m_zoomLevel;
    double m_scale;
    QPolygonF m_visibleSceneRect;
    QPoint m_lastMousePos;
    bool m_dragEvent;

    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public Q_SLOTS:
    void updateScene();

private Q_SLOTS:
    void setZoomLevel(double zoomLevel);
    void setVisibleSceneRect(const QPolygonF &sceneRect);
};

#endif // ISOTOPEGUIDEVIEW_H
