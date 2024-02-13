/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISOTOPEVIEW_H
#define ISOTOPEVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>
#include <QWidget>

class IsotopeScene;

class IsotopeView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit IsotopeView(QWidget *parent = nullptr);
    explicit IsotopeView(QWidget *parent, int mode);
    ~IsotopeView() override;

private:
    void initialize();

private:
    IsotopeScene *m_scene;
    double m_zoomLevel;
    int m_mode;

public:
    double zoomLevel()
    {
        return m_zoomLevel;
    }
    QPolygonF visibleSceneRect() const
    {
        return mapToScene(viewport()->rect());
    }

Q_SIGNALS:
    void zoomLevelChanged(double zoomLevel);
    void visibleSceneRectChanged(const QPolygonF &sceneRect);

public Q_SLOTS:
    void setZoom(double zoom);
    void setMode(int mode);

public:
    int mode() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

#endif // ISOTOPEVIEW_H
