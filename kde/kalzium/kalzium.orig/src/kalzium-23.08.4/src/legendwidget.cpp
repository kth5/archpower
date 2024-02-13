/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "legendwidget.h"

#include "prefs.h"

#include "kalzium_debug.h"
#include <QGridLayout>
#include <QHBoxLayout>

#include <KLocalizedString>

LegendWidget::LegendWidget(QWidget *parent)
    : QWidget(parent)
{
    m_update = true;
    m_dockArea = Qt::BottomDockWidgetArea;
}

LegendWidget::~LegendWidget()
{
    qDeleteAll(m_legendItemList);
}

void LegendWidget::setDockArea(Qt::DockWidgetArea newDockArea)
{
    qCDebug(KALZIUM_LOG) << "dock Area changed" << newDockArea;

    m_dockArea = newDockArea;
    updateContent();
}

void LegendWidget::LockWidget()
{
    m_update = false;
}

void LegendWidget::UnLockWidget()
{
    m_update = true;
}

void LegendWidget::updateContent()
{
    if (!m_update) {
        return;
    }

    QString gradientDesc;
    QList<QPair<QString, QColor>> items;
    KalziumElementProperty *elementProperty = KalziumElementProperty::instance();
    // Handle different Gradients
    switch (elementProperty->gradientId()) {
    case KalziumElementProperty::NOGRADIENT: // None
        break;

    case KalziumElementProperty::SOMGradientType:
        items << qMakePair(elementProperty->gradient()->description(), QColor());
        items << qMakePair(i18nc("one of the three states of matter (solid, liquid, vaporous or unknown)", "Solid"), QColor(Prefs::color_solid()));

        items << qMakePair(i18nc("one of the three states of matter (solid, liquid, vaporous or unknown)", "Liquid"), QColor(Prefs::color_liquid()));

        items << qMakePair(i18nc("one of the three states of matter (solid, liquid, vaporous or unknown)", "Vaporous"), QColor(Prefs::color_vapor()));

        items << qMakePair(i18nc("one of the three states of matter (solid, liquid, vaporous or unknown)", "Unknown"), QColor(Qt::lightGray));
        break;

    default:
        if (elementProperty->gradient()->logarithmicGradient()) {
            gradientDesc = i18nc("one of the two types of gradients available", "logarithmic");
        } else {
            gradientDesc = i18nc("one of the two types of gradients available", "linear");
        }
        items << qMakePair(i18n("%1 (%2)", elementProperty->gradient()->description(), gradientDesc), QColor());
        items << qMakePair(i18nc("Minimum value of the gradient",
                                 "Minimum: %1",
                                 QString::number(elementProperty->gradient()->minValue()) + ' ' + elementProperty->gradient()->unit()),
                           QColor(elementProperty->gradient()->firstColor()));

        items << qMakePair(i18nc("Maximum value of the gradient",
                                 "Maximum: %1",
                                 QString::number(elementProperty->gradient()->maxValue()) + ' ' + elementProperty->gradient()->unit()),
                           QColor(elementProperty->gradient()->secondColor()));
        break;
    }
    // schemes are always there
    items << qMakePair(i18n("Scheme: %1", elementProperty->scheme()->description()), QColor());
    items << elementProperty->scheme()->legendItems();

    updateLegendItemLayout(items);
}

void LegendWidget::updateLegendItemLayout(const QList<legendPair> &list)
{
    if (layout()) {
        delete layout();
    }
    foreach (LegendItem *i, m_legendItemList) {
        delete i;
    }

    m_legendItemList.clear();

    auto layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    int x = 0;
    int y = 0;

    foreach (const legendPair &pair, list) {
        auto item = new LegendItem(pair, this);
        m_legendItemList.append(item);

        if ((m_dockArea == Qt::BottomDockWidgetArea || m_dockArea == Qt::TopDockWidgetArea)) {
            if (!pair.second.isValid()) {
                ++y;
                x = 0;
            }
            if (x >= 3) {
                ++y;
                x = 1;
            }
        }
        layout->addWidget(item, x, y);
        ++x;
    }
    setLayout(layout);
}

void LegendWidget::legendItemAction(QColor color)
{
    Q_EMIT resetElementMatch();

    if (color.operator==(QColor())) {
        return;
    }

    for (int element = 1; element <= 118; ++element) {
        if (isElementMatch(element, color)) {
            Q_EMIT elementMatched(element);
        }
    }
}

bool LegendWidget::isElementMatch(int element, QColor &color)
{
    QColor elementBackgroundColor;
    QColor elementBorderColor;

    elementBackgroundColor = KalziumElementProperty::instance()->getElementColor(element);
    elementBorderColor = KalziumElementProperty::instance()->getBorderColor(element);

    switch (Prefs::colorgradientbox()) {
    case KalziumElementProperty::NOGRADIENT:
        if (elementBackgroundColor.operator!=(color)) {
            return true;
        }
        break;

    default: // all other gradients
        if (elementBorderColor.operator==(color) || elementBackgroundColor.operator==(color)) {
            return true;
        }
    }
    return false;
}

LegendItem::LegendItem(const QPair<QString, QColor> &pair, LegendWidget *parent)
{
    auto ItemLayout = new QHBoxLayout(this);
    ItemLayout->setContentsMargins(0, 0, 0, 0);

    if (pair.second.isValid()) {
        legendItemColor = pair.second;
        if (parent) {
            connect(this, &LegendItem::legenItemHoovered, parent, &LegendWidget::legendItemAction);
        }

        QPixmap LegendPixmap(20, height());
        LegendPixmap.fill(pair.second);

        auto LabelPixmap = new QLabel(this);
        LabelPixmap->setPixmap(LegendPixmap);
        ItemLayout->addWidget(LabelPixmap);

        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Sunken);
    }
    auto LegendLabel = new QLabel(this);
    LegendLabel->setText(pair.first);
    ItemLayout->addWidget(LegendLabel);

    ItemLayout->setAlignment(Qt::AlignLeft);
    setLayout(ItemLayout);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void LegendItem::enterEvent(QEnterEvent *event)
#else
void LegendItem::enterEvent(QEvent *event)
#endif
{
    Q_EMIT legenItemHoovered(legendItemColor);
    QWidget::enterEvent(event);
}

void LegendItem::leaveEvent(QEvent *event)
{
    Q_EMIT legenItemHoovered(QColor());
    QWidget::leaveEvent(event);
}

#include "moc_legendwidget.cpp"
