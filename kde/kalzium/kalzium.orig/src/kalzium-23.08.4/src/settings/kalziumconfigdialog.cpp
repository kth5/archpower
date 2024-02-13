#include "kalziumconfigdialog.h"

#include "ui_settings_calc.h"
#include "ui_settings_colors.h"
#include "ui_settings_gradients.h"

KalziumConfigDialog::KalziumConfigDialog(QWidget *parent, const QString &name, KCoreConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
{
    // colors page
    Ui_setupColors ui_colors;
    auto w_colors = new QWidget(this);
    w_colors->setObjectName(QStringLiteral("colors_page"));
    ui_colors.setupUi(w_colors);
    addPage(w_colors, i18n("Schemes"), QStringLiteral("preferences-desktop-color"));

    // gradients page
    Ui_setupGradients ui_gradients;
    auto w_gradients = new QWidget(this);
    w_gradients->setObjectName(QStringLiteral("gradients_page"));
    ui_gradients.setupUi(w_gradients);
    addPage(w_gradients, i18n("Gradients"), QStringLiteral("preferences-desktop-color"));

    // units page
    m_unitsDialog = new UnitSettingsDialog(this);
    m_unitsDialog->setObjectName(QStringLiteral("units_page"));
    addPage(m_unitsDialog, i18n("Units"), QStringLiteral("system-run"));

    // isotope table page
    m_isotopeTableSettingsDialog = new IsotopeTableSettingsDialog(this);
    m_isotopeTableSettingsDialog->setObjectName(QStringLiteral("isotopemap"));
    connect(m_isotopeTableSettingsDialog, &IsotopeTableSettingsDialog::modeChanged, this, &KalziumConfigDialog::updateButtons);
    addPage(m_isotopeTableSettingsDialog, i18n("Isotope Table"), QStringLiteral("isotopemap"));

    Ui_setupCalc ui_calc;
    auto w_calc = new QWidget(this);
    ui_calc.setupUi(w_calc);
    addPage(w_calc, i18n("Calculator"), QStringLiteral("accessories-calculator"));
}

bool KalziumConfigDialog::hasChanged()
{
    return m_isotopeTableSettingsDialog->hasChanged();
}

bool KalziumConfigDialog::isDefault()
{
    return m_isotopeTableSettingsDialog->isDefault();
}

void KalziumConfigDialog::updateWidgetsDefault()
{
    m_isotopeTableSettingsDialog->setMode(Prefs::defaultIsotopeTableModeValue());
}

void KalziumConfigDialog::updateSettings()
{
    Prefs::setLengthUnit(m_unitsDialog->getLenghtUnitId());
    Prefs::setEnergiesUnit(m_unitsDialog->getEnergyUnitId());
    Prefs::setTemperatureUnit(m_unitsDialog->getTemperatureUnitId());

    Prefs::setIsotopeTableMode(m_isotopeTableSettingsDialog->getMode());

    Prefs::self()->save();
}

#include "moc_kalziumconfigdialog.cpp"
