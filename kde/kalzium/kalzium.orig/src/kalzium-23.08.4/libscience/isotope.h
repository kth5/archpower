/*
    SPDX-FileCopyrightText: 2005-2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISOTOPE_H
#define ISOTOPE_H

#include "chemicaldataobject.h"

#include "science_export.h"

/**
 * @author Carsten Niehaus
 *
 * This class represents an Isotope with all its properties
 */
class SCIENCE_EXPORT Isotope
{
public:
    /**
     * Constructs a new empty isotope.
     */
    Isotope();

    /**
     * Destructor
     */
    ~Isotope();

    /**
     * This struct stores the information how the nucleons in the
     * isotopes are split into neutrons and protons.
     */
    struct Nucleons {
        /**
         * the number of neutrons of the isotope
         */
        int neutrons;

        /**
         * the number of protons of the isotope
         */
        int protons;
    };

    /**
     * @return the mass of the isotope
     */
    double mass() const;

    /**
     * @return the errormargin ( delta mass ) of the isotope
     */
    QString errorMargin() const;

    /**
     * If the isotope belongs to Iron, this method will return "26"
     * @return the number of the element the isotope belongs to
     */
    int parentElementNumber() const;

    /**
     * If the isotope belongs to Iron, this method will return "Fe"
     * @return the symbol of the element the isotope belongs to
     */
    QString parentElementSymbol() const;

    QString spin() const;

    /**
     * @return the magnetic moment of the Isotope
     */
    QString magmoment() const;

    QString abundance() const;

    /**
     * @return for example '17' if halflife of this Isotope is 17 seconds
     * @ref halflife()
     */
    double halflife() const;

    /**
     * @return for example 's' if the unit of the halflife of this Isotope is given in
     * seconds
     */
    QString halflifeUnit() const;

    /**
     * add the ChemicalDataObject @p o
     */
    void addData(const ChemicalDataObject &o);

    /**
     * Set the number of nucleons of the isotope to @p number
     */
    void setNucleons(int number);

    /**
     * @return the sum of protons and neutrons
     */
    int nucleons() const;

    /**
     * @return decay
     */
    double ecdecay() const;

    /**
     * @return decay likeliness
     */
    double eclikeliness() const;

    /**
     * @return decay
     */
    double twoecdecay() const;

    /**
     * @return decay likeliness
     */
    double twoeclikeliness() const;

    /**
     * @return decay
     */
    double ecalphadecay() const;

    /**
     * @return decay likeliness
     */
    double ecalphalikeliness() const;

    /**
     * @return decay
     */
    double ecalphaprotondecay() const;

    /**
     * @return decay likeliness
     */
    double ecalphaprotonlikeliness() const;

    /**
     * @return decay
     */
    double ecprotondecay() const;

    /**
     * @return decay likeliness
     */
    double ecprotonlikeliness() const;

    /**
     * @return decay
     */
    double ectwoprotondecay() const;

    /**
     * @return decay likeliness
     */
    double ectwoprotonlikeliness() const;

    /**
     * @return decay
     */
    double ecthreeprotondecay() const;

    /**
     * @return decay likeliness
     */
    double ecthreeprotonlikeliness() const;

    /**
     * @return decay
     */
    double neutrondecay() const;

    /**
     * @return decay likeliness
     */
    double neutronlikeliness() const;

    /**
     * @return decay
     */
    double twoneutrondecay() const;

    /**
     * @return decay likeliness
     */
    double twoneutronlikeliness() const;

    /**
     * @return decay
     */
    double protondecay() const;

    /**
     * @return decay likeliness
     */
    double protonlikeliness() const;

    /**
     * @return decay
     */
    double twoprotondecay() const;

    /**
     * @return decay likeliness
     */
    double twoprotonlikeliness() const;

    /**
     * @return decay
     */
    double protonalphadecay() const;

    /**
     * @return decay likeliness
     */
    double protonalphalikeliness() const;

    /**
     * @return decay
     */
    double betaminusdecay() const;
    /**
     * @return decay likeliness
     */
    double betaminuslikeliness() const;

    /**
     * @return decay
     */
    double twobetaminusdecay() const;
    /**
     * @return decay likeliness
     */
    double twobetaminuslikeliness() const;

    /**
     * @return decay
     */
    double betaminusneutrondecay() const;
    /**
     * @return decay likeliness
     */
    double betaminusneutronlikeliness() const;

    /**
     * @return decay
     */
    double betaminustwoneutrondecay() const;
    /**
     * @return decay likeliness
     */
    double betaminustwoneutronlikeliness() const;

    /**
     * @return decay
     */
    double betaminusthreeneutrondecay() const;
    /**
     * @return decay likeliness
     */
    double betaminusthreeneutronlikeliness() const;

    /**
     * @return decay
     */
    double betaminusfourneutrondecay() const;
    /**
     * @return decay likeliness
     */
    double betaminusfourneutronlikeliness() const;
    /**
     * @return decay
     */
    double betaminusfissiondecay() const;
    /**
     * @return decay likeliness
     */
    double betaminusfissionlikeliness() const;

    /**
     * @return decay
     */
    double betaminusalphadecay() const;
    /**
     * @return decay likeliness
     */
    double betaminusalphalikeliness() const;

    /**
     * @return decay
     */
    double betaminusalphaneutrondecay() const;
    /**
     * @return decay likeliness
     */
    double betaminusalphaneutronlikeliness() const;

    /**
     * @return decay
     */
    double betaminustwoalphadecay() const;
    /**
     * @return decay likeliness
     */
    double betaminustwoalphalikeliness() const;

    /**
     * @return decay
     */
    double betaminusthreealphadecay() const;
    /**
     * @return decay likeliness
     */
    double betaminusthreealphalikeliness() const;

    /**
     * @return decay
     */
    double betaplusdecay() const;

    /**
     * @return decay likeliness
     */
    double betapluslikeliness() const;

    /**
     * @return decay
     */
    double twobetaplusdecay() const;

    /**
     * @return decay likeliness
     */
    double twobetapluslikeliness() const;

    /**
     * @return decay
     */
    double betaplusprotondecay() const;

    /**
     * @return decay likeliness
     */
    double betaplusprotonlikeliness() const;

    /**
     * @return decay
     */
    double betaplustwoprotondecay() const;

    /**
     * @return decay likeliness
     */
    double betaplustwoprotonlikeliness() const;

    /**
     * @return decay
     */
    double betaplusalphadecay() const;

    /**
     * @return decay likeliness
     */
    double betaplusalphalikeliness() const;

    /**
     * @return decay
     */
    double betaplustwoalphadecay() const;

    /**
     * @return decay likeliness
     */
    double betaplustwoalphalikeliness() const;

    /**
     * @return decay
     */
    double betaplusthreealphadecay() const;

    /**
     * @return decay likeliness
     */
    double betaplusthreealphalikeliness() const;

    /**
     * @return decay
     */
    double alphadecay() const;

    /**
     * @return decay
     */
    double alphalikeliness() const;

    /**
     * @return decay
     */
    double alphabetaminusdecay() const;

    /**
     * @return decay
     */
    double alphabetaminuslikeliness() const;

    /**
     * @return decay
     */
    double spontfissiondecay() const;

    /**
     * @return decay
     */
    double spontfissionlikeliness() const;

    /**
     * This enum stores the different kinds of decay
     */
    enum Decay {
        ALPHA /**<alpha decay*/,
        ALPHABETAMINUS,
        BETAPLUS /**<beta plus decay*/,
        BETAMINUS /**<beta minus decay*/,
        EC /**ec decay*/,
        NEUTRON /**neutron decay*/,
        PROTON /**proton decay*/
    };

    /**
     * @return the nucleons of neutrons of the Isotope after the decay
     */
    Isotope::Nucleons nucleonsAfterDecay(Decay kind);

private:
    /**
     * the symbol of the element the isotope belongs to
     */
    ChemicalDataObject m_parentElementSymbol;

    /**
     * stores the information about the mass of the Isotope
     */
    ChemicalDataObject m_mass;

    /**
     * stores the atomicNumber of the Isotope
     */
    ChemicalDataObject m_identifier;

    /**
     * stores the spin of the Isotope
     */
    ChemicalDataObject m_spin;

    /**
     * stores the magneticMoment of the Isotope
     */
    ChemicalDataObject m_magmoment;

    /**
     * stores the relative abundance of the Isotope
     */
    ChemicalDataObject m_abundance;

    /**
     * stores the halfLife of the Isotope
     */
    ChemicalDataObject m_halflife;

    /**
     * stores decay energy of the isotope
     */
    ChemicalDataObject m_alphadecay;
    ChemicalDataObject m_protondecay;
    ChemicalDataObject m_twoprotondecay;
    ChemicalDataObject m_neutrondecay;
    ChemicalDataObject m_twoneutrondecay;
    ChemicalDataObject m_ecdecay;
    ChemicalDataObject m_twoecdecay;
    ChemicalDataObject m_betaminusdecay;
    ChemicalDataObject m_twobetaminusdecay;
    ChemicalDataObject m_betaplusdecay;
    ChemicalDataObject m_twobetaplusdecay;
    ChemicalDataObject m_betaminusneutrondecay;
    ChemicalDataObject m_betaminusfissiondecay;
    ChemicalDataObject m_betaminustwoneutrondecay;
    ChemicalDataObject m_betaminusthreeneutrondecay;
    ChemicalDataObject m_betaminusfourneutrondecay;
    ChemicalDataObject m_betaminusalphaneutrondecay;
    ChemicalDataObject m_betaminusalphadecay;
    ChemicalDataObject m_betaminustwoalphadecay;
    ChemicalDataObject m_betaminusthreealphadecay;
    ChemicalDataObject m_betaplusprotondecay;
    ChemicalDataObject m_betaplustwoprotondecay;
    ChemicalDataObject m_betaplusalphadecay;
    ChemicalDataObject m_betaplustwoalphadecay;
    ChemicalDataObject m_betaplusthreealphadecay;
    ChemicalDataObject m_alphabetaminusdecay;
    ChemicalDataObject m_protonalphadecay;
    ChemicalDataObject m_ecprotondecay;
    ChemicalDataObject m_ectwoprotondecay;
    ChemicalDataObject m_ecthreeprotondecay;
    ChemicalDataObject m_ecalphadecay;
    ChemicalDataObject m_ecalphaprotondecay;
    ChemicalDataObject m_spontfissiondecay;

    /**
     * stores the likeliness of a decay of the isotope
     */
    ChemicalDataObject m_alphalikeliness;
    ChemicalDataObject m_protonlikeliness;
    ChemicalDataObject m_twoprotonlikeliness;
    ChemicalDataObject m_neutronlikeliness;
    ChemicalDataObject m_twoneutronlikeliness;
    ChemicalDataObject m_eclikeliness;
    ChemicalDataObject m_twoeclikeliness;
    ChemicalDataObject m_betaminuslikeliness;
    ChemicalDataObject m_twobetaminuslikeliness;
    ChemicalDataObject m_betapluslikeliness;
    ChemicalDataObject m_twobetapluslikeliness;
    ChemicalDataObject m_betaminusfissionlikeliness;
    ChemicalDataObject m_betaminusneutronlikeliness;
    ChemicalDataObject m_betaminustwoneutronlikeliness;
    ChemicalDataObject m_betaminusthreeneutronlikeliness;
    ChemicalDataObject m_betaminusfourneutronlikeliness;
    ChemicalDataObject m_betaminusalphaneutronlikeliness;
    ChemicalDataObject m_betaminusalphalikeliness;
    ChemicalDataObject m_betaminustwoalphalikeliness;
    ChemicalDataObject m_betaminusthreealphalikeliness;
    ChemicalDataObject m_betaplusprotonlikeliness;
    ChemicalDataObject m_betaplustwoprotonlikeliness;
    ChemicalDataObject m_betaplusalphalikeliness;
    ChemicalDataObject m_betaplustwoalphalikeliness;
    ChemicalDataObject m_betaplusthreealphalikeliness;
    ChemicalDataObject m_alphabetaminuslikeliness;
    ChemicalDataObject m_protonalphalikeliness;
    ChemicalDataObject m_ecprotonlikeliness;
    ChemicalDataObject m_ectwoprotonlikeliness;
    ChemicalDataObject m_ecthreeprotonlikeliness;
    ChemicalDataObject m_ecalphalikeliness;
    ChemicalDataObject m_ecalphaprotonlikeliness;
    ChemicalDataObject m_spontfissionlikeliness;

    int m_numberOfNucleons;
};

#endif // ISOTOPE_H
