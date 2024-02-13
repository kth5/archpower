/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publishdialog_p.h"
#include <Akonadi/AbstractEmailAddressSelectionDialog>
#include <Akonadi/EmailAddressSelectionDialog>
#include <KCalendarCore/Person>
#include <KEmailAddress>

#include <KLocalizedString>
#include <KPluginFactory>
#include <QPointer>
#include <QTreeView>

using namespace KCalendarCore;
using namespace Akonadi;

PublishDialogPrivate::PublishDialogPrivate(PublishDialog *qq)
    : QObject()
    , q(qq)
{
}

void PublishDialogPrivate::addItem()
{
    mUI.mNameLineEdit->setEnabled(true);
    mUI.mEmailLineEdit->setEnabled(true);
    QListWidgetItem *item = mUI.mListWidget->currentItem();
    if (item) {
        if (!item->text().isEmpty()) {
            item = new QListWidgetItem(mUI.mListWidget);
            mUI.mListWidget->addItem(item);
        }
    } else {
        item = new QListWidgetItem(mUI.mListWidget);
        mUI.mListWidget->addItem(item);
    }

    item->setSelected(true);
    mUI.mNameLineEdit->setPlaceholderText(i18n("(EmptyName)"));
    mUI.mEmailLineEdit->setPlaceholderText(i18n("(EmptyEmail)"));
    mUI.mListWidget->setCurrentItem(item);
    mUI.mRemove->setEnabled(true);
}

void PublishDialogPrivate::removeItem()
{
    if (mUI.mListWidget->selectedItems().isEmpty()) {
        return;
    }
    QListWidgetItem *item = mUI.mListWidget->selectedItems().at(0);

    int row = mUI.mListWidget->row(item);
    mUI.mListWidget->takeItem(row);

    if (!mUI.mListWidget->count()) {
        mUI.mNameLineEdit->setText(QString());
        mUI.mNameLineEdit->setEnabled(false);
        mUI.mEmailLineEdit->setText(QString());
        mUI.mEmailLineEdit->setEnabled(false);
        mUI.mRemove->setEnabled(false);
        return;
    }
    if (row > 0) {
        row--;
    }

    mUI.mListWidget->setCurrentRow(row);
}

void PublishDialogPrivate::insertAddresses(const KContacts::Addressee::List &list)
{
    for (const KContacts::Addressee &contact : list) {
        mUI.mNameLineEdit->setEnabled(true);
        mUI.mEmailLineEdit->setEnabled(true);
        auto item = new QListWidgetItem(mUI.mListWidget);
        item->setSelected(true);
        mUI.mNameLineEdit->setText(contact.name());
        mUI.mEmailLineEdit->setText(contact.preferredEmail());
        mUI.mListWidget->addItem(item);
    }
}

void PublishDialogPrivate::openAddressbook()
{
    QPointer<Akonadi::AbstractEmailAddressSelectionDialog> dialog;

    const KPluginMetaData editWidgetPlugin(QStringLiteral("pim6/akonadi/emailaddressselectionldapdialogplugin"));

    const auto result = KPluginFactory::instantiatePlugin<Akonadi::AbstractEmailAddressSelectionDialog>(editWidgetPlugin, q);
    if (result) {
        dialog = result.plugin;
    } else {
        dialog = new Akonadi::EmailAddressSelectionDialog(q);
    }

    dialog->view()->view()->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(dialog.data(), &Akonadi::AbstractEmailAddressSelectionDialog::insertAddresses, this, &PublishDialogPrivate::insertAddresses);
    if (dialog->exec() == QDialog::Accepted) {
        const Akonadi::EmailAddressSelection::List selections = dialog->selectedAddresses();
        if (!selections.isEmpty()) {
            for (const Akonadi::EmailAddressSelection &selection : selections) {
                mUI.mNameLineEdit->setEnabled(true);
                mUI.mEmailLineEdit->setEnabled(true);
                auto item = new QListWidgetItem(mUI.mListWidget);
                item->setSelected(true);
                mUI.mNameLineEdit->setText(selection.name());
                mUI.mEmailLineEdit->setText(KEmailAddress::extractEmailAddress(KEmailAddress::normalizeAddressesAndEncodeIdn(selection.email())));
                mUI.mListWidget->addItem(item);
            }

            mUI.mRemove->setEnabled(true);
        }
    }
    delete dialog;
}

void PublishDialogPrivate::updateItem()
{
    if (mUI.mListWidget->selectedItems().isEmpty()) {
        return;
    }

    Person person(mUI.mNameLineEdit->text(), mUI.mEmailLineEdit->text());
    QListWidgetItem *item = mUI.mListWidget->selectedItems().at(0);
    item->setText(person.fullName());
}

void PublishDialogPrivate::updateInput()
{
    if (mUI.mListWidget->selectedItems().isEmpty()) {
        return;
    }

    mUI.mNameLineEdit->setEnabled(true);
    mUI.mEmailLineEdit->setEnabled(true);

    QListWidgetItem *item = mUI.mListWidget->selectedItems().at(0);
    QString mail;
    QString name;
    KEmailAddress::extractEmailAddressAndName(item->text(), mail, name);
    mUI.mNameLineEdit->setText(name);
    mUI.mEmailLineEdit->setText(mail);
}

#include "moc_publishdialog_p.cpp"
