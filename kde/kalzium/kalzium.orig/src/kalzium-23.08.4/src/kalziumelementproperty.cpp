/*
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalziumelementproperty.h"

#include "kalziumdataobject.h"
#include "search.h"

#include <KLocalizedString>

#include <prefs.h>

KalziumElementProperty *KalziumElementProperty::instance()
{
    static KalziumElementProperty elementProperty;
    return &elementProperty;
}

KalziumElementProperty::KalziumElementProperty()
    : m_mode(0)
{
    m_currentScheme = Prefs::colorschemebox();

    if (schemeList().count() <= m_currentScheme)
        m_currentScheme = 0;

    m_currentGradient = Prefs::colorgradientbox();

    if (isGradient())
        m_mode = GRADIENTVALUE;
}

KalziumElementProperty::~KalziumElementProperty() = default;

bool KalziumElementProperty::isGradient()
{
    return m_currentGradient > 1;
}

void KalziumElementProperty::setScheme(int newScheme)
{
    m_currentScheme = newScheme;
    Prefs::setColorschemebox(newScheme);
    Prefs::self()->save();
    propertyChanged();
}

void KalziumElementProperty::setGradient(int newGradient)
{
    m_currentGradient = newGradient;
    Prefs::setColorgradientbox(newGradient);
    Prefs::self()->save();

    if (isGradient()) {
        m_mode = GRADIENTVALUE;
    } else {
        m_mode = NORMAL;
    }

    propertyChanged();
}

QStringList KalziumElementProperty::schemeList() const
{
    return KalziumSchemeTypeFactory::instance()->schemes();
}

QStringList KalziumElementProperty::gradientList() const
{
    QStringList customList;
    customList << i18nc("No Gradient", "None");
    customList << KalziumGradientTypeFactory::instance()->gradients();
    return customList;
}

KalziumSchemeType *KalziumElementProperty::scheme() const
{
    return KalziumSchemeTypeFactory::instance()->build(m_currentScheme);
}

KalziumGradientType *KalziumElementProperty::gradient() const
{
    if (m_currentGradient == NOGRADIENT) {
        return KalziumGradientTypeFactory::instance()->build(NOGRADIENT);
    }
    return KalziumGradientTypeFactory::instance()->build(m_currentGradient - 1);
}

int KalziumElementProperty::gradientId() const
{
    return m_currentGradient;
}

int KalziumElementProperty::schemeId() const
{
    return m_currentScheme;
}

void KalziumElementProperty::setSliderValue(double slide)
{
    m_slider = slide;
    propertyChanged();
}

double KalziumElementProperty::getValue(int el) const
{
    if (m_currentGradient != NOGRADIENT)
        return gradient()->value(el);

    return 0;
}

QColor KalziumElementProperty::getElementColor(int el)
{
    if (m_currentGradient == NOGRADIENT)
        return scheme()->elementBrush(el).color();

    return gradientBrushLogic(el);
}

QBrush KalziumElementProperty::getElementBrush(int el)
{
    QBrush elementBrush;
    elementBrush.setStyle(Qt::SolidPattern);
    elementBrush.setColor(Qt::transparent);

    // Hide filtered elements from search
    if (!KalziumDataObject::instance()->search()->matches(el) && KalziumDataObject::instance()->search()->isActive()) {
        return {Qt::darkGray, Qt::Dense7Pattern};
    }

    // The iconic view is the 3rd view (0,1,2,...). Pixmaps don't make nice gradients.
    if (m_currentScheme == 2) {
        elementBrush = scheme()->elementBrush(el);
    } else {
        // add a nice gradient
        QColor color = getElementColor(el);
        QLinearGradient grad(QPointF(0, 0), QPointF(0, 40));
        grad.setColorAt(0, color);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        qreal h, s, v, a;
#else
        float h, s, v, a;
#endif
        color.getHsvF(&h, &s, &v, &a);
        color.setHsvF(h, s, v * 0.7, a);
        grad.setColorAt(1, color);

        elementBrush = QBrush(grad);
    }

    return elementBrush;
}

QColor KalziumElementProperty::getTextColor(int el) const
{
    return scheme()->textColor(el);
}

QColor KalziumElementProperty::getBorderColor(int el) const
{
    // Show scheme color as border when gradients are selected.
    if (m_currentGradient != NOGRADIENT) {
        return scheme()->elementBrush(el).color();
    }

    // Transparent Borders don't make sens.
    if (getTextColor(el) == QColor(Qt::transparent)) {
        return {Qt::black};
    }

    return getTextColor(el);
}

int KalziumElementProperty::getMode() const
{
    return m_mode;
}

QColor KalziumElementProperty::gradientBrushLogic(int el) const
{
    QColor gradientColor;
    bool isActiv = true;
    const double gradientValue = gradient()->value(el);
    const double melting = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::meltingpoint).toDouble();
    const double boiling = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::boilingpoint).toDouble();

    switch (m_currentGradient) {
    case SOMGradientType:
        if (m_slider < melting) {
            // the element is solid
            gradientColor = Prefs::color_solid();
        } else if ((m_slider > melting) && (m_slider < boiling)) {
            // the element is liquid
            gradientColor = Prefs::color_liquid();
        } else if ((m_slider >= boiling) && (boiling > 0.0)) {
            // the element is vaporous
            gradientColor = Prefs::color_vapor();
        } else {
            gradientColor = Qt::lightGray;
        }
        return gradientColor;

    case DISCOVERYDATE:
        if (gradientValue > m_slider) {
            isActiv = false;
        }
        break;

    default: // All other gradients
        if (gradientValue < m_slider) {
            isActiv = false;
        }
        break;
    }

    if (!isActiv && gradientValue != -1) { // FIXME No magic number... Defined in KalziumGradientFactory
        gradientColor = Qt::transparent;
    } else {
        const double coeff = gradient()->elementCoeff(el);
        gradientColor = gradient()->calculateColor(coeff);
    }
    return gradientColor;
}

#include "moc_kalziumelementproperty.cpp"
