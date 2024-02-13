/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QLabel>

#include "kalziumelementproperty.h"

class LegendItem;

/**
 * @author Carsten Niehaus
 *
 * The LegendWidget displays the explanations of what the user is currently
 * seeing in the table
 */
class LegendWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LegendWidget(QWidget *parent);

    ~LegendWidget() override;

    void LockWidget();
    void UnLockWidget();

Q_SIGNALS:
    void elementMatched(int element);
    void resetElementMatch();

public Q_SLOTS:
    void updateContent();

    void setDockArea(Qt::DockWidgetArea newDockArea);

    /// is triggered when a LegenItem is Hoovered.
    void legendItemAction(QColor color);

private:
    bool isElementMatch(int element, QColor &color);

    bool m_update;

    QPixmap m_pixmap;

    QList<LegendItem *> m_legendItemList;

    Qt::DockWidgetArea m_dockArea;

    void updateLegendItemLayout(const QList<legendPair> &list);
};

/**
 * A LegendItem is displayed as one small rectangle which represents the
 * color in the table with a short explanation for it.
 *
 * @author Carsten Niehaus
 */
class LegendItem : public QLabel
{
    Q_OBJECT

public:
    LegendItem(const QPair<QString, QColor> &pair, LegendWidget *parent = nullptr);
    ~LegendItem() override = default;

Q_SIGNALS:
    void legenItemHoovered(QColor color);

private:
    QColor legendItemColor;

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
};

#endif // LEGENDWIDGET_H
