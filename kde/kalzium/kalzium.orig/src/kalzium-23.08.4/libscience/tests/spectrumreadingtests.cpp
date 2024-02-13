/*
    SPDX-FileCopyrightText: 2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "spectrum.h"
#include "spectrumparser.h"
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 2) {
        std::cout << "Usage: spectrum <XML_FILE>\n";
        return 1;
    }

    auto parser = new SpectrumParser();
    QFile xmlFile(argv[1]);

    QXmlInputSource source(&xmlFile);
    QXmlSimpleReader reader;

    reader.setContentHandler(parser);
    reader.parse(source);

    const QList<Spectrum *> v = parser->getSpectrums();

    qDebug() << "Found " << v.count() << " isotopes.";

    for (Spectrum *s : v) {
        if (s) {
            qDebug() << "Element:  " << s->parentElementNumber();
            for (Spectrum::peak *p : s->peaklist()) {
                qDebug() << "         Peak: " << p->wavelength;
            }
        }
    }

    delete parser;
    qDeleteAll(v);

    return 0;
}
