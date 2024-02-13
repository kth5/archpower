/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GRADIENTWIDGET_IMPL_H
#define GRADIENTWIDGET_IMPL_H

#include "ui_gradientwidget.h"
#include <QWidget>


class QTimer;

/**
 * @class GradientWidgetImpl

 * @short Provides a widget for setting the appearance of the pse table
 * @author Carsten Niehaus
 * @author Kashyap Puranik
 * @author Etienne Rebetez
 */
class GradientWidgetImpl : public QWidget, public Ui_gradientWidget
{
    Q_OBJECT

public:
    /**
     * @param elementProperty The elementProperty class
     * @param parent The parent of this widget
     */
    explicit GradientWidgetImpl(QWidget *parent = nullptr);
    ~GradientWidgetImpl() override;

Q_SIGNALS:
    /**
     * Is emitted when the value of the gradient or the spinbox is changed.
     */
    void gradientValueChanged(double);

public Q_SLOTS:
    /**
     * Sets the comboboxes to the current values.
     * the current values come form the elementProperty class.
     */
    void slotGradientChanged();

private Q_SLOTS:
    void play();
    void stop();
    void tick();
    void doubleToSlider(double var);
    void intToSpinbox(int var);
    /**
     * is used to display custom text in the widget according to the current value of the gradient
     */
    void setNewValue(double newValue);

private:
    bool m_play = false; // Indicates whether mode is play or stop
    QTimer *const m_timer;
};
#endif // GRADIENTWIDGET_IMPL_H
