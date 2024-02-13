/*
    SPDX-FileCopyrightText: 2005 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISOTOPEPARSER_H
#define ISOTOPEPARSER_H

#include <QList>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>

#include "science_export.h"

class Isotope;

/**
 * @author Carsten Niehaus <cniehaus@kde.org>
 */
class SCIENCE_EXPORT IsotopeParser : public QXmlDefaultHandler
{
public:
    /**
     * Constructor
     */
    IsotopeParser();
    ~IsotopeParser() override;
    bool startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &attrs) override;

    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;

    bool characters(const QString &ch) override;

    QList<Isotope *> getIsotopes() const;

private:
    class Private;
    Private *const d;
};
#endif // ISOTOPEPARSER_H
