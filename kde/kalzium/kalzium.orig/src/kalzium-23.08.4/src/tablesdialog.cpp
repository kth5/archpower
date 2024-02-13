/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007 Ian Monroe <ian@monroe.nu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tablesdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QVBoxLayout>

#include <KLocalizedString>

TablesDialog::TablesDialog(QWidget *parent)
    : KPageDialog(parent)
{
    setFaceType(List);
    setStandardButtons(QDialogButtonBox::Close);

    // setButtons(Help | Close);
    // setDefaultButton(Close);

    createGreekSymbolTable();
    createNumbersTable();
}

void TablesDialog::createGreekSymbolTable()
{
    auto frame = new QWidget();
    KPageWidgetItem *item = addPage(frame, i18n("Greek alphabet"));
    item->setHeader(i18n("Greek alphabet"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("numbers")));
    auto layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);

    QTableWidget *table = new MyTableWidget(frame);
    table->verticalHeader()->hide();

    table->setColumnCount(3);
    table->setRowCount(24);
    table->setHorizontalHeaderLabels(
        QStringList() << i18n("Uppercase") << i18n("Lowercase")
                      << i18nc("The name of the greek letter in your language. For example 'Alpha' for the first letter. ", "Name"));

    layout->addWidget(table);
    table->setItem(0, 0, new MyWidgetItem(QString(QChar(913)))); // capital Alpha
    table->setItem(1, 0, new MyWidgetItem(QString(QChar(914))));
    table->setItem(2, 0, new MyWidgetItem(QString(QChar(915))));
    table->setItem(3, 0, new MyWidgetItem(QString(QChar(916))));
    table->setItem(4, 0, new MyWidgetItem(QString(QChar(917))));
    table->setItem(5, 0, new MyWidgetItem(QString(QChar(918))));
    table->setItem(6, 0, new MyWidgetItem(QString(QChar(919))));
    table->setItem(7, 0, new MyWidgetItem(QString(QChar(920))));
    table->setItem(8, 0, new MyWidgetItem(QString(QChar(921))));
    table->setItem(9, 0, new MyWidgetItem(QString(QChar(922))));
    table->setItem(10, 0, new MyWidgetItem(QString(QChar(923))));
    table->setItem(11, 0, new MyWidgetItem(QString(QChar(924))));
    table->setItem(12, 0, new MyWidgetItem(QString(QChar(925))));
    table->setItem(13, 0, new MyWidgetItem(QString(QChar(926))));
    table->setItem(14, 0, new MyWidgetItem(QString(QChar(927))));
    table->setItem(15, 0, new MyWidgetItem(QString(QChar(928))));
    table->setItem(16, 0, new MyWidgetItem(QString(QChar(929))));
    table->setItem(17, 0, new MyWidgetItem(QString(QChar(931))));
    table->setItem(18, 0, new MyWidgetItem(QString(QChar(932))));
    table->setItem(19, 0, new MyWidgetItem(QString(QChar(933))));
    table->setItem(20, 0, new MyWidgetItem(QString(QChar(934))));
    table->setItem(21, 0, new MyWidgetItem(QString(QChar(935))));
    table->setItem(22, 0, new MyWidgetItem(QString(QChar(936))));
    table->setItem(23, 0, new MyWidgetItem(QString(QChar(937))));

    // small letters
    table->setItem(0, 1, new MyWidgetItem(QString(QChar(945)))); // small alpha
    table->setItem(1, 1, new MyWidgetItem(QString(QChar(946))));
    table->setItem(2, 1, new MyWidgetItem(QString(QChar(947))));
    table->setItem(3, 1, new MyWidgetItem(QString(QChar(948))));
    table->setItem(4, 1, new MyWidgetItem(QString(QChar(949))));
    table->setItem(5, 1, new MyWidgetItem(QString(QChar(950))));
    table->setItem(6, 1, new MyWidgetItem(QString(QChar(951))));
    table->setItem(7, 1, new MyWidgetItem(QString(QChar(952))));
    table->setItem(8, 1, new MyWidgetItem(QString(QChar(953))));
    table->setItem(9, 1, new MyWidgetItem(QString(QChar(954))));
    table->setItem(10, 1, new MyWidgetItem(QString(QChar(955))));
    table->setItem(11, 1, new MyWidgetItem(QString(QChar(956))));
    table->setItem(12, 1, new MyWidgetItem(QString(QChar(957))));
    table->setItem(13, 1, new MyWidgetItem(QString(QChar(958))));
    table->setItem(14, 1, new MyWidgetItem(QString(QChar(959))));
    table->setItem(15, 1, new MyWidgetItem(QString(QChar(960))));
    table->setItem(16, 1, new MyWidgetItem(QString(QChar(961))));
    // there are two greek letters for sigma
    table->setItem(17, 1, new MyWidgetItem(QString(QChar(962)) + ", " + QString(QChar(963))));
    table->setItem(18, 1, new MyWidgetItem(QString(QChar(964))));
    table->setItem(19, 1, new MyWidgetItem(QString(QChar(965))));
    table->setItem(20, 1, new MyWidgetItem(QString(QChar(966))));
    table->setItem(21, 1, new MyWidgetItem(QString(QChar(967))));
    table->setItem(22, 1, new MyWidgetItem(QString(QChar(968))));
    table->setItem(23, 1, new MyWidgetItem(QString(QChar(969))));

    // english names
    table->setItem(0, 2, new MyWidgetItem(i18n("alpha")));
    table->setItem(1, 2, new MyWidgetItem(i18n("beta")));
    table->setItem(2, 2, new MyWidgetItem(i18n("gamma")));
    table->setItem(3, 2, new MyWidgetItem(i18n("delta")));
    table->setItem(4, 2, new MyWidgetItem(i18n("epsilon")));
    table->setItem(5, 2, new MyWidgetItem(i18n("zeta")));
    table->setItem(6, 2, new MyWidgetItem(i18n("eta")));
    table->setItem(7, 2, new MyWidgetItem(i18n("theta")));
    table->setItem(8, 2, new MyWidgetItem(i18n("iota")));
    table->setItem(9, 2, new MyWidgetItem(i18n("kappa")));
    table->setItem(10, 2, new MyWidgetItem(i18n("lambda")));
    table->setItem(11, 2, new MyWidgetItem(i18n("mu")));
    table->setItem(12, 2, new MyWidgetItem(i18n("nu")));
    table->setItem(13, 2, new MyWidgetItem(i18n("xi")));
    table->setItem(14, 2, new MyWidgetItem(i18n("omicron")));
    table->setItem(15, 2, new MyWidgetItem(i18n("pi")));
    table->setItem(16, 2, new MyWidgetItem(i18n("rho")));
    table->setItem(17, 2, new MyWidgetItem(i18n("sigma")));
    table->setItem(18, 2, new MyWidgetItem(i18n("tau")));
    table->setItem(19, 2, new MyWidgetItem(i18n("upsilon")));
    table->setItem(20, 2, new MyWidgetItem(i18n("phi")));
    table->setItem(21, 2, new MyWidgetItem(i18n("chi")));
    table->setItem(22, 2, new MyWidgetItem(i18n("psi")));
    table->setItem(23, 2, new MyWidgetItem(i18n("omega")));

    table->resizeColumnsToContents();
    frame->setMinimumWidth(qMax(table->columnWidth(0) + table->columnWidth(1) + table->columnWidth(2), table->horizontalHeader()->sizeHint().width()) + 25);
}

void TablesDialog::createNumbersTable()
{
    auto frame = new QWidget();
    KPageWidgetItem *item = addPage(frame, i18n("Numbers"));
    item->setHeader(i18n("Numeric Prefixes and Roman Numerals"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("numbers")));
    auto layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);

    QTableWidget *table = new MyTableWidget(frame);
    table->verticalHeader()->hide();

    table->setColumnCount(3);
    table->setRowCount(28);
    table->setHorizontalHeaderLabels(QStringList() << i18n("Number") << i18nc("For example 'Mono' for 1 and 'Tri' for 3", "Prefix") << i18n("Roman Numerals"));

    layout->addWidget(table);

    table->setItem(0, 0, new MyWidgetItem(i18n("0.5")));
    table->setItem(1, 0, new MyWidgetItem(i18n("1")));
    table->setItem(2, 0, new MyWidgetItem(i18n("1.5")));
    table->setItem(3, 0, new MyWidgetItem(i18n("2")));
    table->setItem(4, 0, new MyWidgetItem(i18n("2.5")));
    table->setItem(5, 0, new MyWidgetItem(i18n("3")));
    table->setItem(6, 0, new MyWidgetItem(i18n("4")));
    table->setItem(7, 0, new MyWidgetItem(i18n("5")));
    table->setItem(8, 0, new MyWidgetItem(i18n("6")));
    table->setItem(9, 0, new MyWidgetItem(i18n("7")));
    table->setItem(10, 0, new MyWidgetItem(i18n("8")));
    table->setItem(11, 0, new MyWidgetItem(i18n("9")));
    table->setItem(12, 0, new MyWidgetItem(i18n("10")));
    table->setItem(13, 0, new MyWidgetItem(i18n("11")));
    table->setItem(14, 0, new MyWidgetItem(i18n("12")));
    table->setItem(15, 0, new MyWidgetItem(i18n("13")));
    table->setItem(16, 0, new MyWidgetItem(i18n("14")));
    table->setItem(17, 0, new MyWidgetItem(i18n("15")));
    table->setItem(18, 0, new MyWidgetItem(i18n("16")));
    table->setItem(19, 0, new MyWidgetItem(i18n("17")));
    table->setItem(20, 0, new MyWidgetItem(i18n("18")));
    table->setItem(21, 0, new MyWidgetItem(i18n("19")));
    table->setItem(22, 0, new MyWidgetItem(i18n("20")));
    table->setItem(23, 0, new MyWidgetItem(i18n("40")));
    table->setItem(24, 0, new MyWidgetItem(i18n("50")));
    table->setItem(25, 0, new MyWidgetItem(i18n("60")));
    table->setItem(26, 0, new MyWidgetItem(i18n("90")));
    table->setItem(27, 0, new MyWidgetItem(i18n("100")));

    // greek names of the numbers
    table->setItem(0, 1, new MyWidgetItem(QStringLiteral("hemi")));
    table->setItem(1, 1, new MyWidgetItem(QStringLiteral("mono")));
    table->setItem(2, 1, new MyWidgetItem(QStringLiteral("sesqui")));
    table->setItem(3, 1, new MyWidgetItem(QStringLiteral("di, bi")));
    table->setItem(4, 1, new MyWidgetItem(QStringLiteral("hemipenta")));
    table->setItem(5, 1, new MyWidgetItem(QStringLiteral("tri")));
    table->setItem(6, 1, new MyWidgetItem(QStringLiteral("tetra")));
    table->setItem(7, 1, new MyWidgetItem(QStringLiteral("penta")));
    table->setItem(8, 1, new MyWidgetItem(QStringLiteral("hexa")));
    table->setItem(9, 1, new MyWidgetItem(QStringLiteral("hepta")));
    table->setItem(10, 1, new MyWidgetItem(QStringLiteral("octa")));
    table->setItem(11, 1, new MyWidgetItem(QStringLiteral("nona, ennea")));
    table->setItem(12, 1, new MyWidgetItem(QStringLiteral("deca")));
    table->setItem(13, 1, new MyWidgetItem(QStringLiteral("hendeca, undeca")));
    table->setItem(14, 1, new MyWidgetItem(QStringLiteral("dodeca")));
    table->setItem(15, 1, new MyWidgetItem(QStringLiteral("trideca")));
    table->setItem(16, 1, new MyWidgetItem(QStringLiteral("tetradeca")));
    table->setItem(17, 1, new MyWidgetItem(QStringLiteral("pentadeca")));
    table->setItem(18, 1, new MyWidgetItem(QStringLiteral("hexadeca")));
    table->setItem(19, 1, new MyWidgetItem(QStringLiteral("heptadeca")));
    table->setItem(20, 1, new MyWidgetItem(QStringLiteral("octadeca")));
    table->setItem(21, 1, new MyWidgetItem(QStringLiteral("nonadeca")));
    table->setItem(22, 1, new MyWidgetItem(QStringLiteral("eicosa")));
    table->setItem(23, 1, new MyWidgetItem(QStringLiteral("tetraconta")));
    table->setItem(24, 1, new MyWidgetItem(QStringLiteral("pentaconta")));
    table->setItem(25, 1, new MyWidgetItem(QStringLiteral("hexaconta")));
    table->setItem(26, 1, new MyWidgetItem(QStringLiteral("nonaconta")));
    table->setItem(27, 1, new MyWidgetItem(QStringLiteral("hecta")));

    // roman symbols
    table->setItem(1, 2, new MyWidgetItem(QStringLiteral("I")));
    table->setItem(3, 2, new MyWidgetItem(QStringLiteral("II")));
    table->setItem(5, 2, new MyWidgetItem(QStringLiteral("III")));
    table->setItem(6, 2, new MyWidgetItem(QStringLiteral("IV")));
    table->setItem(7, 2, new MyWidgetItem(QStringLiteral("V")));
    table->setItem(8, 2, new MyWidgetItem(QStringLiteral("VI")));
    table->setItem(9, 2, new MyWidgetItem(QStringLiteral("VII")));
    table->setItem(10, 2, new MyWidgetItem(QStringLiteral("VIII")));
    table->setItem(11, 2, new MyWidgetItem(QStringLiteral("IX")));
    table->setItem(12, 2, new MyWidgetItem(QStringLiteral("X")));
    table->setItem(13, 2, new MyWidgetItem(QStringLiteral("XI")));
    table->setItem(14, 2, new MyWidgetItem(QStringLiteral("XII")));
    table->setItem(15, 2, new MyWidgetItem(QStringLiteral("XIII")));
    table->setItem(16, 2, new MyWidgetItem(QStringLiteral("XIV")));
    table->setItem(17, 2, new MyWidgetItem(QStringLiteral("XV")));
    table->setItem(18, 2, new MyWidgetItem(QStringLiteral("XVI")));
    table->setItem(19, 2, new MyWidgetItem(QStringLiteral("XVII")));
    table->setItem(20, 2, new MyWidgetItem(QStringLiteral("XVIII")));
    table->setItem(21, 2, new MyWidgetItem(QStringLiteral("XIV")));
    table->setItem(22, 2, new MyWidgetItem(QStringLiteral("XX")));
    table->setItem(23, 2, new MyWidgetItem(QStringLiteral("XL")));
    table->setItem(24, 2, new MyWidgetItem(QStringLiteral("L")));
    table->setItem(25, 2, new MyWidgetItem(QStringLiteral("LX")));
    table->setItem(26, 2, new MyWidgetItem(QStringLiteral("XC")));
    table->setItem(27, 2, new MyWidgetItem(QStringLiteral("C")));

    table->resizeColumnsToContents();
    frame->setMinimumWidth(qMax(table->columnWidth(0) + table->columnWidth(1) + table->columnWidth(2), table->horizontalHeader()->sizeHint().width()) + 25);
}

TablesDialog::~TablesDialog() = default;

MyTableWidget::MyTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
}

void MyTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    auto menu = QMenu((QWidget *)sender());
    menu.addAction(i18n("&Copy"), this, &MyTableWidget::copyToClipboard, QKeySequence(Qt::Key_C | Qt::CTRL));
    menu.exec(event->globalPos());
}

void MyTableWidget::copyToClipboard()
{
    QApplication::clipboard()->setText(currentItem()->data(QTableWidgetItem::Type).toString());
}

#include "moc_tablesdialog.cpp"
