/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotopetablesettingscard.h"

#include <QVBoxLayout>
#include <KLocalizedString>

#include <iostream>

IsotopeTableSettingsCard::IsotopeTableSettingsCard(QWidget *parent, int mode)
    : QFrame(parent)
{
    m_isotopeView = new IsotopeView(this, mode);
    initialize();
}

IsotopeTableSettingsCard::IsotopeTableSettingsCard(QWidget *parent)
    : QFrame(parent)
{
    m_isotopeView = new IsotopeView(this);
    initialize();
}

void IsotopeTableSettingsCard::initialize()
{
    auto vLayout = new QVBoxLayout();

    m_isotopeView->setInteractive(false);
    m_isotopeView->setZoom(0.06);
    // m_isotopeView->setMaximumWidth(200);
    // m_isotopeView->setMaximumHeight(150);

    m_radioButton = new QRadioButton();
    m_radioButton->setText(i18n("Next to each other"));
    connect(m_radioButton, &QRadioButton::toggled, this, [=]() {
        if (m_radioButton->isChecked())
            Q_EMIT checked(m_isotopeView->mode());
    });

    vLayout->addWidget(m_isotopeView);
    vLayout->addWidget(m_radioButton);

    setLayout(vLayout);
    installEventFilter(this);
    m_isotopeView->installEventFilter(this);
    m_isotopeView->viewport()->installEventFilter(this);
    m_radioButton->installEventFilter(this);
    setFocusProxy(m_radioButton);
}

bool IsotopeTableSettingsCard::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::MouseButton::LeftButton) {
            m_radioButton->setChecked(true);
            return true;
        }
    }
    return false;
}

#include "moc_isotopetablesettingscard.cpp"
