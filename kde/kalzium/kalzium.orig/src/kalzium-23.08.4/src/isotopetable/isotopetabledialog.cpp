/*
    SPDX-FileCopyrightText: 2005-2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotopetabledialog.h"

#include "isotopeitem.h"
#include "isotopescene.h"
#include "kalziumschemetype.h"
#include "legendwidget.h"

#include <isotope.h>

#include "kalzium_debug.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtMath>

#include <KConfigGroup>

#include <prefs.h>

IsotopeTableDialog::IsotopeTableDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Isotope Table"));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &IsotopeTableDialog::reject);
    mainLayout->addWidget(buttonBox);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    ui.setupUi(mainWidget);
    ui.guide->setGuidedView(ui.gv);

    connect(ui.gv->scene(), SIGNAL(itemSelected(IsotopeItem *)), this, SLOT(updateDockWidget(IsotopeItem *)));
    connect(ui.gv, &IsotopeView::zoomLevelChanged, this, &IsotopeTableDialog::slotZoomLevelChanged);
    connect(ui.Slider, &QAbstractSlider::valueChanged, this, &IsotopeTableDialog::zoom);

    // Here comes the legend part
    QList<QPair<QString, QColor>> items;

    items << qMakePair(i18nc("alpha ray emission", "alpha"), QColor(Qt::red));
    items << qMakePair(i18nc("Electron capture method", "EC"), QColor(Qt::blue));
    items << qMakePair(i18nc("Many ways", "Multiple"), QColor(Qt::green));
    items << qMakePair(i18nc("Beta plus ray emission", "Beta +"), QColor(Qt::yellow));
    items << qMakePair(i18nc("Beta minus ray emission", "Beta -"), QColor(Qt::white));
    items << qMakePair(i18nc("Stable isotope", "Stable"), QColor(Qt::magenta));
    items << qMakePair(i18nc("Unknown Decay", "unknown"), QColor(Qt::darkGray));

    foreach (const legendPair &pair, items) {
        auto item = new LegendItem(pair);
        ui.infoWidget->layout()->addWidget(item);
    }
    ui.infoWidget->setMinimumWidth(150);
}

void IsotopeTableDialog::zoom(int level)
{
    double zoom = qPow(M_E, level / 10.0);
    (ui.gv)->setZoom(zoom);
}

void IsotopeTableDialog::updateDockWidget(IsotopeItem *item)
{
    Isotope *s = item->isotope();

    const QString header = i18n("<h1>%1 (%2)</h1>", s->parentElementSymbol(), s->parentElementNumber());
    const QString mag = i18n("Magnetic moment: %1", s->magmoment().isEmpty() ? i18nc("Unknown magnetic moment", "Unknown") : s->magmoment());

    QString halflife;
    if (s->halflife() > 0.0) {
        halflife = i18n("Halflife: %1 %2", s->halflife(), s->halflifeUnit());
    } else {
        halflife = i18n("Halflife: Unknown");
    }

    const QString abundance = i18n("Abundance: %1 %", !s->abundance().isEmpty() ? s->abundance() : QStringLiteral("0"));
    const QString nucleons = i18n("Number of nucleons: %1", s->nucleons());
    const QString spin = i18n("Spin: %1", s->spin().isEmpty() ? i18nc("Unknown spin", "Unknown") : s->spin());
    const QString exactMass = i18n("Exact mass: %1 u", s->mass());

    const QString html = header + "<br />" + nucleons + "<br />" + mag + "<br />" + exactMass + "<br />" + spin + "<br />" + abundance + "<br />" + halflife;

    ui.label->setText(html);
}

void IsotopeTableDialog::slotZoomLevelChanged(double value)
{
    const bool b = ui.Slider->blockSignals(true);
    ui.Slider->setValue(qLn(value) * 10.0);
    ui.Slider->blockSignals(b);
}

void IsotopeTableDialog::setMode(int mode)
{
    ui.gv->setMode(mode);
    ui.guide->updateScene();
}

void IsotopeTableDialog::updateMode()
{
    setMode(Prefs::isotopeTableMode());
}

#include "moc_isotopetabledialog.cpp"
