/*
    PeriodicTableScene - Periodic Table Graphics Scene for Kalzium

    SPDX-FileCopyrightText: 2005-2006 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2003-2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERIODICTABLESCENE_H
#define PERIODICTABLESCENE_H

#include <QGraphicsScene>
#include <QPointF>
#include <QTimer>

#include "elementitem.h"

/**
 * @class PeriodicTableScene
 * @author Marcus D. Hanwell
 * @brief This class encapsulates the scene, all items are contained in it.
 *
 * This class implements a QGraphicsScene that holds all of the element items.
 * Any items owned by this class are automatically deleted by it.
 */
class PeriodicTableScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit PeriodicTableScene(QObject *parent = nullptr);
    ~PeriodicTableScene() override;

Q_SIGNALS:
    /**
     * This signal is emitted when an element item is released.
     */
    void elementChanged(int element);
    /**
     * This signal is emitted when an element item is hovered.
     */
    void elementHovered(int element);
    /**
     * This signal is emitted when no element was clicked.
     */
    void freeSpaceClick();

private Q_SLOTS:
    void slotMouseover();

private:
    QTimer m_hoverTimer;
    int m_prevHoverElement;
    QPointF m_eventPos;

protected:
    bool event(QEvent *e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // PERIODICTABLESCENE_H
