/*
    SPDX-FileCopyrightText: 2007 Johannes Simon <johannes.simon@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "exportdialog.h"

#include "kalziumutils.h"

#include "kalzium_debug.h"
#include <QDialogButtonBox>
#include <QFont>
#include <QPushButton>
#include <QVBoxLayout>

#include <kwidgetsaddons_version.h>
#include <KGuiItem>
#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>

static const char HTML_HEADER[] =
    "<html>"
    "\n<head>"
    "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>"
    "\n<style type=\"text/css\">"
    "\nbody {"
    "\n   font-family:arial;"
    "\n}"
    "\n.property {"
    "\n   font-style:italic;"
    "\n}"
    "\nth {"
    "\n   font-weight:bold;"
    "\n   text-align:left;"
    "\n   background-color:#F0F0F0;"
    "\n}"
    "\n</style>"
    "\n</head>"
    "\n<body>";

static const char HTML_FOOTER[] =
    "\n</body>"
    "\n</html>";

static const char XML_HEADER[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";

ElementListEntry::ElementListEntry(Element *element)
    : QListWidgetItem()
{
    m_atomicNum = element->dataAsVariant(ChemicalDataObject::atomicNumber).toInt();
    m_name = element->dataAsString(ChemicalDataObject::name);
    m_element = element;

    setText(m_name);
}

ElementListEntry::~ElementListEntry() = default;

PropertyListEntry::PropertyListEntry(const QString &name, ChemicalDataObject::BlueObelisk type)
    : QListWidgetItem()
{
    setText(name);
    m_type = type;
}

PropertyListEntry::~PropertyListEntry() = default;

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
{
    qCDebug(KALZIUM_LOG) << "ExportDialog::ExportDialog";
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    auto user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ExportDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ExportDialog::reject);
    mainLayout->addWidget(buttonBox);
    qCDebug(KALZIUM_LOG) << "ExportDialog: setButtons";
    ui.setupUi(mainWidget);
    qCDebug(KALZIUM_LOG) << "ExportDialog: ui.setupUi(mainWidget)";
    KGuiItem::assign(user1Button, KGuiItem(i18n("OK")));
    qCDebug(KALZIUM_LOG) << "ExportDialog: setButtonGuiItem(User1, KGuiItem(i18n(\"OK\")));";
    ui.targetFile->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);
    qCDebug(KALZIUM_LOG) << "ui.targetFile->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);";

    setWindowTitle(i18nc("@title:window", "Export Chemical Data"));
    qCDebug(KALZIUM_LOG) << "ui.targetFile->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);";

    populateElementList();
    qCDebug(KALZIUM_LOG) << "ui.targetFile->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);";

    ui.formatList->addItem(QStringLiteral(".html (formatted html document)"), "html");
    ui.formatList->addItem(QStringLiteral(".xml (raw element data)"), "xml");
    ui.formatList->addItem(QStringLiteral(".csv (comma-separated data)"), "csv");
    qCDebug(KALZIUM_LOG) << "ui.formatList->addItem(...);";

    connect(user1Button, &QPushButton::clicked, this, &ExportDialog::slotOkClicked);
    qCDebug(KALZIUM_LOG) << "connect(user1Button, SIGNAL(clicked()), this, SLOT(slotOkClicked()));";
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &ExportDialog::slotHelpRequested);
    qCDebug(KALZIUM_LOG) << "KHelpClient::invokeHelp(QString(), \"kalzium\");";
}

ExportDialog::~ExportDialog()
{
    delete m_outputStream;
}

void ExportDialog::populateElementList()
{
    // Add descriptive headers
    auto header1 = new QListWidgetItem(i18n("Elements"));
    auto header2 = new QListWidgetItem(i18n("Properties"));
    header1->setFlags(Qt::ItemIsEnabled);
    header2->setFlags(Qt::ItemIsEnabled);
    QFont font;
    font.setBold(true);
    header1->setFont(font);
    header2->setFont(font);
    ui.elementListWidget->addItem(header1);
    ui.propertyListWidget->addItem(header2);

    // Add elements
    foreach (Element *element, KalziumDataObject::instance()->ElementList) {
        auto entry = new ElementListEntry(element);
        ui.elementListWidget->addItem(entry);
    }

    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Atomic Number"), ChemicalDataObject::atomicNumber));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Symbol"), ChemicalDataObject::symbol));
    // ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Name"), ChemicalDataObject::name));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Mass"), ChemicalDataObject::mass));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Exact Mass"), ChemicalDataObject::exactMass));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Ionization"), ChemicalDataObject::ionization));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Electron Affinity"), ChemicalDataObject::electronAffinity));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Electronegativity"), ChemicalDataObject::electronegativityPauling));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Covalent Radius"), ChemicalDataObject::radiusCovalent));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Van der Waals Radius"), ChemicalDataObject::radiusVDW));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Melting Point"), ChemicalDataObject::meltingpoint));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Boiling Point"), ChemicalDataObject::boilingpoint));
    ui.propertyListWidget->addItem(new PropertyListEntry(i18n("Family"), ChemicalDataObject::family));
}

void ExportDialog::slotOkClicked()
{
    QString format = ui.formatList->itemData(ui.formatList->currentIndex(), Qt::UserRole).toString();
    QString filename = ui.targetFile->url().toLocalFile();
    if (!filename.endsWith(format)) {
        filename += '.' + format;
    }
    QFile outputFile(filename);
    if (outputFile.exists()) {
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        if (KMessageBox::questionTwoActions(this,
#else
        if (KMessageBox::questionYesNo(this,
#endif
            i18n("File already exists. Do you want to overwrite it?"), QString(),
            KStandardGuiItem::overwrite(),
            KStandardGuiItem::cancel())
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
            == KMessageBox::SecondaryAction) {
#else
            == KMessageBox::No) {
#endif
            return;
        }
    }
    if (!outputFile.open(QIODevice::WriteOnly)) {
        KMessageBox::error(this, i18n("Could not open file for writing."));
        return;
    }

    delete m_outputStream;
    m_outputStream = new QTextStream(&outputFile);
    if (format == QLatin1String("html")) {
        exportToHtml();
    } else if (format == QLatin1String("xml")) {
        exportToXml();
    } else {
        exportToCsv();
    }

    // close the dialog
    done(0);
}

void ExportDialog::slotHelpRequested()
{
    KHelpClient::invokeHelp(QString(), QStringLiteral("kalzium"));
}

void ExportDialog::exportToHtml()
{
    *m_outputStream << HTML_HEADER << "<table>\n";
    foreach (QListWidgetItem *element, ui.elementListWidget->selectedItems()) {
        *m_outputStream << "<tr>\n<th colspan=\"2\">" << ((ElementListEntry *)element)->m_element->dataAsString(ChemicalDataObject::name) << "</th>\n</tr>\n";
        foreach (QListWidgetItem *property, ui.propertyListWidget->selectedItems()) {
            *m_outputStream << "<tr>\n<td class=\"property\">" << ((PropertyListEntry *)property)->text() << "</td>\n<td class=\"value\">"
                            << KalziumUtils::prettyUnit(((ElementListEntry *)element)->m_element, ((PropertyListEntry *)property)->m_type) << "</td>\n</tr>\n";
        }
    }
    *m_outputStream << "</table>\n" << HTML_FOOTER;
}

void ExportDialog::exportToXml()
{
    *m_outputStream << XML_HEADER << "<elements>\n";
    foreach (QListWidgetItem *element, ui.elementListWidget->selectedItems()) {
        *m_outputStream << "  <element name=\"" << ((ElementListEntry *)element)->m_element->dataAsString(ChemicalDataObject::name) << "\">\n";
        foreach (QListWidgetItem *property, ui.propertyListWidget->selectedItems()) {
            *m_outputStream << "    <property name=\"" << ((PropertyListEntry *)property)->text() << "\">"
                            << KalziumUtils::prettyUnit(((ElementListEntry *)element)->m_element, ((PropertyListEntry *)property)->m_type) << "</property>\n";
        }
        *m_outputStream << "  </element>\n";
    }
    *m_outputStream << "</elements>\n";
}

void ExportDialog::exportToCsv()
{
    *m_outputStream << "Name";
    foreach (QListWidgetItem *property, ui.propertyListWidget->selectedItems()) {
        *m_outputStream << ", \"" << ((PropertyListEntry *)property)->text() << "\"";
    }
    *m_outputStream << "\n";
    foreach (QListWidgetItem *element, ui.elementListWidget->selectedItems()) {
        *m_outputStream << "\"" << ((ElementListEntry *)element)->m_element->dataAsString(ChemicalDataObject::name) << "\"";
        foreach (QListWidgetItem *property, ui.propertyListWidget->selectedItems()) {
            *m_outputStream << ", \"" << KalziumUtils::prettyUnit(((ElementListEntry *)element)->m_element, ((PropertyListEntry *)property)->m_type) << "\"";
        }
        *m_outputStream << "\n";
    }
}

#include "moc_exportdialog.cpp"
