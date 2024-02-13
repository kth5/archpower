/*
    SPDX-FileCopyrightText: 2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "elementparser.h"

#include "chemicaldataobject.h"
#include "element.h"


#include <KLocalizedString>
#include <KUnitConversion/Converter>

class ElementSaxParser::Private
{
public:
    Private()
        : currentUnit(KUnitConversion::NoUnit)
        , currentElement(nullptr)
        , inElement(false)
        , inName(false)
        , inMass(false)
        , inExactMass(false)
        , inAtomicNumber(false)
        , inSymbol(false)
        , inIonization(false)
        , inElectronAffinity(false)
        , inElectronegativityPauling(false)
        , inRadiusCovalent(false)
        , inRadiusVDW(false)
        , inBoilingPoint(false)
        , inMeltingPoint(false)
        , inPeriodTableBlock(false)
        , inNameOrigin(false)
        , inDiscoveryDate(false)
        , inDiscoverers(false)
        , inPeriod(false)
        , inCrystalstructure(false)
        , inAcidicbehaviour(false)
        , inFamily(false)
        , inGroup(false)
        , inElectronicconfiguration(false)
        , inDangerSymbol(false)
        , inRPhrase(false)
        , inSPhrase(false)
        , inCountry(false)
        , inOxidation(false)
    {
    }

    ~Private()
    {
        delete currentElement;
        // qDeleteAll(elements);
    }

    ChemicalDataObject currentDataObject;
    int currentUnit; // KUnitConversion::UnitId
    Element *currentElement = nullptr;

    QList<Element *> elements;

    bool inElement;
    bool inName;
    bool inMass;
    bool inExactMass;
    bool inAtomicNumber;
    bool inSymbol;
    bool inIonization;
    bool inElectronAffinity;
    bool inElectronegativityPauling;
    bool inRadiusCovalent;
    bool inRadiusVDW;
    bool inBoilingPoint;
    bool inMeltingPoint;
    bool inPeriodTableBlock;
    bool inNameOrigin;
    bool inDiscoveryDate;
    bool inDiscoverers;
    bool inPeriod;
    bool inCrystalstructure;
    bool inAcidicbehaviour;
    bool inFamily;
    bool inGroup;
    bool inElectronicconfiguration;
    bool inDangerSymbol;
    bool inRPhrase;
    bool inSPhrase;
    bool inCountry;
    bool inOxidation;
};

ElementSaxParser::ElementSaxParser()
    : QXmlDefaultHandler()
    , d(new Private)
{
}

ElementSaxParser::~ElementSaxParser()
{
    delete d;
}

bool ElementSaxParser::startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &attrs)
{
    if (localName == QLatin1String("atom")) {
        d->currentElement = new Element();
        d->inElement = true;
    } else if ((d->inElement && localName == QLatin1String("scalar")) || localName == QLatin1String("array")) {
        for (int i = 0; i < attrs.length(); ++i) {
            if (attrs.localName(i) == QLatin1String("units")) {
                //                 qCDebug(KALZIUM_LIBSCIENCE_LOG) << "value of the unit: " << attrs.value(i);
                d->currentUnit = unit(attrs.value(i));
                //                 qCDebug(KALZIUM_LIBSCIENCE_LOG) << "Took " << d->currentUnit;
                continue;
            }

            if (attrs.value(i) == QLatin1String("bo:atomicNumber")) {
                d->inAtomicNumber = true;
            } else if (attrs.value(i) == QLatin1String("bo:mass")) {
                d->inMass = true;
            } else if (attrs.value(i) == QLatin1String("bo:exactMass")) {
                d->inExactMass = true;
            } else if (attrs.value(i) == QLatin1String("bo:ionization")) {
                d->inIonization = true;
            } else if (attrs.value(i) == QLatin1String("bo:electronAffinity")) {
                d->inElectronAffinity = true;
            } else if (attrs.value(i) == QLatin1String("bo:electronegativityPauling")) {
                d->inElectronegativityPauling = true;
            } else if (attrs.value(i) == QLatin1String("bo:radiusCovalent")) {
                d->inRadiusCovalent = true;
            } else if (attrs.value(i) == QLatin1String("bo:radiusVDW")) {
                d->inRadiusVDW = true;
            } else if (attrs.value(i) == QLatin1String("bo:meltingpoint")) {
                d->inMeltingPoint = true;
            } else if (attrs.value(i) == QLatin1String("bo:boilingpoint")) {
                d->inBoilingPoint = true;
            } else if (attrs.value(i) == QLatin1String("bo:periodTableBlock")) {
                d->inPeriodTableBlock = true;
            } else if (attrs.value(i) == QLatin1String("bo:nameOrigin")) {
                d->inNameOrigin = true;
            } else if (attrs.value(i) == QLatin1String("bo:discoveryDate")) {
                d->inDiscoveryDate = true;
            } else if (attrs.value(i) == QLatin1String("bo:discoverers")) {
                d->inDiscoverers = true;
            } else if (attrs.value(i) == QLatin1String("bo:discoveryCountry")) {
                d->inCountry = true;
            } else if (attrs.value(i) == QLatin1String("bo:period")) {
                d->inPeriod = true;
            } else if (attrs.value(i) == QLatin1String("bo:crystalstructure")) {
                d->inCrystalstructure = true;
            } else if (attrs.value(i) == QLatin1String("bo:acidicbehaviour")) {
                d->inAcidicbehaviour = true;
            } else if (attrs.value(i) == QLatin1String("bo:family")) {
                d->inFamily = true;
            } else if (attrs.value(i) == QLatin1String("bo:group")) {
                d->inGroup = true;
            } else if (attrs.value(i) == QLatin1String("bo:electronicConfiguration")) {
                d->inElectronicconfiguration = true;
            } else if (attrs.value(i) == QLatin1String("bo:dangerSymbol")) {
                d->inDangerSymbol = true;
            } else if (attrs.value(i) == QLatin1String("bo:RPhrase")) {
                d->inRPhrase = true;
            } else if (attrs.value(i) == QLatin1String("bo:SPhrase")) {
                d->inSPhrase = true;
            } else if (attrs.value(i) == QLatin1String("bo:oxidation")) {
                d->inOxidation = true;
            }
        }
    } else if (d->inElement && localName == QLatin1String("label")) {
        for (int i = 0; i < attrs.length(); ++i) {
            if (attrs.localName(i) != QLatin1String("dictRef")) {
                continue;
            }

            if (attrs.value(i) == QLatin1String("bo:symbol")) {
                for (int i = 0; i < attrs.length(); ++i) {
                    if (attrs.localName(i) == QLatin1String("value")) {
                        d->currentDataObject.setData(attrs.value(i));
                        d->currentDataObject.setType(ChemicalDataObject::symbol);

                        if (d->currentElement) {
                            d->currentElement->addData(d->currentDataObject);
                        }
                    }
                }
            } else if (attrs.value(i) == QLatin1String("bo:name")) {
                for (int i = 0; i < attrs.length(); ++i) {
                    if (attrs.localName(i) == QLatin1String("value")) {
                        d->currentDataObject.setData(i18n(attrs.value(i).toUtf8().constData()));
                        d->currentDataObject.setType(ChemicalDataObject::name);

                        if (d->currentElement) {
                            d->currentElement->addData(d->currentDataObject);
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool ElementSaxParser::endElement(const QString &, const QString &localName, const QString &)
{
    if (localName == QLatin1String("atom")) {
        if (d->currentElement->dataAsString(ChemicalDataObject::symbol) != QLatin1String("Xx")) {
            d->elements.append(d->currentElement);
        } else {
            delete d->currentElement;
        }

        d->currentElement = nullptr;
        d->inElement = false;
    } else if (localName == QLatin1String("scalar") || localName == QLatin1String("label") || localName == QLatin1String("array")) {
        d->currentDataObject.setUnit(d->currentUnit);
    }
    return true;
}

bool ElementSaxParser::characters(const QString &ch)
{
    d->currentDataObject = ChemicalDataObject();
    ChemicalDataObject::BlueObelisk type;
    QVariant value;

    if (d->inMass) {
        value = ch.toDouble();
        type = ChemicalDataObject::mass;
        d->inMass = false;
    } else if (d->inExactMass) {
        value = ch.toDouble();
        type = ChemicalDataObject::exactMass;
        d->inExactMass = false;
    } else if (d->inAtomicNumber) {
        value = ch.toInt();
        type = ChemicalDataObject::atomicNumber;
        d->inAtomicNumber = false;
    } else if (d->inIonization) {
        value = ch.toDouble();
        ;
        type = ChemicalDataObject::ionization;
        d->inIonization = false;
    } else if (d->inElectronAffinity) {
        value = ch.toDouble();
        type = ChemicalDataObject::electronAffinity;
        d->inElectronAffinity = false;
    } else if (d->inElectronegativityPauling) {
        value = ch.toDouble();
        type = ChemicalDataObject::electronegativityPauling;
        d->inElectronegativityPauling = false;
    } else if (d->inRadiusCovalent) {
        value = ch.toDouble();
        type = ChemicalDataObject::radiusCovalent;
        d->inRadiusCovalent = false;
    } else if (d->inRadiusVDW) {
        value = ch.toDouble();
        type = ChemicalDataObject::radiusVDW;
        d->inRadiusVDW = false;
    } else if (d->inMeltingPoint) {
        value = ch.toDouble();
        type = ChemicalDataObject::meltingpoint;
        d->inMeltingPoint = false;
    } else if (d->inBoilingPoint) {
        value = ch.toDouble();
        type = ChemicalDataObject::boilingpoint;
        d->inBoilingPoint = false;
    } else if (d->inPeriodTableBlock) {
        value = ch;
        type = ChemicalDataObject::periodTableBlock;
        d->inPeriodTableBlock = false;
    } else if (d->inNameOrigin) {
        value = i18n(ch.toUtf8().constData());
        type = ChemicalDataObject::nameOrigin;
        d->inNameOrigin = false;
    } else if (d->inDiscoveryDate) {
        value = ch.toInt();
        type = ChemicalDataObject::date;
        d->inDiscoveryDate = false;
    } else if (d->inDiscoverers) {
        value = ch;
        type = ChemicalDataObject::discoverers;
        d->inDiscoverers = false;
    } else if (d->inPeriod) {
        value = ch.toInt();
        type = ChemicalDataObject::period;
        d->inPeriod = false;
    } else if (d->inCrystalstructure) {
        value = ch;
        type = ChemicalDataObject::crystalstructure;
        d->inCrystalstructure = false;
    } else if (d->inAcidicbehaviour) {
        value = ch.toInt();
        type = ChemicalDataObject::acidicbehaviour;
        d->inAcidicbehaviour = false;
    } else if (d->inFamily) {
        value = ch;
        type = ChemicalDataObject::family;
        d->inFamily = false;
    } else if (d->inGroup) {
        value = ch.toInt();
        type = ChemicalDataObject::group;
        d->inGroup = false;
    } else if (d->inElectronicconfiguration) {
        value = ch;
        type = ChemicalDataObject::electronicConfiguration;
        d->inElectronicconfiguration = false;
    } else if (d->inDangerSymbol) {
        value = ch;
        type = ChemicalDataObject::dangerSymbol;
        d->inDangerSymbol = false;
    } else if (d->inRPhrase) {
        value = ch;
        type = ChemicalDataObject::RPhrase;
        d->inRPhrase = false;
    } else if (d->inSPhrase) {
        value = ch;
        type = ChemicalDataObject::SPhrase;
        d->inSPhrase = false;
    } else if (d->inCountry) {
        if (ch == QLatin1String("ancient")) {
            value = 0;
            type = ChemicalDataObject::date;
        } else {
            value = ch;
            type = ChemicalDataObject::discoveryCountry;
        }
        d->inCountry = false;
    } else if (d->inOxidation) {
        value = ch;
        type = ChemicalDataObject::oxidation;
        d->inOxidation = false;
    } else { // it is a non known value. Do not create a wrong object but return
        return true;
    }

    d->currentDataObject.setData(value);
    d->currentDataObject.setType(type);
    d->currentDataObject.setUnit(d->currentUnit);

    if (d->currentElement) {
        d->currentElement->addData(d->currentDataObject);
    }

    return true;
}

int ElementSaxParser::unit(const QString &unit) const
{
    if (unit == QLatin1String("siUnits:kelvin")) {
        return KUnitConversion::Kelvin;
    } else if (unit == QLatin1String("units:ev")) {
        return KUnitConversion::Electronvolt;
    } else if (unit == QLatin1String("units:ang")) {
        return KUnitConversion::Angstrom;
    } else if (unit == QLatin1String("bo:noUnit")) {
        return KUnitConversion::NoUnit;
    } else {
        return KUnitConversion::NoUnit;
    }
}

QList<Element *> ElementSaxParser::getElements() const
{
    return d->elements;
}
