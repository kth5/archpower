/*
    SPDX-FileCopyrightText: 2005, 2006, 2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotope.h"
#include "isotopeparser.h"
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 2) {
        std::cout << "Usage: isotopes <XML_FILE>\n";
        return 1;
    }

    auto parser = new IsotopeParser();
    QFile xmlFile(argv[1]);

    QXmlInputSource source(&xmlFile);
    QXmlSimpleReader reader;

    reader.setContentHandler(parser);
    reader.parse(source);

    const QList<Isotope *> v = parser->getIsotopes();

    qDebug() << "Found " << v.count() << " isotopes.";
    ;

    qDebug() << "As a test I am now issuing all isotopes with 50 nuclueons: ";

    for (Isotope *i : v) {
        if (i) {
            // X             if (i->nucleons() == 50 ) {
            // X                 qDebug() << "   Isotope of " << i->parentElementSymbol() << " with a mass of " << i->mass();
            // X                 qDebug() << "       Halflife: " << i->halflife() << i->halflifeUnit( );
            // X             }
            if (i->parentElementSymbol() == QLatin1String("Ti")) {
                qDebug() << "   Isotope of " << i->parentElementSymbol() << " with a mass of " << i->mass();
                qDebug() << "       Halflife: " << i->halflife() << i->halflifeUnit();
            }
        }
    }

    delete parser;
    qDeleteAll(v);

    return 0;
}
