/*
    SPDX-FileCopyrightText: 2007, 2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISOTOPESCENE_H
#define ISOTOPESCENE_H

#include <QGraphicsScene>

class IsotopeItem;

class IsotopeScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit IsotopeScene(QObject *parent, int mode);
    ~IsotopeScene() override;

    void updateContextHelp(IsotopeItem *item);

private:
    void initialize();

private:
    void drawIsotopes();

    // the size of each item
    int m_itemSize;
    int m_mode;

    /// this group stores all IsotopeItems
    QGraphicsItemGroup *m_isotopeGroup;

public:
    int mode() const
    {
        return m_mode;
    }

Q_SIGNALS:
    void itemSelected(IsotopeItem *item);

public:
    void setMode(int mode);
};

#endif // ISOTOPESCENE_H
