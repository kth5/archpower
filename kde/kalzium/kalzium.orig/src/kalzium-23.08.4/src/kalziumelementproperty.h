/*
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMELEMENTPROPERTY_H
#define KALZIUMELEMENTPROPERTY_H

#include <QObject>
#include <QBrush>

#include "kalziumgradienttype.h"
#include "kalziumschemetype.h"

/**
 * The logic of the scheme and the gradients is merged in this class. It provides
 * an API to set the gradient and scheme without worrying about the real id's.
 * Elements can then get their properties with the element functions.
 * Here is also the place where the gradientslider is evaluated. (see gradientwidget)
 * @short This class holds the logic of the appearance from the periodic table
 * @author Etienne Rebetez
 */

class KalziumElementProperty : public QObject
{
    Q_OBJECT

public:
    /**
     * Get the instance of this factory.
     */
    static KalziumElementProperty *instance();

    enum ELEMENTTEXTINFORMATIONMODE {
        NORMAL = 0, // Only Symbol Number
        ELNUMBER,
        GRADIENTVALUE
    };

    enum SPECIALGRADIENTYPDEF { NOGRADIENT = 0, SOMGradientType = 1, DISCOVERYDATE = 9 };

    /**
     * all available schemes. Can be used to populate menus.
     */
    QStringList schemeList() const;
    /**
     * all available gradients. Can be used to populate menus.
     */
    QStringList gradientList() const;

    /**
     * Returns the current scheme id as int. Equivalent to the scheme list.
     */
    int schemeId() const;
    /**
     * Returns the current gradient id as int. Equivalent to the gradient list.
     */
    int gradientId() const;

    /**
     * Returns the current real KalziumSchemeType class
     */
    KalziumSchemeType *scheme() const;
    /**
     * Returns the current real KalziumGradientType class
     */
    KalziumGradientType *gradient() const;

    /**
     * This is an element function.
     * It returns the Brush (Background) for the given element.
     * It uses the color form getElementColor.
     * @param el Number of the element.
     */
    QBrush getElementBrush(int el);
    /**
     * This is an element function.
     * It returns the Color (Background) for the given element.
     * @param el Number of the element.
     */
    QColor getElementColor(int el);
    /**
     * This is an element function.
     * It returns the Text color for the given element.
     * @param el Number of the element.
     */
    QColor getTextColor(int el) const;
    /**
     * This is an element function.
     * It returns the border color of the for the given element.
     * @param el Number of the element.
     */
    QColor getBorderColor(int el) const;

    /**
     * This is an element function.
     * It returns the value from the current gradient type for the given element.
     * @param el Number of the element.
     */
    double getValue(int el) const;

    /**
     * This is an element function.
     * The mode returns the enum ELEMENTTEXTINFORMATIONMODE.
     * The mode is used to differ different layouts in the elementitem.
     */
    int getMode() const;

Q_SIGNALS:
    /**
     * Is emitted every time a property (scheme, gradient, ...) is changed.
     * all elements will be redrawn.
     */
    void propertyChanged();

public Q_SLOTS:
    /**
     * gets the value from the gradientwidget. The value which is used to
     * determine if an element is active or not.
     * @param slide value of the current gradient
     */
    void setSliderValue(double slide);
    /**
     * sets the new scheme
     */
    void setScheme(int newScheme);
    /**
     * sets the new gradient
     */
    void setGradient(int newGradient);

private:
    KalziumElementProperty();
    ~KalziumElementProperty() override;

    bool isGradient();

    QColor gradientBrushLogic(int el) const;

    int m_currentScheme;
    int m_currentGradient;

    double m_slider;
    int m_mode;
};

#endif // KALZIUMELEMENTPROPERTY_H
