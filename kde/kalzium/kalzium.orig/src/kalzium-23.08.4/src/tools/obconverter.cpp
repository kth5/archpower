/*
    SPDX-FileCopyrightText: 2007, 2008 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2006 Georges Khaznadar <georgesk@debian.org>
    SPDX-FileCopyrightText: 2006, 2007 Jerome Pansanel <j.pansanel@pansanel.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "obconverter.h"

// Qt includes
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>

// KDE includes
#include <kwidgetsaddons_version.h>
#include <KGuiItem>
#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>
using namespace std;
using namespace OpenBabel;

KOpenBabel::KOpenBabel(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "OpenBabel Frontend"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KOpenBabel::reject);
    connect(user1Button, &QPushButton::clicked, this, &KOpenBabel::slotConvert);
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &KOpenBabel::slotHelpRequested);

    user1Button->setDefault(true);

    OBConvObject = new OBConversion();

    ui.setupUi(mainWidget);

    KGuiItem::assign(user1Button, KGuiItem(i18n("Convert")));
    mainLayout->addWidget(buttonBox);

    setupWindow();
}

KOpenBabel::~KOpenBabel()
{
    delete OBConvObject;
    OBConvObject = nullptr;
}

void KOpenBabel::setupWindow()
{
    // Set multiple selection possible
    ui.FileListView->setSelectionMode(QAbstractItemView::SelectionMode(3));

    // Creating the main layout
    QStringList InputType;
    vector<string> InputFormat = OBConvObject->GetSupportedInputFormat();
    for (vector<string>::iterator it = InputFormat.begin(); it != InputFormat.end(); ++it) {
        InputType << QString((*it).c_str());
    }
    ui.InputTypeComboBox->addItems(InputType);

    QStringList OutputType;
    vector<string> OutputFormat = OBConvObject->GetSupportedOutputFormat();
    for (vector<string>::iterator it = OutputFormat.begin(); it != OutputFormat.end(); ++it) {
        OutputType << QString((*it).c_str());
    }
    ui.OutputTypeComboBox->addItems(OutputType);

    // Create connection
    connect(ui.addFileButton, &QAbstractButton::clicked, this, &KOpenBabel::slotAddFile);

    connect(ui.deleteFileButton, &QAbstractButton::clicked, this, &KOpenBabel::slotDeleteFile);

    connect(ui.selectAllFileButton, &QAbstractButton::clicked, this, &KOpenBabel::slotSelectAll);

    connect(ui.FileListView, &QListWidget::itemSelectionChanged, this, &KOpenBabel::slotGuessInput);
}

void KOpenBabel::slotAddFile()
{
    QStringList InputType;
    vector<string> InputFormat = OBConvObject->GetSupportedInputFormat();
    for (vector<string>::iterator it = InputFormat.begin(); it != InputFormat.end(); ++it) {
        InputType << QString((*it).c_str());
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // InputType is now something like this:                                                                                    //
    // "acr -- ACR format [Read-only]", "alc -- Alchemy format", "arc -- Accelrys/MSI Biosym/Insight II CAR format [Read-only]" //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    QStringList tmpList = InputType;
    tmpList.replaceInStrings(QRegularExpression("^"), QStringLiteral("*."));
    tmpList.replaceInStrings(QRegularExpression("(.*) -- (.*)"), "\\2(\\1)");
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // tmpList is now something like this:                                                                                      //
    // "ACR format [Read-only] (*.acr)", "Alchemy format (*.alc)"                                                                   //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const QList<QUrl> fl = QFileDialog::getOpenFileUrls(
        this,
        i18n("Open Molecule File"),
        QUrl(),
        i18n("All Files") + QStringLiteral("(*);;") + tmpList.join(QLatin1String(";;")) // add all possible extensions like "*.cml *.mol"
    );

    for (const QUrl &u : fl) {
        new QListWidgetItem(u.toDisplayString(), ui.FileListView);
    }
}

void KOpenBabel::slotSelectAll()
{
    ui.FileListView->selectAll();
}

void KOpenBabel::slotDeleteFile()
{
    const QList<QListWidgetItem *> p = ui.FileListView->selectedItems();
    for (QListWidgetItem *item : p) {
        delete item;
    }
}

void KOpenBabel::slotGuessInput()
{
    const QList<QListWidgetItem *> p = ui.FileListView->selectedItems();
    bool first = true;
    QString suffix;
    if (p.count()) {
        for (QListWidgetItem *item : p) {
            if (first) {
                first = false;
                suffix = item->text().remove(QRegularExpression("^.*\\."));
            } else {
                if (item->text().remove(QRegularExpression("^.*\\.")) == suffix) {
                    continue;
                } else {
                    // All the file types are not same, set type to default
                    ui.InputTypeComboBox->setCurrentIndex(0);
                    return;
                }
            }
        }
    }
    for (int i = 0; i < ui.InputTypeComboBox->count(); ++i) {
        if (ui.InputTypeComboBox->itemText(i).indexOf(QRegularExpression('^' + suffix + " --")) >= 0) {
            ui.InputTypeComboBox->setCurrentIndex(i);
            return;
        }
    }
    // The suffix has not been found, set type to default
    ui.InputTypeComboBox->setCurrentIndex(0);
}

void KOpenBabel::slotConvert()
{
    QString iformat = ui.InputTypeComboBox->currentText();
    QString oformat = ui.OutputTypeComboBox->currentText();
    iformat = iformat.remove(QRegularExpression(" --.*"));
    oformat = oformat.remove(QRegularExpression(" --.*"));

    const QList<QListWidgetItem *> p = ui.FileListView->selectedItems();
    if (p.isEmpty()) {
        KMessageBox::error(this, i18n("You must select some files first."), i18n("No files selected"));
        return;
    }
    QListIterator<QListWidgetItem *> it(p);
    QStringList cmdList; // Full command
    QVector<QStringList> cmdArgList; // Arguments only
    foreach (QListWidgetItem *item, p) {
        QString ifname = QUrl(item->text()).toLocalFile();
        QString ofname = ifname;
        ofname = ofname.remove(QRegularExpression("\\.([^\\.]*$)"));
        ofname = ofname + QStringLiteral(".") + oformat;

        bool proceed = true;

        if (QFile::exists(ofname)) {
            // something named ofname already exists
            switch (KMessageBox::warningContinueCancel(this,
                                                       i18n("The file %1 already exists. Do you want to overwrite if possible?", ofname),
                                                       i18n("The File %1 Already Exists -- KOpenBabel", ofname))) {
            case KMessageBox::Cancel:
                proceed = false;
                break;
            default:
                break;
            }
        }
        if (proceed) {
            QStringList arguments;
            arguments << QStringLiteral("-i") + iformat << ifname << QStringLiteral("-o") + oformat << ofname;
            cmdArgList.append(arguments);
            cmdList.append(QStringLiteral("babel ") + arguments.join(QStringLiteral(" ")));
        }
    }
    if (cmdArgList.count() > 0) {
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        switch (KMessageBox::questionTwoActions(this,
#else
        switch (KMessageBox::questionYesNo(this,
#endif
            cmdList.join(QStringLiteral("\n")),
            i18n("Is it okay to run these commands? -- KOpenBabel"),
            KGuiItem(i18n("Convert")),
            KStandardGuiItem::cancel())) {
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        case KMessageBox::PrimaryAction:
#else
        case KMessageBox::Yes:
#endif
            foreach (const QStringList &s, cmdArgList) {
                QProcess::startDetached(QStringLiteral("babel"), s);
            }
            break;
        default:
            break;
        }
    }
}

void KOpenBabel::slotHelpRequested()
{
    KHelpClient::invokeHelp(QStringLiteral("commands"), QStringLiteral("kalzium"));
}

void KOpenBabel::addFile(const QString &filename)
{
    ui.FileListView->addItem(filename);
}

#include "moc_obconverter.cpp"
