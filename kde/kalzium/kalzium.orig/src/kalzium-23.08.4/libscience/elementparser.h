/*
    SPDX-FileCopyrightText: 2005 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ELEMENTPARSER_H
#define ELEMENTPARSER_H

#include <QList>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>

#include "science_export.h"

class Element;

/**
 * @author Carsten Niehaus <cniehaus@kde.org>
 */
class SCIENCE_EXPORT ElementSaxParser : public QXmlDefaultHandler
{
public:
    /**
     * Constructor
     */
    ElementSaxParser();
    ~ElementSaxParser() override;
    bool startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &attrs) override;

    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;

    bool characters(const QString &ch) override;

    QList<Element *> getElements() const;

private:
    /**
     * Looks up a name @p unitname. The valid names are
     * hard-coded in the C++ code, currently
     *   - bo:kelvin
     *   - bo:ev
     *   - bo:nm
     *   - bo:pm
     *   - bo:y
     *   - bo:s
     *   - bo:noUnit
     *
     * @return the BlueObeliskUnit of a ChemicalDataObject
     *   corresponding to @p unitname, or noUnit if the name
     *   doesn't match any of the known values.
     * @param unitname the attribute-text of the XML parsed
     */
    int unit(const QString &unitname) const;

    class Private;
    Private *const d;
};

#endif // ELEMENTPARSER_H
