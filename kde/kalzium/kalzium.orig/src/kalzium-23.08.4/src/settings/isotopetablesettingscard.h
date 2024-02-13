/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISOTOPETABLESETTINGSCARD_H
#define ISOTOPETABLESETTINGSCARD_H

#include "isotopeview.h"
#include <QFrame>
#include <QRadioButton>

class IsotopeTableSettingsCard : public QFrame
{
    Q_OBJECT

public:
    explicit IsotopeTableSettingsCard(QWidget *parent = nullptr);
    IsotopeTableSettingsCard(QWidget *parent, int mode);
    QString text() const
    {
        return m_radioButton->text();
    }
    int mode() const
    {
        return m_isotopeView->mode();
    }
    bool isChecked() const
    {
        return m_radioButton->isChecked();
    }

private:
    void initialize();

private:
    IsotopeView *m_isotopeView = nullptr;
    QRadioButton *m_radioButton = nullptr;

public Q_SLOTS:
    void setText(const QString &text)
    {
        m_radioButton->setText(text);
    }
    void setMode(int mode)
    {
        m_isotopeView->setMode(mode);
    }
    void setZoom(qreal zoom)
    {
        m_isotopeView->setZoom(zoom);
    }
    void setChecked(bool checked)
    {
        m_radioButton->setChecked(checked);
    }
    void setRadioButtonObjectName(QString name)
    {
        m_radioButton->setObjectName(name);
    }

Q_SIGNALS:
    void checked(int mode);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

#endif // ISOTOPETABLESETTINGSCARD_H
