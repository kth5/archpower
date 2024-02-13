/*
    SPDX-FileCopyrightText: 2005-2008 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotopeparser.h"

#include "chemicaldataobject.h"
#include "isotope.h"

#include "kalzium_libscience_debug.h"

#include <KUnitConversion/Converter>

class IsotopeParser::Private
{
public:
    Private()
        : currentUnit(KUnitConversion::NoUnit)
        , currentErrorValue(QVariant())
        , currentElementSymbol(QString())
        , currentIsotope(nullptr)
        , inIsotope(false)
        , inElement(false)
        , inAtomicNumber(false)
        , inExactMass(false)
        , inSpin(false)
        , inMagMoment(false)
        , inHalfLife(false)
        , inAlphaDecay(false)
        , inAlphaDecayLikeliness(false)
        , inProtonDecay(false)
        , inProtonDecayLikeliness(false)
        , inTwoProtonDecay(false)
        , inTwoProtonDecayLikeliness(false)
        , inNeutronDecay(false)
        , inNeutronDecayLikeliness(false)
        , inTwoNeutronDecay(false)
        , inTwoNeutronDecayLikeliness(false)
        , inECDecay(false)
        , inECDecayLikeliness(false)
        , inTwoECDecay(false)
        , inTwoECDecayLikeliness(false)
        , inBetaminusDecay(false)
        , inBetaminusDecayLikeliness(false)
        , inBetaminusFissionDecay(false)
        , inBetaminusFissionDecayLikeliness(false)
        , inTwoBetaminusDecay(false)
        , inTwoBetaminusDecayLikeliness(false)
        , inBetaplusDecay(false)
        , inBetaplusDecayLikeliness(false)
        , inTwoBetaplusDecay(false)
        , inTwoBetaplusDecayLikeliness(false)
        , inBetaminusNeutronDecay(false)
        , inBetaminusNeutronDecayLikeliness(false)
        , inBetaminusTwoNeutronDecay(false)
        , inBetaminusTwoNeutronDecayLikeliness(false)
        , inBetaminusThreeNeutronDecay(false)
        , inBetaminusThreeNeutronDecayLikeliness(false)
        , inBetaminusFourNeutronDecay(false)
        , inBetaminusFourNeutronDecayLikeliness(false)
        , inBetaminusAlphaNeutronDecay(false)
        , inBetaminusAlphaNeutronDecayLikeliness(false)
        , inBetaminusAlphaDecay(false)
        , inBetaminusAlphaDecayLikeliness(false)
        , inBetaminusTwoAlphaDecay(false)
        , inBetaminusTwoAlphaDecayLikeliness(false)
        , inBetaminusThreeAlphaDecay(false)
        , inBetaminusThreeAlphaDecayLikeliness(false)
        , inBetaplusProtonDecay(false)
        , inBetaplusProtonDecayLikeliness(false)
        , inBetaplusTwoProtonDecay(false)
        , inBetaplusTwoProtonDecayLikeliness(false)
        , inBetaplusAlphaDecay(false)
        , inBetaplusAlphaDecayLikeliness(false)
        , inBetaplusTwoAlphaDecay(false)
        , inBetaplusTwoAlphaDecayLikeliness(false)
        , inBetaplusThreeAlphaDecay(false)
        , inBetaplusThreeAlphaDecayLikeliness(false)
        , inAlphaBetaminusDecay(false)
        , inAlphaBetaminusDecayLikeliness(false)
        , inProtonAlphaDecay(false)
        , inProtonAlphaDecayLikeliness(false)
        , inECProtonDecay(false)
        , inECProtonDecayLikeliness(false)
        , inECTwoProtonDecay(false)
        , inECTwoProtonDecayLikeliness(false)
        , inECThreeProtonDecay(false)
        , inECThreeProtonDecayLikeliness(false)
        , inECAlphaDecay(false)
        , inECAlphaDecayLikeliness(false)
        , inECAlphaProtonDecay(false)
        , inECAlphaProtonDecayLikeliness(false)
        , inSpontFissionDecay(false)
        , inSpontFissionDecayLikeliness(false)
        , inAbundance(false)
    {
    }

    ~Private()
    {
        delete currentIsotope;
        // qDeleteAll(isotopes);
    }

    ChemicalDataObject currentDataObject;
    int currentUnit;
    QVariant currentErrorValue;
    QString currentElementSymbol;
    Isotope *currentIsotope;

    QList<Isotope *> isotopes;

    bool inIsotope;
    bool inElement;
    bool inAtomicNumber;
    bool inExactMass;
    bool inSpin;
    bool inMagMoment;
    bool inHalfLife;
    bool inAlphaDecay;
    bool inAlphaDecayLikeliness;
    bool inProtonDecay;
    bool inProtonDecayLikeliness;
    bool inTwoProtonDecay;
    bool inTwoProtonDecayLikeliness;
    bool inNeutronDecay;
    bool inNeutronDecayLikeliness;
    bool inTwoNeutronDecay;
    bool inTwoNeutronDecayLikeliness;
    bool inECDecay;
    bool inECDecayLikeliness;
    bool inTwoECDecay;
    bool inTwoECDecayLikeliness;
    bool inBetaminusDecay;
    bool inBetaminusDecayLikeliness;
    bool inBetaminusFissionDecay;
    bool inBetaminusFissionDecayLikeliness;
    bool inTwoBetaminusDecay;
    bool inTwoBetaminusDecayLikeliness;
    bool inBetaplusDecay;
    bool inBetaplusDecayLikeliness;
    bool inTwoBetaplusDecay;
    bool inTwoBetaplusDecayLikeliness;
    bool inBetaminusNeutronDecay;
    bool inBetaminusNeutronDecayLikeliness;
    bool inBetaminusTwoNeutronDecay;
    bool inBetaminusTwoNeutronDecayLikeliness;
    bool inBetaminusThreeNeutronDecay;
    bool inBetaminusThreeNeutronDecayLikeliness;
    bool inBetaminusFourNeutronDecay;
    bool inBetaminusFourNeutronDecayLikeliness;
    bool inBetaminusAlphaNeutronDecay;
    bool inBetaminusAlphaNeutronDecayLikeliness;
    bool inBetaminusAlphaDecay;
    bool inBetaminusAlphaDecayLikeliness;
    bool inBetaminusTwoAlphaDecay;
    bool inBetaminusTwoAlphaDecayLikeliness;
    bool inBetaminusThreeAlphaDecay;
    bool inBetaminusThreeAlphaDecayLikeliness;
    bool inBetaplusProtonDecay;
    bool inBetaplusProtonDecayLikeliness;
    bool inBetaplusTwoProtonDecay;
    bool inBetaplusTwoProtonDecayLikeliness;
    bool inBetaplusAlphaDecay;
    bool inBetaplusAlphaDecayLikeliness;
    bool inBetaplusTwoAlphaDecay;
    bool inBetaplusTwoAlphaDecayLikeliness;
    bool inBetaplusThreeAlphaDecay;
    bool inBetaplusThreeAlphaDecayLikeliness;
    bool inAlphaBetaminusDecay;
    bool inAlphaBetaminusDecayLikeliness;
    bool inProtonAlphaDecay;
    bool inProtonAlphaDecayLikeliness;
    bool inECProtonDecay;
    bool inECProtonDecayLikeliness;
    bool inECTwoProtonDecay;
    bool inECTwoProtonDecayLikeliness;
    bool inECThreeProtonDecay;
    bool inECThreeProtonDecayLikeliness;
    bool inECAlphaDecay;
    bool inECAlphaDecayLikeliness;
    bool inECAlphaProtonDecay;
    bool inECAlphaProtonDecayLikeliness;
    bool inSpontFissionDecay;
    bool inSpontFissionDecayLikeliness;
    bool inAbundance;
};

IsotopeParser::IsotopeParser()
    : QXmlDefaultHandler()
    , d(new Private)
{
}

IsotopeParser::~IsotopeParser()
{
    delete d;
}

bool IsotopeParser::startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &attrs)
{
    if (localName == QLatin1String("isotopeList")) {
        d->inElement = true;

        // now save the symbol of the current element
        for (int i = 0; i < attrs.length(); ++i) {
            if (attrs.localName(i) == QLatin1String("id")) {
                d->currentElementSymbol = attrs.value(i);
            }
        }
    } else if (d->inElement && localName == QLatin1String("isotope")) {
        d->currentIsotope = new Isotope();
        d->currentIsotope->addData(ChemicalDataObject(QVariant(d->currentElementSymbol), ChemicalDataObject::symbol));
        d->inIsotope = true;
        for (int i = 0; i < attrs.length(); ++i) {
            if (attrs.localName(i) == QLatin1String("number")) {
                d->currentIsotope->setNucleons(attrs.value(i).toInt());
            }
        }
    } else if (d->inIsotope && localName == QLatin1String("scalar")) {
        for (int i = 0; i < attrs.length(); ++i) {
            if (attrs.localName(i) == QLatin1String("errorValue")) {
                d->currentErrorValue = QVariant(attrs.value(i));
                continue;
            }

            if (attrs.value(i) == QLatin1String("bo:atomicNumber")) {
                d->inAtomicNumber = true;
            } else if (attrs.value(i) == QLatin1String("bo:exactMass")) {
                d->inExactMass = true;
            } else if (attrs.value(i) == QLatin1String("bo:halfLife")) {
                for (int i = 0; i < attrs.length(); ++i) {
                    if (attrs.localName(i) == QLatin1String("units")) {
                        if (attrs.value(i) == QLatin1String("siUnits:s")) {
                            d->currentUnit = KUnitConversion::Second;
                        } else if (attrs.value(i) == QLatin1String("units:y")) {
                            d->currentUnit = KUnitConversion::Year;
                        } else {
                            d->currentUnit = KUnitConversion::NoUnit;
                        }
                    }
                }

                d->currentDataObject.setUnit(d->currentUnit);
                d->inHalfLife = true;
            } else if (attrs.value(i) == QLatin1String("bo:alphaDecay")) {
                d->inAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:alphaDecayLikeliness")) {
                d->inAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:protonDecay")) {
                d->inProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:protonDecayLikeliness")) {
                d->inProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:2protonDecay")) {
                d->inTwoProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:2protonDecayLikeliness")) {
                d->inTwoProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:neutronDecay")) {
                d->inNeutronDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:neutronDecayLikeliness")) {
                d->inNeutronDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:2neutronDecay")) {
                d->inTwoNeutronDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:2neutronDecayLikeliness")) {
                d->inTwoNeutronDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecDecay")) {
                d->inECDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecDecayLikeliness")) {
                d->inECDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:2ecDecay")) {
                d->inTwoECDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:2ecDecayLikeliness")) {
                d->inTwoECDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusDecay")) {
                d->inBetaminusDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusDecayLikeliness")) {
                d->inBetaminusDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusfissionDecay")) {
                d->inBetaminusFissionDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusfissionDecayLikeliness")) {
                d->inBetaminusFissionDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:2betaminusDecay")) {
                d->inTwoBetaminusDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:2betaminusDecayLikeliness")) {
                d->inTwoBetaminusDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplusDecay")) {
                d->inBetaplusDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplusDecayLikeliness")) {
                d->inBetaplusDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:2betaplusDecay")) {
                d->inTwoBetaplusDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:2betaplusDecayLikeliness")) {
                d->inTwoBetaplusDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusneutronDecay")) {
                d->inBetaminusNeutronDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusneutronDecayLikeliness")) {
                d->inBetaminusNeutronDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus2neutronDecay")) {
                d->inBetaminusTwoNeutronDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus2neutronDecayLikeliness")) {
                d->inBetaminusTwoNeutronDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus3neutronDecay")) {
                d->inBetaminusThreeNeutronDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus3neutronDecayLikeliness")) {
                d->inBetaminusThreeNeutronDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus4neutronDecay")) {
                d->inBetaminusFourNeutronDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus4neutronDecayLikeliness")) {
                d->inBetaminusFourNeutronDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusalphaneutronDecay")) {
                d->inBetaminusAlphaNeutronDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusalphaneutronDecayLikeliness")) {
                d->inBetaminusAlphaNeutronDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusalphaDecay")) {
                d->inBetaminusAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminusalphaDecayLikeliness")) {
                d->inBetaminusAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus2alphaDecay")) {
                d->inBetaminusTwoAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus2alphaDecayLikeliness")) {
                d->inBetaminusTwoAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus3alphaDecay")) {
                d->inBetaminusThreeAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaminus3alphaDecayLikeliness")) {
                d->inBetaminusThreeAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplusprotonDecay")) {
                d->inBetaplusProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplusprotonDecayLikeliness")) {
                d->inBetaplusProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplus2protonDecay")) {
                d->inBetaplusTwoProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplus2protonDecayLikeliness")) {
                d->inBetaplusTwoProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplusalphaDecay")) {
                d->inBetaplusAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplusalphaDecayLikeliness")) {
                d->inBetaplusAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplus2alphaDecay")) {
                d->inBetaplusTwoAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplus2alphaDecayLikeliness")) {
                d->inBetaplusTwoAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplus3alphaDecay")) {
                d->inBetaplusThreeAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:betaplus3alphaDecayLikeliness")) {
                d->inBetaplusThreeAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:alphabetaminusDecay")) {
                d->inAlphaBetaminusDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:alphabetaminusDecayLikeliness")) {
                d->inAlphaBetaminusDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:protonalphaDecay")) {
                d->inProtonAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:protonalphaDecayLikeliness")) {
                d->inProtonAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecprotonDecay")) {
                d->inECProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecprotonDecayLikeliness")) {
                d->inECProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:ec2protonDecay")) {
                d->inECTwoProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:ec2protonDecayLikeliness")) {
                d->inECTwoProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:ec3protonDecay")) {
                d->inECThreeProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:ec3protonDecayLikeliness")) {
                d->inECThreeProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecalphaDecay")) {
                d->inECAlphaDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecalphaDecayLikeliness")) {
                d->inECAlphaDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecalphaprotonDecay")) {
                d->inECAlphaProtonDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:ecalphaprotonDecayLikeliness")) {
                d->inECAlphaProtonDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:spontfissionDecay")) {
                d->inSpontFissionDecay = true;
            } else if (attrs.value(i) == QLatin1String("bo:spontfissionDecayLikeliness")) {
                d->inSpontFissionDecayLikeliness = true;
            } else if (attrs.value(i) == QLatin1String("bo:spin")) {
                d->inSpin = true;
            } else if (attrs.value(i) == QLatin1String("bo:magneticMoment")) {
                d->inMagMoment = true;
            } else if (attrs.value(i) == QLatin1String("bo:relativeAbundance")) {
                d->inAbundance = true;
            }
        }
    }
    return true;
}

bool IsotopeParser::endElement(const QString &, const QString &localName, const QString &)
{
    if (localName == QLatin1String("isotope")) {
        d->isotopes.append(d->currentIsotope);

        d->currentIsotope = nullptr;
        d->inIsotope = false;
    } else if (localName == QLatin1String("isotopeList")) { // a new list of isotopes start...
        d->inElement = false;
    }

    return true;
}

bool IsotopeParser::characters(const QString &ch)
{
    ChemicalDataObject::BlueObelisk type;
    QVariant value;

    if (d->inExactMass) {
        value = ch.toDouble();
        type = ChemicalDataObject::exactMass;
        d->inExactMass = false;
    } else if (d->inAtomicNumber) {
        value = ch.toInt();
        type = ChemicalDataObject::atomicNumber;
        d->inAtomicNumber = false;
    } else if (d->inSpin) {
        value = ch;
        type = ChemicalDataObject::spin;
        d->inSpin = false;
    } else if (d->inMagMoment) {
        value = ch;
        type = ChemicalDataObject::magneticMoment;
        d->inMagMoment = false;
    } else if (d->inHalfLife) {
        value = ch.toDouble();
        type = ChemicalDataObject::halfLife;
        d->inHalfLife = false;
    } else if (d->inAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::alphaDecay;
        d->inAlphaDecay = false;
    } else if (d->inAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::alphaDecayLikeliness;
        d->inAlphaDecayLikeliness = false;
    } else if (d->inProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::protonDecay;
        d->inProtonDecay = false;
    } else if (d->inProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::protonDecayLikeliness;
        d->inProtonDecayLikeliness = false;
    } else if (d->inTwoProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::twoprotonDecay;
        d->inTwoProtonDecay = false;
    } else if (d->inTwoProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::twoprotonDecayLikeliness;
        d->inTwoProtonDecayLikeliness = false;
    } else if (d->inNeutronDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::neutronDecay;
        d->inNeutronDecay = false;
    } else if (d->inNeutronDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::neutronDecayLikeliness;
        d->inNeutronDecayLikeliness = false;
    } else if (d->inTwoNeutronDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::twoneutronDecay;
        d->inTwoNeutronDecay = false;
    } else if (d->inTwoNeutronDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::twoneutronDecayLikeliness;
        d->inTwoNeutronDecayLikeliness = false;
    } else if (d->inECDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecDecay;
        d->inECDecay = false;
    } else if (d->inECDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecDecayLikeliness;
        d->inECDecayLikeliness = false;
    } else if (d->inTwoECDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::twoecDecay;
        d->inTwoECDecay = false;
    } else if (d->inTwoECDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::twoecDecayLikeliness;
        d->inTwoECDecayLikeliness = false;
    } else if (d->inBetaminusDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusDecay;
        d->inBetaminusDecay = false;
    } else if (d->inBetaminusDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusDecayLikeliness;
        d->inBetaminusDecayLikeliness = false;
    } else if (d->inBetaminusFissionDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusfissionDecay;
        d->inBetaminusFissionDecay = false;
    } else if (d->inBetaminusFissionDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusfissionDecayLikeliness;
        d->inBetaminusFissionDecayLikeliness = false;
    } else if (d->inTwoBetaminusDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::twobetaminusDecay;
        d->inTwoBetaminusDecay = false;
    } else if (d->inTwoBetaminusDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::twobetaminusDecayLikeliness;
        d->inTwoBetaminusDecayLikeliness = false;
    } else if (d->inBetaplusDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusDecay;
        d->inBetaplusDecay = false;
    } else if (d->inBetaplusDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusDecayLikeliness;
        d->inBetaplusDecayLikeliness = false;
    } else if (d->inTwoBetaplusDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::twobetaplusDecay;
        d->inTwoBetaplusDecay = false;
    } else if (d->inTwoBetaplusDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::twobetaplusDecayLikeliness;
        d->inTwoBetaplusDecayLikeliness = false;
    } else if (d->inBetaminusNeutronDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusneutronDecay;
        d->inBetaminusNeutronDecay = false;
    } else if (d->inBetaminusNeutronDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusneutronDecayLikeliness;
        d->inBetaminusNeutronDecayLikeliness = false;
    } else if (d->inBetaminusTwoNeutronDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminustwoneutronDecay;
        d->inBetaminusTwoNeutronDecay = false;
    } else if (d->inBetaminusTwoNeutronDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminustwoneutronDecayLikeliness;
        d->inBetaminusTwoNeutronDecayLikeliness = false;
    } else if (d->inBetaminusThreeNeutronDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusthreeneutronDecay;
        d->inBetaminusThreeNeutronDecay = false;
    } else if (d->inBetaminusThreeNeutronDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusthreeneutronDecayLikeliness;
        d->inBetaminusThreeNeutronDecayLikeliness = false;
    } else if (d->inBetaminusFourNeutronDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusfourneutronDecay;
        d->inBetaminusFourNeutronDecay = false;
    } else if (d->inBetaminusFourNeutronDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusfourneutronDecayLikeliness;
        d->inBetaminusFourNeutronDecayLikeliness = false;
    } else if (d->inBetaminusAlphaNeutronDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusalphaneutronDecay;
        d->inBetaminusAlphaNeutronDecay = false;
    } else if (d->inBetaminusAlphaNeutronDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusalphaneutronDecayLikeliness;
        d->inBetaminusAlphaNeutronDecayLikeliness = false;
    } else if (d->inBetaminusAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusalphaDecay;
        d->inBetaminusAlphaDecay = false;
    } else if (d->inBetaminusAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusalphaDecayLikeliness;
        d->inBetaminusAlphaDecayLikeliness = false;
    } else if (d->inBetaminusTwoAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminustwoalphaDecay;
        d->inBetaminusTwoAlphaDecay = false;
    } else if (d->inBetaminusTwoAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminustwoalphaDecayLikeliness;
        d->inBetaminusTwoAlphaDecayLikeliness = false;
    } else if (d->inBetaminusThreeAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusthreealphaDecay;
        d->inBetaminusThreeAlphaDecay = false;
    } else if (d->inBetaminusThreeAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaminusthreealphaDecayLikeliness;
        d->inBetaminusThreeAlphaDecayLikeliness = false;
    } else if (d->inBetaplusProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusprotonDecay;
        d->inBetaplusProtonDecay = false;
    } else if (d->inBetaplusProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusprotonDecayLikeliness;
        d->inBetaplusProtonDecayLikeliness = false;
    } else if (d->inBetaplusTwoProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplustwoprotonDecay;
        d->inBetaplusTwoProtonDecay = false;
    } else if (d->inBetaplusTwoProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplustwoprotonDecayLikeliness;
        d->inBetaplusTwoProtonDecayLikeliness = false;
    } else if (d->inBetaplusAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusalphaDecay;
        d->inBetaplusAlphaDecay = false;
    } else if (d->inBetaplusAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusalphaDecayLikeliness;
        d->inBetaplusAlphaDecayLikeliness = false;
    } else if (d->inBetaplusTwoAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplustwoalphaDecay;
        d->inBetaplusTwoAlphaDecay = false;
    } else if (d->inBetaplusTwoAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplustwoalphaDecayLikeliness;
        d->inBetaplusTwoAlphaDecayLikeliness = false;
    } else if (d->inBetaplusThreeAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusthreealphaDecay;
        d->inBetaplusThreeAlphaDecay = false;
    } else if (d->inBetaplusThreeAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::betaplusthreealphaDecayLikeliness;
        d->inBetaplusThreeAlphaDecayLikeliness = false;
    } else if (d->inAlphaBetaminusDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::alphabetaminusDecay;
        d->inAlphaBetaminusDecay = false;
    } else if (d->inAlphaBetaminusDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::alphabetaminusDecayLikeliness;
        d->inAlphaBetaminusDecayLikeliness = false;
    } else if (d->inProtonAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::protonalphaDecay;
        d->inProtonAlphaDecay = false;
    } else if (d->inProtonAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::protonalphaDecayLikeliness;
        d->inProtonAlphaDecayLikeliness = false;
    } else if (d->inECProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecprotonDecay;
        d->inECProtonDecay = false;
    } else if (d->inECProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecprotonDecayLikeliness;
        d->inECProtonDecayLikeliness = false;
    } else if (d->inECTwoProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::ectwoprotonDecay;
        d->inECTwoProtonDecay = false;
    } else if (d->inECTwoProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::ectwoprotonDecayLikeliness;
        d->inECTwoProtonDecayLikeliness = false;
    } else if (d->inECThreeProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecthreeprotonDecay;
        d->inECThreeProtonDecay = false;
    } else if (d->inECThreeProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecthreeprotonDecayLikeliness;
        d->inECThreeProtonDecayLikeliness = false;
    } else if (d->inECAlphaDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecalphaDecay;
        d->inECAlphaDecay = false;
    } else if (d->inECAlphaDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecalphaDecayLikeliness;
        d->inECAlphaDecayLikeliness = false;
    } else if (d->inECAlphaProtonDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecalphaprotonDecay;
        d->inECAlphaProtonDecay = false;
    } else if (d->inECAlphaProtonDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::ecalphaprotonDecayLikeliness;
        d->inECAlphaProtonDecayLikeliness = false;
    } else if (d->inSpontFissionDecay) {
        value = ch.toDouble();
        type = ChemicalDataObject::spontfissionDecay;
        d->inSpontFissionDecay = false;
    } else if (d->inSpontFissionDecayLikeliness) {
        value = ch.toDouble();
        type = ChemicalDataObject::spontfissionDecayLikeliness;
        d->inSpontFissionDecayLikeliness = false;
    } else if (d->inAbundance) {
        value = ch;
        type = ChemicalDataObject::relativeAbundance;
        d->inAbundance = false;
    } else { // it is a non known value. Do not create a wrong object but return
        return true;
    }

    if (type == ChemicalDataObject::exactMass) {
        d->currentDataObject.setErrorValue(d->currentErrorValue);
    }

    d->currentDataObject.setData(value);
    d->currentDataObject.setType(type);

    if (d->currentIsotope) {
        d->currentIsotope->addData(d->currentDataObject);
    }

    return true;
}

QList<Isotope *> IsotopeParser::getIsotopes() const
{
    return d->isotopes;
}
