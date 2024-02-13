/*
    SPDX-FileCopyrightText: 2005 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHEMICALDATAOBJECT_H
#define CHEMICALDATAOBJECT_H

#include <QSharedData>
#include <QSharedDataPointer>
#include <QVariant>

#include "science_export.h"

class ChemicalDataObjectPrivate;
/**
 * A ChemicalDataObject is an object which contains information about
 * a chemical element. This can for example be a boiling point. The information
 * is stored in a QVariant.
 * This class supports the CML-format defined by the BlueObelisk-Project.
 *
 * @author Carsten Niehaus <cniehaus@kde.org>
 */
class SCIENCE_EXPORT ChemicalDataObject
{
public:
    /**
     * The BlueObelisk-project defines in their XML file the dataset
     * with the names of the enum plus "bo:". So for symbol
     * it is "bo:symbol". To avoid confusion I will choose the very
     * same naming
     */
    enum BlueObelisk {
        atomicNumber = 0 /**< The atomic number of the element */,
        symbol /**< the symbol of the element */,
        name /**< The IUPAC name of the element */,
        mass /**< # IUPAC Official Masses */,
        exactMass /**< exact masses of the most common isotopes for each element */,
        ionization /**< First inizationenergy */,
        electronAffinity /**< the electron affinity of the element */,
        electronegativityPauling /**< the electronegativity in the definition of Pauling*/,
        radiusCovalent /**< the covalent radius */,
        radiusVDW /**< the van der Waals radius */,
        meltingpoint /**< the meltingpoint */,
        boilingpoint /**< the boilingpoint */,
        periodTableBlock /**< the block of the element */,
        family /**< "Noblegas" "Non-Metal" "Rare_Earth" "Alkaline_Earth" "Alkali_Earth" "Transition" "Other_Metal" "Metalloids" "Halogene" */,
        acidicbehaviour /**< 0 means acidic, 1 means basic, 2 means neutral, 3 means amphoteric*/,
        crystalstructure /**< own, bcc, hdp, ccp, hcp, fcc, d, sc, tet, rh, or, mono*/,
        electronicConfiguration /**< the electronic configuration, for example 1s2 for He*/,
        group /**< This is a value between 1 and 8*/,
        nameOrigin /**< the origin of the name */,
        orbit /**< the quantumorbit of the element */,
        period /**< the period of the element */,
        date /**< date of discovery of the element. When 0, the element has been known in ancient times. When the value is -1 the element has not yet been
                officially recognized by the IUPAC */
        ,
        discoverers /** The name of the discoverers, separated by semicolomns */,
        relativeAbundance /** The abundance, relative to 100 */,
        spin /**< The spin */,
        magneticMoment /**< The magnetic dipole moment */,
        halfLife /**< The halflife */,
        alphaDecayLikeliness /**< The percentage of alphadecay */,
        alphaDecay /**< The decayenergy of alphadecay in MeV */,
        alphabetaminusDecayLikeliness /**< The percentage of alphadecay */,
        alphabetaminusDecay /**< The decayenergy of alphadecay in MeV */,
        betaplusDecayLikeliness /**< The percentage of betaplusdecay */,
        betaplusDecay /**< The decayenergy of betaplusdecay in MeV */,
        twobetaplusDecayLikeliness,
        twobetaplusDecay,
        betaplusprotonDecayLikeliness /**< The percentage of betaplusdecay */,
        betaplusprotonDecay /**< The decayenergy of betaplusdecay in MeV */,
        betaplustwoprotonDecayLikeliness,
        betaplustwoprotonDecay,
        betaplusalphaDecayLikeliness /**< The percentage of betaplusdecay */,
        betaplusalphaDecay /**< The decayenergy of betaplusdecay in MeV */,
        betaplustwoalphaDecayLikeliness,
        betaplustwoalphaDecay,
        betaplusthreealphaDecayLikeliness,
        betaplusthreealphaDecay,
        betaminusDecayLikeliness /**< The percentage of betaminusdecay */,
        betaminusDecay /**< The decayenergy of betaminusdecay in MeV */,
        twobetaminusDecayLikeliness,
        twobetaminusDecay,
        betaminusneutronDecayLikeliness /**< The percentage of betaminusdecay */,
        betaminusneutronDecay /**< The decayenergy of betaminusdecay in MeV */,
        betaminustwoneutronDecayLikeliness,
        betaminustwoneutronDecay,
        betaminusthreeneutronDecayLikeliness,
        betaminusthreeneutronDecay,
        betaminusfourneutronDecayLikeliness,
        betaminusfourneutronDecay,
        betaminusfissionDecayLikeliness /**< The percentage of betaminusdecay */,
        betaminusfissionDecay /**< The decayenergy of betaminusdecay in MeV */,
        betaminusalphaDecayLikeliness /**< The percentage of betaminusdecay */,
        betaminusalphaDecay /**< The decayenergy of betaminusdecay in MeV */,
        betaminustwoalphaDecayLikeliness,
        betaminustwoalphaDecay,
        betaminusthreealphaDecayLikeliness,
        betaminusthreealphaDecay,
        betaminusalphaneutronDecay,
        betaminusalphaneutronDecayLikeliness,
        ecDecayLikeliness /**< The percentage of ecdecay */,
        ecDecay /**< The decayenergy of ecminusdecay in MeV */,
        twoecDecayLikeliness,
        twoecDecay,
        ecalphaDecayLikeliness,
        ecalphaDecay,
        ecalphaprotonDecayLikeliness,
        ecalphaprotonDecay,
        ecprotonDecayLikeliness,
        ecprotonDecay,
        ectwoprotonDecayLikeliness,
        ectwoprotonDecay,
        ecthreeprotonDecayLikeliness,
        ecthreeprotonDecay,
        protonDecayLikeliness /**< The percentage of protondecay */,
        protonDecay /**< The decayenergy of protonminusdecay in MeV */,
        twoprotonDecayLikeliness,
        twoprotonDecay,
        protonalphaDecayLikeliness /**< The percentage of protondecay */,
        protonalphaDecay /**< The decayenergy of protonminusdecay in MeV */,
        neutronDecayLikeliness /**< The percentage of neutrondecay */,
        neutronDecay /**< The decayenergy of neutronminusdecay in MeV */,
        twoneutronDecayLikeliness,
        twoneutronDecay,
        spontfissionDecayLikeliness,
        spontfissionDecay,
        dangerSymbol /**< the danger symbols are the dangers associated with an element, for example Xn,T+ */,
        RPhrase /**< */,
        SPhrase /**< */,
        discoveryCountry,
        oxidation /**< Oxidation states*/
    };

    /**
     * Constructor.
     */
    ChemicalDataObject();

    /**
     * Constructor.
     * @param v the data of the object
     * @param type the type of the data
     * @param errorValue the error margin of the value @p v
     *
     * @see errorValue()
     */
    ChemicalDataObject(const QVariant &v, BlueObelisk type, const QVariant &errorValue = QVariant(0));

    /**
     * Copy constructor.
     */
    ChemicalDataObject(const ChemicalDataObject &other);

    /**
     * Destructor.
     */
    ~ChemicalDataObject();

    /**
     * Set the data of this object to @p v
     * @param v the value of the object
     */
    void setData(const QVariant &v);

    /**
     * Set the error value of this object to @p v.
     * The error has to have the same unit as the value.
     * @param v the value of the object
     */
    void setErrorValue(const QVariant &v);

    /**
     * Every ChemicalDataObject contains one data. For example a
     * integer value which represents the boiling point. This method
     * returns the value as a QString.
     *
     * For bool, the returned string will be "false" or "true"
     * For a QString, the QString will be returned
     * For a int or double, the value will be returned as a QString
     *
     * @return the value as a QString
     */
    QString valueAsString() const;

    /**
     * Every ChemicalDataObject contains one data. For example a
     * integer value which represents the boiling point. This method
     * returns the value as a QVariant.
     *
     * @return the value as a QVariant
     */
    QVariant value() const;

    /**
     * @return the error margin of the object
     */
    QVariant errorValue() const;

    /**
     * @return the type of dataset of this object
     */
    BlueObelisk type() const;

    /**
     * @param type the type of this object
     */
    void setType(BlueObelisk type);

    /**
     * @overload
     */
    void setType(int type);

    /**
     * Compare the value @p v with the data of this object
     */
    bool operator==(const int v) const;

    /**
     * Compare the value @p v with the data of this object
     */
    bool operator==(const double v) const;

    /**
     * Compare the value @p v with the data of this object
     */
    bool operator==(const bool v) const;

    /**
     * Compare the value @p v with the data of this object
     */
    bool operator==(const QString &v) const;

    /**
     * @return the unit of the object as a QString. For example kelvin
     * will be returned as "bo:kelvin"
     */
    QString unitAsString() const;

    /**
     * @return the unit of the object
     */
    int unit() const;

    /**
     * set the unit of this object to @p unit
     * @param unit the BlueObeliskUnit for this object
     */
    void setUnit(int unit);

    ChemicalDataObject &operator=(const ChemicalDataObject &other);

    bool operator==(const ChemicalDataObject &other) const;

    bool operator!=(const ChemicalDataObject &other) const;

private:
    QSharedDataPointer<ChemicalDataObjectPrivate> d;
};

#endif // CHEMICALDATAOBJECT_H
