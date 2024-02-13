/*
    SPDX-FileCopyrightText: 2003, 2004, 2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "detailinfodlg.h"

#include "element.h"
#include "isotope.h"
#include "kalziumdataobject.h"
#include "kalziumutils.h"
#include "orbitswidget.h"
#include "prefs.h"
#include "psetables.h"
#include "spectrumviewimpl.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QLocale>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTextBrowser>

#include <KActionCollection>
#include <KHelpClient>

DetailedInfoDlg::DetailedInfoDlg(int el, QWidget *parent)
    : KPageDialog(parent)
    , m_tableTyp(0)
{
    setFaceType(List);

    buttonBox()->clear();
    buttonBox()->addButton(QDialogButtonBox::Close);
    buttonBox()->addButton(QDialogButtonBox::Help);

    const QString nextButtonIconSource = (layoutDirection() == Qt::LeftToRight) ? "arrow-right" : "arrow-left";
    auto nextButton = new QPushButton(QIcon::fromTheme(nextButtonIconSource), i18nc("Next element", "Next"), this);
    nextButton->setToolTip(i18n("Goes to the next element"));

    const QString prevButtonIconSource = (layoutDirection() == Qt::LeftToRight) ? "arrow-left" : "arrow-right";
    auto prevButton = new QPushButton(QIcon::fromTheme(prevButtonIconSource), i18nc("Previous element", "Previous"), this);
    prevButton->setToolTip(i18n("Goes to the previous element"));

    buttonBox()->addButton(prevButton, QDialogButtonBox::ActionRole);
    buttonBox()->addButton(nextButton, QDialogButtonBox::ActionRole);

    resize(820, 580);

    m_baseHtml = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QStringLiteral("data/htmlview/"), QStandardPaths::LocateDirectory);
    m_baseHtml2 = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QStringLiteral("data/hazardsymbols/"), QStandardPaths::LocateDirectory);

    // X     m_picsdir = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "elempics/") + "elempics/";
    // X     m_picsdir = QFileInfo(m_picsdir).absolutePath();

    // creating the tabs but not the contents, as that will be done when setting the element
    createContent();

    m_actionCollection = new KActionCollection(this);
    KStandardAction::quit(this, SLOT(close()), m_actionCollection);

    connect(prevButton, &QPushButton::clicked, this, &DetailedInfoDlg::showPreviousElement);
    connect(nextButton, &QPushButton::clicked, this, &DetailedInfoDlg::showNextElement);
    connect(buttonBox(), &QDialogButtonBox::helpRequested, this, &DetailedInfoDlg::slotHelp);

    // setting the element and updating the whole dialog
    setElement(el);
}

DetailedInfoDlg::~DetailedInfoDlg()
{
    qDeleteAll(m_htmlpages);
}

void DetailedInfoDlg::setElement(int el)
{
    Element *element = KalziumDataObject::instance()->element(el);
    if (!element) {
        return;
    }

    m_element = element;
    m_elementNumber = el;

    Q_EMIT elementChanged(m_elementNumber);

    reloadContent();

    /* enableButton(User1, true);
     user2Button->setEnabled(true);
      if (m_elementNumber == 1) {
          user2Button->setEnabled(false);
      } else if (m_elementNumber == KalziumDataObject::instance()->numberOfElements()) {
          user1Button->setEnabled(false);
      }*/
}

// void DetailedInfoDlg::setOverviewBackgroundColor(const QColor &bgColor)
// {
// //     dTab->setBackgroundColor(bgColor);
// }

void DetailedInfoDlg::setTableType(int tableTyp)
{
    m_tableTyp = tableTyp;
}

QTextBrowser *DetailedInfoDlg::addHTMLTab(const QString &title, const QString &icontext, const QString &iconname)
{
    auto frame = new QWidget(this);
    KPageWidgetItem *item = addPage(frame, title);
    item->setHeader(icontext);
    item->setIcon(QIcon::fromTheme(iconname));
    auto layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);
    auto browser = new QTextBrowser(frame);
    browser->setOpenExternalLinks(true);
    layout->addWidget(browser);
    return browser;
}

void DetailedInfoDlg::fillHTMLTab(QTextBrowser *browser, const QString &htmlcode)
{
    if (!browser) {
        return;
    }
    browser->setHtml(htmlcode);
}

QString DetailedInfoDlg::getHtml(DATATYPE type)
{
    QString html = R"(<table width="100%" summary="characteristics">)";

    switch (type) {
    case MISC: {
        // discovery date and discoverers
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(discovery.png" alt="icon"/></td><td>)");
        html += KalziumUtils::prettyUnit(m_element, ChemicalDataObject::date);
        QString discoverers = m_element->dataAsString(ChemicalDataObject::discoverers);
        if (!discoverers.isEmpty()) {
            discoverers = discoverers.replace(';', QLatin1String(", "));
            html += "<br />" + i18n("It was discovered by %1.", discoverers);
        }
        html.append("</td></tr>");
        // origin of the name
        QString nameorigin = m_element->dataAsString(ChemicalDataObject::nameOrigin);
        if (!nameorigin.isEmpty()) {
            html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(book.png" alt="icon"/></td><td>)");
            html.append(i18n("Origin of the name:<br/>%1", nameorigin));
            html.append("</td></tr>");
        }
        // X             if (m_element->artificial() || m_element->radioactive()) {
        // X                 html.append("<tr><td><img src=\"file://" + m_baseHtml + "structure.png\" alt=\"icon\"/></td><td>");
        // X                 if (!m_element->radioactive()) {
        // X                     html.append(i18n("This element is artificial"));
        // X                 } else if (!m_element->artificial()) {
        // X                     html.append(i18n("This element is radioactive"));
        // X                 } else {
        // X                     html.append(i18n("This element is radioactive and artificial"));
        // X                 }
        // X                 html.append("</td></tr>");
        // X             }
        break;
    }
    case ISOTOPES: {
        html.append("<tr><td>");
        html.append(isotopeTable());
        html.append("</td></tr>");
        break;
    }
    case DATA: {
        // melting point
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(meltingpoint.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Melting Point")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::meltingpoint));
        html.append("</td></tr>");

        // boiling point
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(boilingpoint.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Boiling Point")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::boilingpoint));
        html.append("</td></tr>");

        // electro affinity
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(electronaffinity.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Electron Affinity")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::electronAffinity));
        html.append("</td></tr>");

        // Electronic configuration
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(structure.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Electronic configuration")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::electronicConfiguration));
        html.append("</td></tr>");

        // covalent radius
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(radius.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Covalent Radius")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::radiusCovalent));
        html.append("</td></tr>");

        // van der Waals radius
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(radius.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Van der Waals Radius")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::radiusVDW));
        html.append("</td></tr>");

        // mass
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(mass.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Atomic mass")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::mass));
        html.append("</td></tr>");

        // 1st ionization energy
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(ionization.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Ionization energy"), i18n("First Ionization energy")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::ionization));
        html.append("</td></tr>");

        // electro negativity
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(structure.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Electronegativity")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::electronegativityPauling));
        html.append("</td></tr>");

        // Oxidation numbers
        html.append("<tr><td><img src=\"file://" + m_baseHtml + R"(ionization.png" alt="icon"/></td><td>)");
        html.append(createWikiLink(i18n("Oxidation states")));
        html.append("</td><td>");
        html.append(KalziumUtils::prettyUnit(m_element, ChemicalDataObject::oxidation));
        html.append("</td></tr>");
        break;
    }
    case EXTRA: {
        // Wikipedia.org
        //         html.append ("<tr><td><img src=\"file://" + m_baseHtml + "wiki.png\" alt=\"icon\"/></td><td>");
        html.append("<tr><td>");
        html.append(createWikiLink(m_element->dataAsString(ChemicalDataObject::name),
                                   i18nc("Link to element's Wikipedia page, %1 is localized language name", "Wikipedia (%1)", QLocale().nativeLanguageName())));
        html.append("</td></tr>");

        // https://education.jlab.org/itselemental/ele001.html
        html.append("<tr><td>");
        html.append("<a href=\"https://"); // https://
        html.append("education.jlab.org/itselemental/ele");
        html.append(QStringLiteral("%1").arg(m_element->dataAsString(ChemicalDataObject::atomicNumber), 3, '0'));
        html.append(".html");
        html.append(R"(" target="_blank" > )");
        html.append("Jefferson Lab");
        html.append("</a>");
        html.append("</td></tr>");

        // FIXME only works with english locals
        html.append("<tr><td>");
        html.append("<a href=\"https://"); // https://
        html.append("www.webelements.com/");
        if (QLocale().uiLanguages().first().startsWith(QLatin1String("en"))) {
            html.append(m_element->dataAsString(ChemicalDataObject::name).toLower()); // hydrogen
        }
        html.append(R"(" target="_blank" >)");
        html.append("Webelements");
        html.append("</a></td></tr>");

        // chemipedia.org
        // html.append("<tr><td><img src=\"file://" + m_baseHtml + "chemi.png\" alt=\"icon\"/></td><td>");

        // html.append("</td></tr>");
        // physics.nist.gov
        // html.append("<tr><td><img src=\"file://" + m_baseHtml + "nist.png\" alt=\"icon\"/></td><td>");

        // html.append("</td></tr>");
    }
    }

    html += QLatin1String("</table></div></body></html>");

    return html;
}

QString DetailedInfoDlg::isotopeTable() const
{
    QList<Isotope *> list = KalziumDataObject::instance()->isotopes(m_elementNumber);

    QString html;
    html = QStringLiteral("<table cellspacing=\"0\" border=\"1\" cellpadding=\"3\" style=\"border-style: solid;\"><tr><th>");
    html += i18n("Mass");
    html += QLatin1String("</th><th>");
    html += i18n("Neutrons");
    html += QLatin1String("</th><th>");
    html += i18n("Percentage");
    html += QLatin1String("</th><th>");
    html += i18n("Half-life period");
    html += QLatin1String("</th><th>");
    html += i18n("Energy and Mode of Decay");
    html += QLatin1String("</th><th>");
    html += i18n("Spin and Parity");
    html += QLatin1String("</th><th>");
    html += i18n("Magnetic Moment");
    html += QLatin1String("</th></tr>");

    foreach (Isotope *isotope, list) {
        html.append("<tr><td align=\"right\">");
        if (isotope->mass() > 0.0) {
            html.append(i18n("%1 u", isotope->mass()));
        }
        html.append("</td><td>");
        html.append(QString::number(((isotope)->nucleons() - (isotope)->parentElementNumber())));
        html.append("</td><td>");
        if (!(isotope)->abundance().isEmpty()) {
            html.append(i18nc("this can for example be '24%'", "%1%", (isotope)->abundance()));
        }
        html.append("</td><td>");
        if ((isotope)->halflife() > 0.0) {
            html.append(i18nc("The first argument is the value, the second is the unit. For example '17 s' for '17 seconds',.",
                              "%1 %2",
                              (isotope)->halflife(),
                              (isotope)->halflifeUnit()));
        }
        html.append("</td><td>");
        if ((isotope)->spontfissionlikeliness() > 0.0) {
            if ((isotope)->spontfissiondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->spontfissiondecay()));
            }
            html.append(i18nc("Spontaneous fission", " SF"));
            if ((isotope)->spontfissionlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->spontfissionlikeliness()));
            }
            if ((isotope)->alphalikeliness() > 0.0 || (isotope)->protonlikeliness() > 0.0 || (isotope)->twoprotonlikeliness() > 0.0
                || (isotope)->neutronlikeliness() > 0.0 || (isotope)->twoneutronlikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0
                || (isotope)->twoeclikeliness() > 0.0 || (isotope)->betaminuslikeliness() > 0.0 || (isotope)->betaminusfissionlikeliness() > 0.0
                || (isotope)->twobetaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0
                || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->alphalikeliness() > 0.0) {
            if ((isotope)->alphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->alphadecay()));
            }
            html.append(i18nc("Alpha decay", " %1", QChar(945)));
            if ((isotope)->alphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->alphalikeliness()));
            }
            if ((isotope)->protonlikeliness() > 0.0 || (isotope)->twoprotonlikeliness() > 0.0 || (isotope)->neutronlikeliness() > 0.0
                || (isotope)->twoneutronlikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0 || (isotope)->twoeclikeliness() > 0.0
                || (isotope)->betaminuslikeliness() > 0.0 || (isotope)->betaminusfissionlikeliness() > 0.0 || (isotope)->twobetaminuslikeliness() > 0.0
                || (isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0
                || (isotope)->betaminustwoneutronlikeliness() > 0.0 || (isotope)->betaminusthreeneutronlikeliness() > 0.0
                || (isotope)->betaminusfourneutronlikeliness() > 0.0 || (isotope)->betaminusalphaneutronlikeliness() > 0.0
                || (isotope)->betaminusalphalikeliness() > 0.0 || (isotope)->betaminustwoalphalikeliness() > 0.0
                || (isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->protonlikeliness() > 0.0) {
            if ((isotope)->protondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->protondecay()));
            }
            html.append(i18nc("Proton decay", " p"));
            if ((isotope)->protonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->protonlikeliness()));
            }
            if ((isotope)->twoprotonlikeliness() > 0.0 || (isotope)->neutronlikeliness() > 0.0 || (isotope)->twoneutronlikeliness() > 0.0
                || (isotope)->eclikeliness() > 0.0 || (isotope)->twoeclikeliness() > 0.0 || (isotope)->betaminuslikeliness() > 0.0
                || (isotope)->betaminusfissionlikeliness() > 0.0 || (isotope)->twobetaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0
                || (isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->twoprotonlikeliness() > 0.0) {
            if ((isotope)->twoprotondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->twoprotondecay()));
            }
            html.append(i18n(" 2p"));
            if ((isotope)->twoprotonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->twoprotonlikeliness()));
            }
            if ((isotope)->neutronlikeliness() > 0.0 || (isotope)->twoneutronlikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0
                || (isotope)->twoeclikeliness() > 0.0 || (isotope)->betaminuslikeliness() > 0.0 || (isotope)->betaminusfissionlikeliness() > 0.0
                || (isotope)->twobetaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0
                || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->neutronlikeliness() > 0.0) {
            if ((isotope)->neutrondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->neutrondecay()));
            }
            html.append(i18nc("Neutron decay", " n"));
            if ((isotope)->neutronlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->neutronlikeliness()));
            }
            if ((isotope)->twoneutronlikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0 || (isotope)->twoeclikeliness() > 0.0
                || (isotope)->betaminuslikeliness() > 0.0 || (isotope)->betaminusfissionlikeliness() > 0.0 || (isotope)->twobetaminuslikeliness() > 0.0
                || (isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0
                || (isotope)->betaminustwoneutronlikeliness() > 0.0 || (isotope)->betaminusthreeneutronlikeliness() > 0.0
                || (isotope)->betaminusfourneutronlikeliness() > 0.0 || (isotope)->betaminusalphaneutronlikeliness() > 0.0
                || (isotope)->betaminusalphalikeliness() > 0.0 || (isotope)->betaminustwoalphalikeliness() > 0.0
                || (isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->twoneutronlikeliness() > 0.0) {
            if ((isotope)->twoneutrondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->twoneutrondecay()));
            }
            html.append(i18n(" 2n"));
            if ((isotope)->twoneutronlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->twoneutronlikeliness()));
            }
            if ((isotope)->eclikeliness() > 0.0 || (isotope)->twoeclikeliness() > 0.0 || (isotope)->betaminuslikeliness() > 0.0
                || (isotope)->betaminusfissionlikeliness() > 0.0 || (isotope)->twobetaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0
                || (isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->eclikeliness() > 0.0) {
            if ((isotope)->ecdecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->ecdecay()));
            }
            html.append(i18nc("Electron capture", " EC"));
            if ((isotope)->eclikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->eclikeliness()));
            }
            if ((isotope)->twoeclikeliness() > 0.0 || (isotope)->betaminuslikeliness() > 0.0 || (isotope)->betaminusfissionlikeliness() > 0.0
                || (isotope)->twobetaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0
                || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->twoeclikeliness() > 0.0) {
            if ((isotope)->twoecdecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->twoecdecay()));
            }
            html.append(i18n(" 2EC"));
            if ((isotope)->twoeclikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->twoeclikeliness()));
            }
            if ((isotope)->betaminuslikeliness() > 0.0 || (isotope)->betaminusfissionlikeliness() > 0.0 || (isotope)->twobetaminuslikeliness() > 0.0
                || (isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0
                || (isotope)->betaminustwoneutronlikeliness() > 0.0 || (isotope)->betaminusthreeneutronlikeliness() > 0.0
                || (isotope)->betaminusfourneutronlikeliness() > 0.0 || (isotope)->betaminusalphaneutronlikeliness() > 0.0
                || (isotope)->betaminusalphalikeliness() > 0.0 || (isotope)->betaminustwoalphalikeliness() > 0.0
                || (isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminuslikeliness() > 0.0) {
            if ((isotope)->betaminusdecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusdecay()));
            }
            html.append(i18nc("Electron emmision", " %1<sup>-</sup>", QChar(946)));
            if ((isotope)->betaminuslikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminuslikeliness()));
            }
            if ((isotope)->betaminusfissionlikeliness() > 0.0 || (isotope)->twobetaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0
                || (isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminusfissionlikeliness() > 0.0) {
            if ((isotope)->betaminusfissiondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusfissiondecay()));
            }
            html.append(i18n(" %1<sup>-</sup>, fission", QChar(946)));
            if ((isotope)->betaminusfissionlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminusfissionlikeliness()));
            }
            if ((isotope)->twobetaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0
                || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->twobetaminuslikeliness() > 0.0) {
            if ((isotope)->twobetaminusdecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->twobetaminusdecay()));
            }
            html.append(i18n(" 2%1<sup>-</sup>", QChar(946)));
            if ((isotope)->twobetaminuslikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->twobetaminuslikeliness()));
            }
            if ((isotope)->betapluslikeliness() > 0.0 || (isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0
                || (isotope)->betaminustwoneutronlikeliness() > 0.0 || (isotope)->betaminusthreeneutronlikeliness() > 0.0
                || (isotope)->betaminusfourneutronlikeliness() > 0.0 || (isotope)->betaminusalphaneutronlikeliness() > 0.0
                || (isotope)->betaminusalphalikeliness() > 0.0 || (isotope)->betaminustwoalphalikeliness() > 0.0
                || (isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betapluslikeliness() > 0.0) {
            if ((isotope)->betaplusdecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaplusdecay()));
            }
            html.append(i18nc("Positron emission", " %1<sup>+</sup>", QChar(946)));
            if ((isotope)->betapluslikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betapluslikeliness()));
            }
            if ((isotope)->twobetapluslikeliness() > 0.0 || (isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->twobetapluslikeliness() > 0.0) {
            if ((isotope)->twobetaplusdecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->twobetaplusdecay()));
            }
            html.append(i18n(" 2%1<sup>+</sup>", QChar(946)));
            if ((isotope)->twobetapluslikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->twobetapluslikeliness()));
            }
            if ((isotope)->betaminusneutronlikeliness() > 0.0 || (isotope)->betaminustwoneutronlikeliness() > 0.0
                || (isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminusneutronlikeliness() > 0.0) {
            if ((isotope)->betaminusneutrondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusneutrondecay()));
            }
            html.append(i18n(" %1 <sup>-</sup>n", QChar(946)));
            if ((isotope)->betaminusneutronlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminusneutronlikeliness()));
            }
            if ((isotope)->betaminustwoneutronlikeliness() > 0.0 || (isotope)->betaminusthreeneutronlikeliness() > 0.0
                || (isotope)->betaminusfourneutronlikeliness() > 0.0 || (isotope)->betaminusalphaneutronlikeliness() > 0.0
                || (isotope)->betaminusalphalikeliness() > 0.0 || (isotope)->betaminustwoalphalikeliness() > 0.0
                || (isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminustwoneutronlikeliness() > 0.0) {
            if ((isotope)->betaminustwoneutrondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminustwoneutrondecay()));
            }
            html.append(i18n(" %1<sup>-</sup>2n", QChar(946)));
            if ((isotope)->betaminustwoneutronlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminustwoneutronlikeliness()));
            }
            if ((isotope)->betaminusthreeneutronlikeliness() > 0.0 || (isotope)->betaminusfourneutronlikeliness() > 0.0
                || (isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminusthreeneutronlikeliness() > 0.0) {
            if ((isotope)->betaminusthreeneutrondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusthreeneutrondecay()));
            }
            html.append(i18n(" %1<sup>-</sup>3n", QChar(946)));
            if ((isotope)->betaminusthreeneutronlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminusthreeneutronlikeliness()));
            }
            if ((isotope)->betaminusfourneutronlikeliness() > 0.0 || (isotope)->betaminusalphaneutronlikeliness() > 0.0
                || (isotope)->betaminusalphalikeliness() > 0.0 || (isotope)->betaminustwoalphalikeliness() > 0.0
                || (isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminusfourneutronlikeliness() > 0.0) {
            if ((isotope)->betaminusfourneutrondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusfourneutrondecay()));
            }
            html.append(i18n(" %1<sup>-</sup>4n", QChar(946)));
            if ((isotope)->betaminusfourneutronlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminusfourneutronlikeliness()));
            }
            if ((isotope)->betaminusalphaneutronlikeliness() > 0.0 || (isotope)->betaminusalphalikeliness() > 0.0
                || (isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminusalphaneutronlikeliness() > 0.0) {
            if ((isotope)->betaminusalphaneutrondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusalphaneutrondecay()));
            }
            html.append(i18n(" %1<sup>-</sup>%2 n", QChar(946), QChar(945)));
            if ((isotope)->betaminusalphaneutronlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminusalphaneutronlikeliness()));
            }
            if ((isotope)->betaminusalphalikeliness() > 0.0 || (isotope)->betaminustwoalphalikeliness() > 0.0
                || (isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminusalphalikeliness() > 0.0) {
            if ((isotope)->betaminusalphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusalphadecay()));
            }
            html.append(i18n(" %1<sup>-</sup>%2", QChar(946), QChar(945)));
            if ((isotope)->betaminusalphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminusalphalikeliness()));
            }
            if ((isotope)->betaminustwoalphalikeliness() > 0.0 || (isotope)->betaminusthreealphalikeliness() > 0.0
                || (isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminustwoalphalikeliness() > 0.0) {
            if ((isotope)->betaminustwoalphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminustwoalphadecay()));
            }
            html.append(i18n(" %1<sup>-</sup>2%2", QChar(946), QChar(945)));
            if ((isotope)->betaminustwoalphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminustwoalphalikeliness()));
            }
            if ((isotope)->betaminusthreealphalikeliness() > 0.0 || (isotope)->betaplusprotonlikeliness() > 0.0
                || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaminusthreealphalikeliness() > 0.0) {
            if ((isotope)->betaminusthreealphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaminusthreealphadecay()));
            }
            html.append(i18n(" %1<sup>-</sup>3%2", QChar(946), QChar(945)));
            if ((isotope)->betaminusthreealphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaminusthreealphalikeliness()));
            }
            if ((isotope)->betaplusprotonlikeliness() > 0.0 || (isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0
                || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaplusprotonlikeliness() > 0.0) {
            if ((isotope)->betaplusprotondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaplusprotondecay()));
            }
            html.append(i18n(" %1<sup>+</sup>p", QChar(946)));
            if ((isotope)->betaplusprotonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaplusprotonlikeliness()));
            }
            if ((isotope)->betaplustwoprotonlikeliness() > 0.0 || (isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0
                || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaplustwoprotonlikeliness() > 0.0) {
            if ((isotope)->betaplustwoprotondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaplustwoprotondecay()));
            }
            html.append(i18n(" %1<sup>+</sup>2p", QChar(946)));
            if ((isotope)->betaplustwoprotonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaplustwoprotonlikeliness()));
            }
            if ((isotope)->betaplusalphalikeliness() > 0.0 || (isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0
                || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaplusalphalikeliness() > 0.0) {
            if ((isotope)->betaplusalphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaplusalphadecay()));
            }
            html.append(i18n(" %1<sup>+</sup>%2", QChar(946), QChar(945)));
            if ((isotope)->betaplusalphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaplusalphalikeliness()));
            }
            if ((isotope)->betaplustwoalphalikeliness() > 0.0 || (isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0
                || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0
                || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaplustwoalphalikeliness() > 0.0) {
            if ((isotope)->betaplustwoalphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaplustwoalphadecay()));
            }
            html.append(i18n(" %1<sup>+</sup>2%2", QChar(946), QChar(945)));
            if ((isotope)->betaplustwoalphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaplustwoalphalikeliness()));
            }
            if ((isotope)->betaplusthreealphalikeliness() > 0.0 || (isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0
                || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->betaplusthreealphalikeliness() > 0.0) {
            if ((isotope)->betaplusthreealphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->betaplusthreealphadecay()));
            }
            html.append(i18n(" %1<sup>+</sup>3%2", QChar(946), QChar(945)));
            if ((isotope)->betaplusthreealphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->betaplusthreealphalikeliness()));
            }
            if ((isotope)->alphabetaminuslikeliness() > 0.0 || (isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0
                || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->alphabetaminuslikeliness() > 0.0) {
            if ((isotope)->alphabetaminusdecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->alphabetaminusdecay()));
            }
            html.append(i18n(" %1 %2<sup>-</sup>", QChar(945), QChar(946)));
            if ((isotope)->alphabetaminuslikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->alphabetaminuslikeliness()));
            }
            if ((isotope)->protonalphalikeliness() > 0.0 || (isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0
                || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->protonalphalikeliness() > 0.0) {
            if ((isotope)->protonalphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->protonalphadecay()));
            }
            html.append(i18n(" p%1", QChar(945)));
            if ((isotope)->protonalphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->protonalphalikeliness()));
            }
            if ((isotope)->ecprotonlikeliness() > 0.0 || (isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0
                || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->ecprotonlikeliness() > 0.0) {
            if ((isotope)->ecprotondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->ecprotondecay()));
            }
            html.append(i18n(" ECp"));
            if ((isotope)->ecprotonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->ecprotonlikeliness()));
            }
            if ((isotope)->ectwoprotonlikeliness() > 0.0 || (isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->ectwoprotonlikeliness() > 0.0) {
            if ((isotope)->ectwoprotondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->ectwoprotondecay()));
            }
            html.append(i18n(" EC2p"));
            if ((isotope)->ectwoprotonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->ectwoprotonlikeliness()));
            }
            if ((isotope)->ecthreeprotonlikeliness() > 0.0 || (isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0
                || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->ecthreeprotonlikeliness() > 0.0) {
            if ((isotope)->ecthreeprotondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->ecthreeprotondecay()));
            }
            html.append(i18n(" EC3p"));
            if ((isotope)->ecthreeprotonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->ecthreeprotonlikeliness()));
            }
            if ((isotope)->ecalphalikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->ecalphalikeliness() > 0.0) {
            if ((isotope)->ecalphadecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->ecalphadecay()));
            }
            html.append(i18n(" EC%1", QChar(945)));
            if ((isotope)->ecalphalikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->ecalphalikeliness()));
            }
            if ((isotope)->ecalphaprotonlikeliness() > 0.0 || (isotope)->ecalphaprotonlikeliness() > 0.0) {
                html.append(i18n(", "));
            }
        }
        if ((isotope)->ecalphaprotonlikeliness() > 0.0) {
            if ((isotope)->ecalphaprotondecay() > 0.0) {
                html.append(i18n("%1 MeV", (isotope)->ecalphaprotondecay()));
            }
            html.append(i18n(" EC%1 p", QChar(945)));
            if ((isotope)->ecalphaprotonlikeliness() < 100.0) {
                html.append(i18n("(%1%)", (isotope)->ecalphaprotonlikeliness()));
            }
        }

        /*        if ((isotope)->alphalikeliness() > 0.0) {
                    if ((isotope)->alphadecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->alphadecay()));
                    }
                    html.append(i18n(" %1", QChar(945)));
                    if ((isotope)->alphalikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->alphalikeliness()));
                    }
                    if ((isotope)->betaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0) {
                        html.append(i18n(", "));
                    }
                }
                if ((isotope)->alphabetaminuslikeliness() > 0.0) {
                    if ((isotope)->alphabetaminusdecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->alphabetaminusdecay()));
                    }
                    html.append(i18n(" %1, %2<sup>-</sup>", QChar(945), QChar(946) ));
                    if ((isotope)->alphabetaminuslikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->alphabetaminuslikeliness()));
                    }
                    if ((isotope)->betaminuslikeliness() > 0.0 || (isotope)->betapluslikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0) {
                        html.append(i18n(", "));
                    }
                }
                if ((isotope)->betaminuslikeliness() > 0.0) {
                    if ((isotope)->betaminusdecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->betaminusdecay()));
                    }
                    html.append(i18n(" %1<sup>-</sup>", QChar(946)));
                    if ((isotope)->betaminuslikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->betaminuslikeliness()));
                    }

                    if ((isotope)->betapluslikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0) {
                        html.append(i18n(", "));
                    }
                }
                if ((isotope)->betaminusneutronlikeliness() > 0.0) {
                    if ((isotope)->betaminusneutrondecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->betaminusneutrondecay()));
                    }
                    html.append(i18n(" %1<sup>-</sup>, n", QChar(946)));
                    if ((isotope)->betaminusneutronlikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->betaminusneutronlikeliness()));
                    }

                    if ((isotope)->betapluslikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0) {
                        html.append(i18n(", "));
                    }
                }
                if ((isotope)->betaminusfissionlikeliness() > 0.0) {
                    if ((isotope)->betaminusfissiondecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->betaminusfissiondecay()));
                    }
                    html.append(i18n(" %1<sup>-</sup>, fission", QChar(946)));
                    if ((isotope)->betaminusfissionlikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->betaminusfissionlikeliness()));
                    }

                    if ((isotope)->betapluslikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0) {
                        html.append(i18n(", "));
                    }
                }
                if ((isotope)->betaminusalphalikeliness() > 0.0) {
                    if ((isotope)->betaminusalphadecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->betaminusalphadecay()));
                    }
                    html.append(i18n(" %1<sup>-</sup>, %2", QChar(946), QChar(945)));
                    if ((isotope)->betaminusalphalikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->betaminusalphalikeliness()));
                    }

                    if ((isotope)->betapluslikeliness() > 0.0 || (isotope)->eclikeliness() > 0.0) {
                        html.append(i18n(", "));
                    }
                }
                if ((isotope)->betapluslikeliness() > 0.0) {
                    if ((isotope)->betaplusdecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->betaplusdecay()));
                    }
                    html.append(i18n(" %1<sup>+</sup>", QChar(946)));
                    if ((isotope)->betapluslikeliness() == (isotope)->eclikeliness()) {
                        if ((isotope)->ecdecay() > 0.0) {
                            html.append(i18n("%1 MeV", (isotope)->ecdecay()));
                        }
                        html.append(i18nc("Acronym of Electron Capture"," EC"));
                    }
                    if ((isotope)->betapluslikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->betapluslikeliness()));
                    }
                    html += ' ';
                }
                if ((isotope)->betaplusprotonlikeliness() > 0.0) {
                    if ((isotope)->betaplusprotondecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->betaplusprotondecay()));
                    }
                    html.append(i18n(" %1<sup>+</sup>, p", QChar(946)));
                    if ((isotope)->betaplusprotonlikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->betaplusprotonlikeliness()));
                    }
                    html += ' ';
                }
                if ((isotope)->betaplusalphalikeliness() > 0.0) {
                    if ((isotope)->betaplusalphadecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->betaplusalphadecay()));
                    }
                    html.append(i18n(" %1<sup>+</sup>, %2", QChar(946),QChar(945)));
                    if ((isotope)->betaplusalphalikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->betaplusalphalikeliness()));
                    }
                    html += ' ';
                }
                if ((isotope)->eclikeliness() > 0.0) {
                    if ((isotope)->ecdecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->ecdecay()));
                    }
                    html.append(i18nc("Acronym of Electron Capture"," EC"));
                    if ((isotope)->eclikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->eclikeliness()));
                    }
                }
                if ((isotope)->neutronlikeliness() > 0.0) {
                    if ((isotope)->neutrondecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->neutrondecay()));
                    }
                    html.append(i18nc("Acronym of neutron emission"," n"));
                    if ((isotope)->neutronlikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->neutronlikeliness()));
                    }
                }
            if ((isotope)->protonlikeliness() > 0.0) {
                    if ((isotope)->protondecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->protondecay()));
                    }
                    html.append(i18nc("Acronym of proton emission"," p"));
                    if ((isotope)->protonlikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->protonlikeliness()));
                    }
                }
            if ((isotope)->protonalphalikeliness() > 0.0) {
                    if ((isotope)->protonalphadecay() > 0.0) {
                        html.append(i18n("%1 MeV", (isotope)->protonalphadecay()));
                    }
                    html.append(i18n(" p, %1", QChar(945)));
                    if ((isotope)->protonlikeliness() < 100.0) {
                        html.append(i18n("(%1%)", (isotope)->protonlikeliness()));
                    }
                }

        */
        html.append("</td><td>");
        html.append((isotope)->spin());
        html.append("</td><td>");
        if (!(isotope)->magmoment().isEmpty()) {
            html.append(i18n("%1 %2<sub>n</sub>", (isotope)->magmoment(), QChar(956)));
        }
        html.append("</td></tr>");
    }

    html += QLatin1String("</table>");

    return html;
}

void DetailedInfoDlg::createContent()
{
    KPageWidgetItem *item = nullptr;

    // Removed the overview Tab, because its an Dockwidget and doesn't show much information.
    // overview tab
    //     QWidget *m_pOverviewTab = new QWidget();
    //     item = addPage(m_pOverviewTab, i18n("Overview"));
    //     item->setHeader(i18n("Overview"));
    //     item->setIcon(QIcon("overview"));
    //     QVBoxLayout *overviewLayout = new QVBoxLayout(m_pOverviewTab);
    //     overviewLayout->setMargin(0);
    //     dTab = new DetailedGraphicalOverview(m_pOverviewTab);
    //     dTab->setObjectName("DetailedGraphicalOverview");
    //     overviewLayout->addWidget(dTab);

    // X      // picture tab
    // X      QWidget *m_pPictureTab = new QWidget();
    // X      item = addPage(m_pPictureTab, i18n("Picture"));
    // X      item->setHeader(i18n("What does this element look like?"));
    // X      item->setIcon(QIcon("elempic"));
    // X      QVBoxLayout *mainLayout = new QVBoxLayout(m_pPictureTab);
    // X      mainLayout->setMargin(0);
    // X      piclabel = new QLabel(m_pPictureTab);
    // X      piclabel->setMinimumSize(400, 350);
    // X      mainLayout->addWidget(piclabel);

    // html tab
    m_htmlpages[QStringLiteral("new")] = addHTMLTab(i18n("Data Overview"), i18n("Data Overview"), QStringLiteral("applications-science"));

    // atomic model tab
    auto m_pModelTab = new QWidget(this);
    item = addPage(m_pModelTab, i18n("Atom Model"));
    item->setHeader(i18n("Atom Model"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("orbits")));
    auto modelLayout = new QVBoxLayout(m_pModelTab);
    modelLayout->setContentsMargins(0, 0, 0, 0);
    wOrbits = new OrbitsWidget(m_pModelTab);
    modelLayout->addWidget(wOrbits);

    // html tabs
    m_htmlpages[QStringLiteral("isotopes")] = addHTMLTab(i18n("Isotopes"), i18n("Isotopes"), QStringLiteral("isotopemap"));
    m_htmlpages[QStringLiteral("misc")] = addHTMLTab(i18n("Miscellaneous"), i18n("Miscellaneous"), QStringLiteral("misc"));

    // spectrum widget tab
    auto m_pSpectrumTab = new QWidget(this);
    item = addPage(m_pSpectrumTab, i18n("Spectrum"));
    item->setHeader(i18n("Spectrum"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("spectrum")));
    auto spectrumLayout = new QVBoxLayout(m_pSpectrumTab);
    spectrumLayout->setContentsMargins(0, 0, 0, 0);
    m_spectrumStack = new QStackedWidget(m_pSpectrumTab);
    spectrumLayout->addWidget(m_spectrumStack);
    m_spectrumview = new SpectrumViewImpl(m_spectrumStack);
    m_spectrumview->setObjectName(QStringLiteral("spectrumwidget"));
    m_spectrumStack->addWidget(m_spectrumview);
    m_spectrumLabel = new QLabel(m_spectrumStack);
    m_spectrumStack->addWidget(m_spectrumLabel);

    // html extra tab
    m_htmlpages[QStringLiteral("extra")] = addHTMLTab(i18n("Extra information"), i18n("Extra Information"), QStringLiteral("applications-internet"));
}

void DetailedInfoDlg::reloadContent()
{
    // reading the most common data
    const QString element_name = m_element->dataAsString(ChemicalDataObject::name);
    const QString element_symbol = m_element->dataAsString(ChemicalDataObject::symbol);
    const QString element_block = m_element->dataAsString(ChemicalDataObject::periodTableBlock);

    // updating caption
    setWindowTitle(
        i18nc("@title:window, for example: [C] Carbon (6 - Block p)", "[%1] %2 (%3 - Block %4)", element_symbol, element_name, m_elementNumber, element_block));

    // X      // updating picture tab
    // X      QString picpath = m_picsdir + element_symbol + ".jpg";
    // X      if (QFile::exists(picpath)) {
    // X           QImage img(picpath, "JPEG");
    // X           img = img.scaled(400, 400, Qt::KeepAspectRatio);
    // X           piclabel->setPixmap(QPixmap::fromImage(img));
    // X      } else {
    // X           piclabel->setText(i18n("No picture of %1 found.", element_name));
    // X      }

    // updating atomic model tab
    wOrbits->setElementNumber(m_elementNumber);
    /*
       wOrbits->setWhatsThis(
       i18n("Here you can see the atomic hull of %1. %2 has the configuration %3.")
       .arg(m_element->dataAsString(ChemicalDataObject::name))
       .arg(m_element->dataAsString(ChemicalDataObject::name))
       .arg(""));//m_element->parsedOrbits()));
       */

    // updating html tabs
    fillHTMLTab(m_htmlpages[QStringLiteral("new")], getHtml(DATA));
    fillHTMLTab(m_htmlpages[QStringLiteral("misc")], getHtml(MISC));
    fillHTMLTab(m_htmlpages[QStringLiteral("isotopes")], getHtml(ISOTOPES));
    fillHTMLTab(m_htmlpages[QStringLiteral("extra")], getHtml(EXTRA));

    Spectrum *spec = KalziumDataObject::instance()->spectrum(m_elementNumber);

    // updating spectrum widget
    if (spec) {
        m_spectrumview->setSpectrum(spec);
        m_spectrumStack->setCurrentWidget(m_spectrumview);
    } else {
        m_spectrumLabel->setText(i18n("No spectrum of %1 found.", element_name));
        m_spectrumStack->setCurrentWidget(m_spectrumLabel);
    }
}

QString DetailedInfoDlg::createWikiLink(QString link)
{
    return createWikiLink(link, link);
}

QString DetailedInfoDlg::createWikiLink(QString link, QString displayString)
{
    QString html;
    QString language(QLocale().uiLanguages().first());

    // Wikipedia.org
    html.append("<a href=\"https://"); // https://
    html.append(language.split('-').at(0)); // en.
    html.append(".wikipedia.org/wiki/"); // wikipedia.org
    html.append(link); // /hydrogen
    html.append(R"(" target="_blank" > )");
    html.append(displayString);
    html.append("</a>");
    // Example from the comment "https://en.wikipedia.org/wiki/hydrogen"

    return html;
}

void DetailedInfoDlg::slotHelp()
{
    // TODO fix this stuff...
#if 0
    QString chapter = "infodialog_overview";
    switch (activePageIndex()) {
    case 0:
        chapter = "infodialog_overview";
        break;
    case 1:
        chapter = "infodialog_orbits";
        break;
    case 2:
        chapter = "infodialog_chemical";
        break;
    case 3:
        chapter = "infodialog_energies";
        break;
    case 4:
        chapter = "infodialog_misc";
        break;
    case 5:
        chapter = "infodialog_spectrum";
        break;
    case 6:
        chapter = "infodialog_warnings";
        break;
    }
#endif
    KHelpClient::invokeHelp(QStringLiteral("info-dlg.html#infodialog_spectrum"), QStringLiteral("kalzium"));
}

void DetailedInfoDlg::showNextElement()
{
    setElement(pseTables::instance()->getTabletype(m_tableTyp)->nextOf(m_elementNumber));
}

void DetailedInfoDlg::showPreviousElement()
{
    setElement(pseTables::instance()->getTabletype(m_tableTyp)->previousOf(m_elementNumber));
}

#include "moc_detailinfodlg.cpp"
