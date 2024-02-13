/*
    SPDX-FileCopyrightText: 2006, 2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rsdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

#include "kalziumdataobject.h"
#include "kalziumutils.h"

#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>

RSDialog::RSDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Risks/Security Phrases"));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Close, this);
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RSDialog::reject);
    mainLayout->addWidget(buttonBox);

    createRPhrases();
    createSPhrases();

    ui.setupUi(mainWidget);

    connect(ui.filterButton, &QAbstractButton::clicked, this, &RSDialog::filter);
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &RSDialog::slotHelp);

    filter();
}

void RSDialog::filter()
{
    // if the RS sentence starts or ends with a - invalidate it.
    // It is probably an user error
    if (ui.r_le->text().startsWith('-') || ui.r_le->text().endsWith('-') || ui.s_le->text().startsWith('-') || ui.s_le->text().endsWith('-')) {
        invalidPhaseString();
        return;
    }

    QList<int> r;
    QList<int> s;

    // for now only separation by a - is allowed
    if (!ui.r_le->text().isEmpty()) {
        const QStringList rSplit = ui.r_le->text().split('-');
        for (const QString &st : rSplit) {
            r << st.toInt();
        }
    }

    // for now only separation by a - is allowed
    if (!ui.s_le->text().isEmpty()) {
        const QStringList sSplit = ui.s_le->text().split('-');
        for (const QString &st : sSplit) {
            s << st.toInt();
        }
    }

    filterRS(r, s);
}

void RSDialog::filterRS(const QList<int> &r, const QList<int> &s)
{
    QString string(QStringLiteral("<qt>"));

    if (!r.isEmpty()) {
        string.append("<h2>" + i18n("R-Phrases:") + "</h2>");
        for (int i : r) {
            QString phrase("<b>" + QString::number(i) + " - ");
            phrase.append(rphrase(i) + "</b>");
            string.append(phrase + "<br>");
        }
    }
    if (!s.isEmpty()) {
        string.append("<h2>" + i18n("S-Phrases:") + "</h2>");
        for (int i : s) {
            QString phrase("<b>" + QString::number(i) + " -  ");
            phrase.append(sphrase(i) + "</b>");
            string.append(phrase + "<br>");
        }
    }
    if (s.isEmpty() && r.isEmpty())
        string.append("<h2>" + i18n("You asked for no R/S-Phrases.") + "</h2>");

    string.append("</qt>");

    ui.text->setHtml(string);
}

QString RSDialog::rphrase(int number)
{
    QString p;

    QMap<int, QString>::const_iterator i = rphrases_map.constBegin();
    while (i != rphrases_map.constEnd()) {
        if (i.key() == number) {
            return i.value();
        }

        ++i;
    }

    return p;
}

QString RSDialog::sphrase(int number)
{
    QString p;

    QMap<int, QString>::const_iterator i = sphrases_map.constBegin();
    while (i != sphrases_map.constEnd()) {
        if (i.key() == number) {
            return i.value();
        }

        ++i;
    }

    return p;
}

void RSDialog::createSPhrases()
{
    QStringList sphrases;
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S1: Keep locked up");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S2: Keep out of the reach of children");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S3: Keep in a cool place");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S4: Keep away from living quarters");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S5: Keep contents under ... ( appropriate liquid to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S6: Keep under ... ( inert gas to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S7: Keep container tightly closed");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S8: Keep container dry");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S9: Keep container in a well-ventilated place");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S12: Do not keep the container sealed");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S13: Keep away from food, drink and animal feedingstuffs");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S14: Keep away from ... ( incompatible materials to be indicated by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S15: Keep away from heat");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S16: Keep away from sources of ignition - No smoking");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S17: Keep away from combustible material");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S18: Handle and open container with care");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S20: When using do not eat or drink");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S21: When using do not smoke");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S22: Do not breathe dust");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S23: Do not breathe gas/fumes/vapour/spray ( appropriate wording to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S24: Avoid contact with skin");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S25: Avoid contact with eyes");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S26: In case of contact with eyes, rinse immediately with plenty of water and seek medical advice");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S27: Take off immediately all contaminated clothing");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S28: After contact with skin, wash immediately with plenty of ... ( to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S29: Do not empty into drains");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S30: Never add water to this product");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S33: Take precautionary measures against static discharges");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S35: This material and its container must be disposed of in a safe way");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S36: Wear suitable protective clothing");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S37: Wear suitable gloves");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S38: In case of insufficient ventilation wear suitable respiratory equipment");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S39: Wear eye/face protection");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S40: To clean the floor and all objects contaminated by this material use ... ( to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S41: In case of fire and/or explosion do not breathe fumes");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S42: During fumigation/spraying wear suitable respiratory equipment ( appropriate wording to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S43: In case of fire use ... ( indicate in the space the precise type of fire-fighting equipment. If water increases the risk add - Never use water "
        ")");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S45: In case of accident or if you feel unwell seek medical advice immediately ( show the label where possible )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S46: If swallowed, seek medical advice immediately and show this container or label");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S47: Keep at temperature not exceeding ... °C ( to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S48: Keep wet with ... ( appropriate material to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S49: Keep only in the original container");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S50: Do not mix with ... ( to be specified by the manufacturer )");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S51: Use only in well-ventilated areas");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S52: Not recommended for interior use on large surface areas");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S53: Avoid exposure - obtain special instructions before use");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S56: Dispose of this material and its container at hazardous or special waste collection point");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S57: Use appropriate containment to avoid environmental contamination");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S59: Refer to manufacturer/supplier for information on recovery/recycling");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S60: This material and its container must be disposed of as hazardous waste");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S61: Avoid release to the environment. Refer to special instructions/safety data sheet");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S62: If swallowed, do not induce vomiting: seek medical advice immediately and show this container or label");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S63: In case of accident by inhalation: remove casualty to fresh air and keep at rest");
    sphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "S64: If swallowed, rinse mouth with water ( only if the person is conscious )");

    QRegularExpression reg("(R|S)(\\d+): (.*)");

    for (const QString &p : std::as_const(sphrases)) {
        int number = 0;
        QString phrase(QLatin1String(""));
        QRegularExpressionMatch match = reg.match(p);

        if (match.hasMatch()) {
            const QString part1 = match.captured(2);
            const QString part2 = match.captured(3);

            phrase = part2;
            number = part1.toInt();
        }

        sphrases_map.insert(number, phrase);
    }
}

void RSDialog::createRPhrases()
{
    QStringList rphrases;
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R1: Explosive when dry");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R2: Risk of explosion by shock, friction, fire or other sources of ignition");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R3: Extreme risk of explosion by shock, friction, fire or other sources of ignition");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R4: Forms very sensitive explosive metallic compounds");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R5: Heating may cause an explosion");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R6: Explosive with or without contact with air");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R7: May cause fire");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R8: Contact with combustible material may cause fire");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R9: Explosive when mixed with combustible material");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R10: Flammable");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R11: Highly flammable");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R12: Extremely flammable");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R14: Reacts violently with water");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R15: Contact with water liberates extremely flammable gases");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R16: Explosive when mixed with oxidising substances");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R17: Spontaneously flammable in air");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R18: In use, may form flammable/explosive vapour-air mixture");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R19: May form explosive peroxides");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R20: Harmful by inhalation");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R21: Harmful in contact with skin");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R22: Harmful if swallowed");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R23: Toxic by inhalation");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R24: Toxic in contact with skin");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R25: Toxic if swallowed");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R26: Very toxic by inhalation");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R27: Very toxic in contact with skin");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R28: Very toxic if swallowed");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R29: Contact with water liberates toxic gas.");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R30: Can become highly flammable in use");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R31: Contact with acids liberates toxic gas");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R32: Contact with acids liberates very toxic gas");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R33: Danger of cumulative effects");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R34: Causes burns");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R35: Causes severe burns");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R36: Irritating to eyes");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R37: Irritating to respiratory system");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R38: Irritating to skin");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R39: Danger of very serious irreversible effects");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R40: Limited evidence of a carcinogenic effect");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R41: Risk of serious damage to eyes");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R42: May cause sensitisation by inhalation");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R43: May cause sensitisation by skin contact");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R44: Risk of explosion if heated under confinement");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R45: May cause cancer");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R46: May cause heritable genetic damage");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R48: Danger of serious damage to health by prolonged exposure");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R49: May cause cancer by inhalation");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R50: Very toxic to aquatic organisms");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R51: Toxic to aquatic organisms");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R52: Harmful to aquatic organisms");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R53: May cause long-term adverse effects in the aquatic environment");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R54: Toxic to flora");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R55: Toxic to fauna");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R56: Toxic to soil organisms");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R57: Toxic to bees");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R58: May cause long-term adverse effects in the environment");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R59: Dangerous for the ozone layer");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R60: May impair fertility");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R61: May cause harm to the unborn child");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R62: Possible risk of impaired fertility");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R63: Possible risk of harm to the unborn child");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R64: May cause harm to breast-fed babies");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R65: Harmful: may cause lung damage if swallowed");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R66: Repeated exposure may cause skin dryness or cracking");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R67: Vapours may cause drowsiness and dizziness");
    rphrases << i18nc(
        "Please take the official translations! You find them here: https://eur-lex.europa.eu/LexUriServ/LexUriServ.do?uri=CELEX:32001L0059:EN:HTML",
        "R68: Possible risk of irreversible effects");

    QRegularExpression reg("(R|S)(\\d+): (.*)");

    for (const QString &p : std::as_const(rphrases)) {
        int number = 0;
        QString phrase(QLatin1String(""));
        QRegularExpressionMatch match = reg.match(p);

        if (match.hasMatch()) {
            const QString part1 = match.captured(2);
            const QString part2 = match.captured(3);

            phrase = part2;
            number = part1.toInt();
        }

        rphrases_map.insert(number, phrase);
    }
}

void RSDialog::slotHelp()
{
    KHelpClient::invokeHelp(QStringLiteral("tools.html#rs_phrases"), QStringLiteral("kalzium"));
}

void RSDialog::invalidPhaseString()
{
    KMessageBox::error(nullptr, i18n("At least one of the specified phrases is invalid."));
}

#include "moc_rsdialog.cpp"
