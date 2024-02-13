#ifndef KALZIUMCONFIGDIALOG_H
#define KALZIUMCONFIGDIALOG_H

#include <KConfigDialog>

#include "isotopetablesettingsdialog.h"
#include "unitsettingsdialog.h"

class KalziumConfigDialog : public KConfigDialog
{
    Q_OBJECT

public:
    KalziumConfigDialog(QWidget *parent, const QString &name, KCoreConfigSkeleton *config);

    UnitSettingsDialog *m_unitsDialog = nullptr;
    IsotopeTableSettingsDialog *m_isotopeTableSettingsDialog = nullptr;

private:
    bool hasChanged() override;
    bool isDefault() override;

private
    Q_SLOT : void updateWidgetsDefault() override;
    void updateSettings() override;
};

#endif
