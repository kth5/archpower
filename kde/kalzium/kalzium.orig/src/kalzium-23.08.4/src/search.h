/*
    SPDX-FileCopyrightText: 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMSEARCH_H
#define KALZIUMSEARCH_H

#include <QList>
#include <QObject>

#include "element.h"
// class Element;

/**
 * Represent a search.
 *
 * @author Pino Toscano
 */
class Search : public QObject
{
    Q_OBJECT

public:
    /**
     * The kind of search we can perform
     */
    enum SearchKind { SearchByName = 0x01, SearchBySymbol = 0x02, SearchAll = 0xFF };

    /**
     * Construct a new empty search.
     */
    Search();

    /**
     * @return the search text
     */
    QString searchText() const;

    /**
     * @return the kind of search
     */
    SearchKind searchKind() const;

    /**
     * is the current Search active?
     * @return whether this search is active
     */
    bool isActive() const;

    /**
     * @return the found elements
     */
    const QList<Element *> &foundElements() const;

    /**
     * @return whether the element @p el matches the search
     */
    bool matches(Element *el) const;

    /**
     * @return whether the element @p el matches the search
     * overloaded function to use direct the element number.
     */
    bool matches(int el) const;

public Q_SLOTS:
    /**
     * Search the @p text by looking at the element using the
     * specified @p kind
     */
    void doSearch(const QString &text, SearchKind kind);
    /**
     * Reset the current search (and put it not active).
     */
    void resetSearch();

Q_SIGNALS:
    /**
     * The current search has changed (ie the found elements have
     * changed)
     */
    void searchChanged();
    /**
     * The current search has been reset.
     */
    void searchReset();

private:
    bool m_isActive = false;

    QString m_searchText;
    SearchKind m_searchKind;

    QList<Element *> m_foundElements;
};

#endif // KALZIUMSEARCH_H
