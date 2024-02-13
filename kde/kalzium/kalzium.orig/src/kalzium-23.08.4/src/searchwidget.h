/*
    SPDX-FileCopyrightText: 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMSEARCHWIDGET_H
#define KALZIUMSEARCHWIDGET_H

#include <QWidget>

class KLineEdit;
class QTimer;

/**
 * A small widget to control the Search kept in KalziumDataObject.
 *
 * @author Pino Toscano
 */
class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Construct a new empty search.
     */
    explicit SearchWidget(QWidget *parent = nullptr);

    ~SearchWidget() override;

    void giveFocus();

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void slotReturnPressed();
    void doSearch();

private:
    KLineEdit *const m_searchLine;
    QTimer *m_timer = nullptr;
};

#endif // KALZIUMSEARCHWIDGET_H
