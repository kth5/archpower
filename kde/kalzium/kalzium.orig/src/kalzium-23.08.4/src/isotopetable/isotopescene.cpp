/*
    SPDX-FileCopyrightText: 2005-2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotopescene.h"

#include "isotopeitem.h"
#include "kalziumdataobject.h"

#include <element.h>
#include <isotope.h>

IsotopeScene::IsotopeScene(QObject *parent, int mode)
    : QGraphicsScene(parent)
{
    m_mode = mode;
    m_isotopeGroup = new QGraphicsItemGroup();
    m_isotopeGroup->setHandlesChildEvents(false);
    addItem(m_isotopeGroup);

    m_itemSize = 10;
    drawIsotopes();
}

IsotopeScene::~IsotopeScene()
{
    delete m_isotopeGroup;
}

void IsotopeScene::updateContextHelp(IsotopeItem *item)
{
    Q_EMIT itemSelected(item);
}

void IsotopeScene::drawIsotopes()
{
    for (auto item : m_isotopeGroup->childItems()) {
        m_isotopeGroup->removeFromGroup(item);
        delete item;
    }

    const QList<Element *> elist = KalziumDataObject::instance()->ElementList;

    for (Element *e : elist) {
        int elementNumber = e->dataAsVariant(ChemicalDataObject::atomicNumber).toInt();

        const QList<Isotope *> ilist = KalziumDataObject::instance()->isotopes(elementNumber);
        for (Isotope *i : ilist) {
            int x = elementNumber * m_itemSize;
            int y = (300 - i->nucleons()) * m_itemSize;

            if (m_mode == 0) {
                // One part to the side of the other
                int threshold = 60;
                if (elementNumber > threshold) {
                    y += 120 * m_itemSize;
                    x += 5 * m_itemSize;
                }
            } else if (m_mode == 1) {
                // Both parts continuous
            } else if (m_mode == 2) {
                // Horizontally
                y = (elist.count() - elementNumber) * m_itemSize;
                x = i->nucleons() * m_itemSize;
            } else if (m_mode == 3) {
                // Horizontally (shifted)
                y = (elist.count() - elementNumber) * m_itemSize;
                x = (i->nucleons() - elementNumber) * m_itemSize;
            }

            auto item = new IsotopeItem(i, x, y, m_itemSize, m_itemSize);
            m_isotopeGroup->addToGroup(item);
        }
    }
}

void IsotopeScene::setMode(int mode)
{
    m_mode = mode;
    drawIsotopes();
}

#include "moc_isotopescene.cpp"
