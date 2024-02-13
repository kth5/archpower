/*
    SPDX-FileCopyrightText: 2004, 2005, 2006, 2007 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DETAILEDGRAPHICALOVERVIEW_H
#define DETAILEDGRAPHICALOVERVIEW_H

#include <QWidget>

class Element;

/**
 * @brief The widget which displays the most important information
 *
 * In one widget like a lot people know it from school
 *
 * @author Carsten Niehaus
 */
class DetailedGraphicalOverview : public QWidget
{
    Q_OBJECT

public:
    /**
     * Construct a new DetailedGraphicalOverview.
     *
     * @param parent the parent of this widget
     */
    explicit DetailedGraphicalOverview(QWidget *parent);

public Q_SLOTS:
    /**
     * Set @p el as the element to be drawn
     */
    void setElement(int el);

private:
    /**
     * Set the background color to @p bgColor.
     */
    void setBackgroundColor(QColor bgColor);

    /**
     * the element whose data will be used
     */
    Element *m_element = nullptr;

    /**
     * The background color.
     */
    QBrush m_backgroundBrush;

protected:
    void paintEvent(QPaintEvent *) override;
};

#endif // DETAILEDGRAPHICALOVERVIEW_H
