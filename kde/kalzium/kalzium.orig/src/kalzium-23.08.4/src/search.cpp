/*
    SPDX-FileCopyrightText: 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "search.h"

#include "kalziumdataobject.h"

Search::Search()
    : m_isActive(false)
    , m_searchKind(Search::SearchAll)
{
}

QString Search::searchText() const
{
    return m_searchText;
}

Search::SearchKind Search::searchKind() const
{
    return m_searchKind;
}

bool Search::isActive() const
{
    return m_isActive;
}

const QList<Element *> &Search::foundElements() const
{
    return m_foundElements;
}

bool Search::matches(Element *e) const
{
    return m_foundElements.contains(e);
}

bool Search::matches(int el) const
{
    Element *element = KalziumDataObject::instance()->element(el);
    return matches(element);
}

void Search::doSearch(const QString &text, SearchKind kind)
{
    m_isActive = true;
    m_searchText = text;
    m_searchKind = kind;
    QList<Element *> newresults;
    foreach (Element *e, KalziumDataObject::instance()->ElementList) {
        bool found = false;
        if (!found && e->dataAsString(ChemicalDataObject::name).contains(text, Qt::CaseInsensitive)) {
            found = true;
        }
        if (!found && e->dataAsString(ChemicalDataObject::symbol).contains(text, Qt::CaseInsensitive)) {
            found = true;
        }
        if (found) {
            newresults << e;
        }
    }
    if (newresults != m_foundElements) {
        m_foundElements = newresults;
        Q_EMIT searchChanged();
    }
}

void Search::resetSearch()
{
    if (!m_isActive) {
        return;
    }

    m_foundElements.clear();
    m_isActive = false;
    Q_EMIT searchReset();
}

#include "moc_search.cpp"
