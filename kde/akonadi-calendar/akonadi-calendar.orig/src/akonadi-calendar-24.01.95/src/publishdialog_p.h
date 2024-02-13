/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "publishdialog.h"
#include "ui_publishdialog_base.h"
#include <KContacts/Addressee>

namespace Akonadi
{
class PublishDialogPrivate : public QObject
{
    Q_OBJECT
public:
    explicit PublishDialogPrivate(PublishDialog *q);
    void insertAddresses(const KContacts::Addressee::List &list);

public Q_SLOTS:
    void addItem();
    void removeItem();
    void openAddressbook();
    void updateItem();
    void updateInput();

public:
    Ui::PublishDialog_base mUI;
    PublishDialog *q = nullptr;
};
}
