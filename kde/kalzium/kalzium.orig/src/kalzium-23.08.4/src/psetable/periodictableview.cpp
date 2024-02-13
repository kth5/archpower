/*
    SPDX-FileCopyrightText: 2005-2006 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2003-2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>


    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "periodictableview.h"

#include <QTimer>
#include <QSvgGenerator>

#include "psetables.h"
#include <prefs.h>

PeriodicTableView::PeriodicTableView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);

    setMouseTracking(true);

    m_currentTableInex = Prefs::table();

    m_tableScene = new PeriodicTableScene(this);
    setScene(m_tableScene);
    connect(m_tableScene, &PeriodicTableScene::freeSpaceClick, this, &PeriodicTableView::fitPseInView);

    m_tableStates = new PeriodicTableStates(createElementItems(), createNumerationItems());

    fitPseInView();
}

QList<NumerationItem *> PeriodicTableView::createNumerationItems() const
{
    // Creating Nummerationitems here, we use the classic periodic table as reference (18 elements in a row)
    const int xMax = 18;

    QList<NumerationItem *> numerationItemList;

    for (int i = 0; i < xMax; ++i) {
        numerationItemList << new NumerationItem(i);
        m_tableScene->addItem(numerationItemList.at(i));
        connect(this, &PeriodicTableView::numerationChange, numerationItemList.at(i), &NumerationItem::setNumerationType);
    }

    return numerationItemList;
}

QList<ElementItem *> PeriodicTableView::createElementItems() const
{
    QList<ElementItem *> elementItemList;
    KalziumElementProperty *elProperty = KalziumElementProperty::instance();

    foreach (int intElement, pseTables::instance()->getTabletype(0)->elements()) {
        auto item = new ElementItem(elProperty, intElement);

        connect(elProperty, &KalziumElementProperty::propertyChanged, item, &ElementItem::redraw);
        m_tableScene->addItem(item);
        elementItemList << item;
    }

    return elementItemList;
}

PeriodicTableScene *PeriodicTableView::pseScene() const
{
    return m_tableScene;
}

int PeriodicTableView::table() const
{
    return m_currentTableInex;
}

void PeriodicTableView::slotChangeTable(int table)
{
    m_currentTableInex = table;
    Prefs::setTable(m_currentTableInex);

    setBiggerSceneRect();
    m_tableStates->setTableState(m_currentTableInex);

    QTimer::singleShot(RESIZE_SCENE_TIMEOUT, this, &PeriodicTableView::fitPseInView);
}

void PeriodicTableView::slotSelectOneElement(int element)
{
    slotUnSelectElements();
    slotSelectAdditionalElement(element);
}

void PeriodicTableView::slotSelectAdditionalElement(int element)
{
    if (element > 0) {
        m_tableScene->items().at(--element)->setSelected(true);
    }
}

void PeriodicTableView::slotUnSelectElements()
{
    foreach (QGraphicsItem *item, m_tableScene->selectedItems()) {
        item->setSelected(false);
    }
}

void PeriodicTableView::setBiggerSceneRect()
{
    QRectF newRect(sceneRect());
    QRectF pseRect(m_tableStates->pseRect(m_currentTableInex));

    if (sceneRect().width() < pseRect.width()) {
        newRect.setWidth(pseRect.width());
    }

    if (sceneRect().height() < pseRect.height()) {
        newRect.setHeight(pseRect.height());
    }

    setSceneRect(newRect);
}

void PeriodicTableView::resizeEvent(QResizeEvent *event)
{
    fitPseInView();
    QGraphicsView::resizeEvent(event);
}

void PeriodicTableView::fitPseInView()
{
    if (operator!=(sceneRect(), m_tableStates->pseRect(m_currentTableInex))) {
        setSceneRect(m_tableStates->pseRect(m_currentTableInex));
    }

    fitInView(sceneRect(), Qt::KeepAspectRatio);
}

bool PeriodicTableView::event(QEvent *e)
{
    return QGraphicsView::event(e);
}

void PeriodicTableView::generateSvg(const QString &filename)
{
    auto svgGen = new QSvgGenerator();
    svgGen->setFileName(filename);

    QPainter painter;
    painter.begin(svgGen);
    painter.rotate(180);
    render(&painter);
    painter.end();

    delete svgGen;
}

PeriodicTableView::~PeriodicTableView()
{
    delete scene();
    delete m_tableStates;
}

#include "moc_periodictableview.cpp"
