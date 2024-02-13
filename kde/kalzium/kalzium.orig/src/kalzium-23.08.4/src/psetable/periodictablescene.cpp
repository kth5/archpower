/*
    PeriodicTableScene - Periodic Table Graphics Scene for Kalzium

    SPDX-FileCopyrightText: 2005-2006 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2003-2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "periodictablescene.h"

#include <QApplication>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

PeriodicTableScene::PeriodicTableScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_prevHoverElement(-1)
{
    QPalette widgetPalette = palette();
    setBackgroundBrush(QBrush(widgetPalette.window()));

    setItemIndexMethod(QGraphicsScene::NoIndex);

    m_hoverTimer.setSingleShot(true);
    connect(&m_hoverTimer, &QTimer::timeout, this, &PeriodicTableScene::slotMouseover);
}

PeriodicTableScene::~PeriodicTableScene() = default;

bool PeriodicTableScene::event(QEvent *e)
{
    return QGraphicsScene::event(e);
}

void PeriodicTableScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    QGraphicsItem *item = QGraphicsScene::itemAt(event->scenePos(), QTransform());
    if (item->data(0).toInt() > 0 && item->data(0).toInt() < 119) {
        m_eventPos = event->scenePos();
    } else {
        Q_EMIT freeSpaceClick();
    }

    QGraphicsScene::mousePressEvent(event);
}

void PeriodicTableScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem *item = QGraphicsScene::itemAt(m_eventPos, QTransform());

    if ((QApplication::mouseButtons() & Qt::LeftButton) && (event->pos() - m_eventPos).manhattanLength() > QApplication::startDragDistance()
        && item->data(0).toInt() > 0) {
        Element *pointedElement = KalziumDataObject::instance()->element(item->data(0).toInt());

        auto drag = new QDrag(event->widget());
        auto mimeData = new QMimeData;

        mimeData->setText(pointedElement->dataAsString(ChemicalDataObject::name));
        drag->setMimeData(mimeData);

        QPixmap pix(item->boundingRect().width() + 1, item->boundingRect().height() + 1);
        pix.fill(palette().color(QPalette::Window));

        QPainter painter(&pix);
        item->paint(&painter, new QStyleOptionGraphicsItem());

        drag->setPixmap(pix);
        drag->exec(Qt::CopyAction | Qt::MoveAction);

        m_eventPos = QPoint();
    } else {
        m_eventPos = event->scenePos();

        if (m_hoverTimer.isActive()) {
            m_hoverTimer.stop();
        }
        m_hoverTimer.start(100);
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void PeriodicTableScene::slotMouseover()
{
    QGraphicsItem *item = QGraphicsScene::itemAt(m_eventPos, QTransform());

    if (item->data(0).toInt() > 0 && item->data(0).toInt() < 119) {
        int num = item->data(0).toInt();
        if ((num > 0) && (num != m_prevHoverElement)) {
            Q_EMIT elementHovered(num);
        }
        m_prevHoverElement = num;
    }
}

void PeriodicTableScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    QGraphicsItem *item = QGraphicsScene::itemAt(event->scenePos(), QTransform());
    if (item->data(0).toInt() > 0 && item->data(0).toInt() < 119) {
        Q_EMIT(elementChanged(item->data(0).toInt()));
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

#include "moc_periodictablescene.cpp"
