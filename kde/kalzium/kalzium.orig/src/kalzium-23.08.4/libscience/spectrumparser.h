/*
    SPDX-FileCopyrightText: 2005 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPECTRUMPARSER_H
#define SPECTRUMPARSER_H

#include "science_export.h"

#include <QXmlAttributes>
#include <QXmlDefaultHandler>

#include "spectrum.h"

#include "chemicaldataobject.h"

class Spectrum;

/**
 * @author Carsten Niehaus <cniehaus@kde.org>
 */
class SCIENCE_EXPORT SpectrumParser : public QXmlDefaultHandler
{
public:
    /**
     * Constructor
     */
    SpectrumParser();
    ~SpectrumParser() override;
    bool startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &attrs) override;

    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;

    bool characters(const QString &ch) override;

    QList<Spectrum *> getSpectrums() const;

private:
    QString currentElementID;

private:
    class Private;
    Private *const d;
};

#endif // SPECTRUMPARSER_H
