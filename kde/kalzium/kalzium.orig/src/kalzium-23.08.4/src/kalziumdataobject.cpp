/*
    SPDX-FileCopyrightText: 2005, 2006, 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalziumdataobject.h"

#include <elementparser.h>
#include <isotope.h>
#include <isotopeparser.h>
#include <spectrumparser.h>

#include "kalzium_debug.h"
#include <QCoreApplication>
#include <QFile>
#include <QPainter>
#include <QStandardPaths>
#include <QSvgRenderer>

#include <KUnitConversion/Converter>

struct StaticKalziumDataObject {
    KalziumDataObject kdo;
};

Q_GLOBAL_STATIC(StaticKalziumDataObject, s_kdo)

KalziumDataObject *KalziumDataObject::instance()
{
    return &s_kdo->kdo;
}

KalziumDataObject::KalziumDataObject()
{
    // reading elements
    auto parser = new ElementSaxParser();

    QFile xmlFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/elements.xml")));
    QXmlInputSource source(&xmlFile);
    QXmlSimpleReader reader;

    reader.setContentHandler(parser);
    reader.parse(source);

    ElementList = parser->getElements();

    // we don't need parser anymore, let's free its memory
    delete parser;

    // read the spectra
    auto spectrumparser = new SpectrumParser();

    QFile xmlSpFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/spectra.xml")));
    QXmlInputSource spsource(&xmlSpFile);
    QXmlSimpleReader sp_reader;

    sp_reader.setContentHandler(spectrumparser);
    sp_reader.parse(spsource);

    m_spectra = spectrumparser->getSpectrums();

    // we don't need spectrumparser anymore, let's free its memory
    delete spectrumparser;

    // reading isotopes
    auto isoparser = new IsotopeParser();

    QFile xmlIsoFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/isotopes.xml")));
    QXmlInputSource isosource(&xmlIsoFile);
    QXmlSimpleReader isoreader;

    isoreader.setContentHandler(isoparser);
    isoreader.parse(isosource);

    const QList<Isotope *> isotopes = isoparser->getIsotopes();

    // we don't need isoparser anymore, let's free its memory
    delete isoparser;

    for (Isotope *iso : isotopes) {
        int num = iso->parentElementNumber();
        if (m_isotopes.contains(num)) {
            m_isotopes[num].append(iso);
        } else {
            QList<Isotope *> newlist;
            newlist.append(iso);
            m_isotopes.insert(num, newlist);
        }
    }

    // cache it
    m_numOfElements = ElementList.count();

    qAddPostRoutine(KalziumDataObject::cleanup);
}

KalziumDataObject::~KalziumDataObject()
{
    // Delete all elements
    qDeleteAll(ElementList);

    // Delete all isotopes
    QHashIterator<int, QList<Isotope *>> i(m_isotopes);
    while (i.hasNext()) {
        i.next();
        qDeleteAll(i.value());
    }

    // Delete the spectra
    qDeleteAll(m_spectra);
}

Element *KalziumDataObject::element(int number)
{
    // checking that we are requesting a valid element
    if ((number <= 0) || (number > m_numOfElements))
        return nullptr;
    return ElementList[number - 1];
}

QString KalziumDataObject::unitAsString(const int unit) const
{
    return KUnitConversion::Converter().unit(KUnitConversion::UnitId(unit)).symbol();
}

QPixmap KalziumDataObject::pixmap(int number)
{
    // checking that we are requesting a valid element
    if ((number <= 0) || (number > m_numOfElements))
        return {};
    if (PixmapList.isEmpty())
        loadIconSet();
    return PixmapList[number - 1];
}

QList<Isotope *> KalziumDataObject::isotopes(Element *element)
{
    return isotopes(element->dataAsVariant(ChemicalDataObject::atomicNumber).toInt());
}

QList<Isotope *> KalziumDataObject::isotopes(int number)
{
    return m_isotopes.contains(number) ? m_isotopes.value(number) : QList<Isotope *>();
}

Spectrum *KalziumDataObject::spectrum(int number)
{
    foreach (Spectrum *s, m_spectra) {
        if (s->parentElementNumber() == number) {
            return s;
        }
    }

    return nullptr;
}

void KalziumDataObject::setSearch(Search *srch)
{
    m_search = srch;
}

Search *KalziumDataObject::search() const
{
    return m_search;
}

void KalziumDataObject::cleanup()
{
    KalziumDataObject::instance()->cleanPixmaps();
}

void KalziumDataObject::loadIconSet()
{
    // FIXME in case we ever get more than one theme we need
    // a settings-dialog where we can select the different iconsets...
    const QString setname = QStringLiteral("school");
    const QString pathname = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "data/iconsets/" + setname + '/', QStandardPaths::LocateDirectory);
    QSvgRenderer renderer;

    for (int i = 0; i < m_numOfElements; ++i) {
        const QString filename = pathname + QString::number(i + 1) + ".svg";

        renderer.load(filename);
        QPixmap pix(40, 40);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        renderer.render(&p);

        Element *e = ElementList.at(i);
        const QString esymbol = e->dataAsString(ChemicalDataObject::symbol);
        p.drawText(0, 0, 40, 40, Qt::AlignCenter | Qt::TextWordWrap, esymbol);
        p.end();

        PixmapList << pix;
    }
}

void KalziumDataObject::cleanPixmaps()
{
    PixmapList.clear();
}
