/*
    SPDX-FileCopyrightText: 2005 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "isotope.h"


#include "kalzium_libscience_debug.h"

Isotope::Isotope() = default;

Isotope::~Isotope() = default;

void Isotope::addData(const ChemicalDataObject &o)
{
    if (o.type() == ChemicalDataObject::exactMass) {
        m_mass = o;
    } else if (o.type() == ChemicalDataObject::atomicNumber) {
        m_identifier = o;
    } else if (o.type() == ChemicalDataObject::symbol) {
        m_parentElementSymbol = o;
    } else if (o.type() == ChemicalDataObject::spin) {
        m_spin = o;
    } else if (o.type() == ChemicalDataObject::magneticMoment) {
        m_magmoment = o;
    } else if (o.type() == ChemicalDataObject::relativeAbundance) {
        m_abundance = o;
    } else if (o.type() == ChemicalDataObject::halfLife) {
        m_halflife = o;
    } else if (o.type() == ChemicalDataObject::betaminusfissionDecay) {
        m_betaminusfissiondecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusfissionDecayLikeliness) {
        m_betaminusfissionlikeliness = o;
    } else if (o.type() == ChemicalDataObject::alphaDecay) {
        m_alphadecay = o;
    } else if (o.type() == ChemicalDataObject::alphaDecayLikeliness) {
        m_alphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::protonDecay) {
        m_protondecay = o;
    } else if (o.type() == ChemicalDataObject::protonDecayLikeliness) {
        m_protonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::twoprotonDecay) {
        m_twoprotondecay = o;
    } else if (o.type() == ChemicalDataObject::twoprotonDecayLikeliness) {
        m_twoprotonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::neutronDecay) {
        m_neutrondecay = o;
    } else if (o.type() == ChemicalDataObject::neutronDecayLikeliness) {
        m_neutronlikeliness = o;
    } else if (o.type() == ChemicalDataObject::twoneutronDecay) {
        m_twoneutrondecay = o;
    } else if (o.type() == ChemicalDataObject::twoneutronDecayLikeliness) {
        m_twoneutronlikeliness = o;
    } else if (o.type() == ChemicalDataObject::ecDecay) {
        m_ecdecay = o;
    } else if (o.type() == ChemicalDataObject::ecDecayLikeliness) {
        m_eclikeliness = o;
    } else if (o.type() == ChemicalDataObject::twoecDecay) {
        m_twoecdecay = o;
    } else if (o.type() == ChemicalDataObject::twoecDecayLikeliness) {
        m_twoeclikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminusDecay) {
        m_betaminusdecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusDecayLikeliness) {
        m_betaminuslikeliness = o;
    } else if (o.type() == ChemicalDataObject::twobetaminusDecay) {
        m_twobetaminusdecay = o;
    } else if (o.type() == ChemicalDataObject::twobetaminusDecayLikeliness) {
        m_twobetaminuslikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaplusDecay) {
        m_betaplusdecay = o;
    } else if (o.type() == ChemicalDataObject::betaplusDecayLikeliness) {
        m_betapluslikeliness = o;
    } else if (o.type() == ChemicalDataObject::twobetaplusDecay) {
        m_twobetaplusdecay = o;
    } else if (o.type() == ChemicalDataObject::twobetaplusDecayLikeliness) {
        m_twobetapluslikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminusneutronDecay) {
        m_betaminusneutrondecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusneutronDecayLikeliness) {
        m_betaminusneutronlikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminustwoneutronDecay) {
        m_betaminustwoneutrondecay = o;
    } else if (o.type() == ChemicalDataObject::betaminustwoneutronDecayLikeliness) {
        m_betaminustwoneutronlikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminusthreeneutronDecay) {
        m_betaminusthreeneutrondecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusthreeneutronDecayLikeliness) {
        m_betaminusthreeneutronlikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminusfourneutronDecay) {
        m_betaminusfourneutrondecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusfourneutronDecayLikeliness) {
        m_betaminusfourneutronlikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminusalphaneutronDecay) {
        m_betaminusalphaneutrondecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusalphaneutronDecayLikeliness) {
        m_betaminusalphaneutronlikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminusalphaDecay) {
        m_betaminusalphadecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusalphaDecayLikeliness) {
        m_betaminusalphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminustwoalphaDecay) {
        m_betaminustwoalphadecay = o;
    } else if (o.type() == ChemicalDataObject::betaminustwoalphaDecayLikeliness) {
        m_betaminustwoalphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaminusthreealphaDecay) {
        m_betaminusthreealphadecay = o;
    } else if (o.type() == ChemicalDataObject::betaminusthreealphaDecayLikeliness) {
        m_betaminusthreealphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaplusprotonDecay) {
        m_betaplusprotondecay = o;
    } else if (o.type() == ChemicalDataObject::betaplusprotonDecayLikeliness) {
        m_betaplusprotonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaplustwoprotonDecay) {
        m_betaplustwoprotondecay = o;
    } else if (o.type() == ChemicalDataObject::betaplustwoprotonDecayLikeliness) {
        m_betaplustwoprotonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaplusalphaDecay) {
        m_betaplusalphadecay = o;
    } else if (o.type() == ChemicalDataObject::betaplusalphaDecayLikeliness) {
        m_betaplusalphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaplustwoalphaDecay) {
        m_betaplustwoalphadecay = o;
    } else if (o.type() == ChemicalDataObject::betaplustwoalphaDecayLikeliness) {
        m_betaplustwoalphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::betaplusthreealphaDecay) {
        m_betaplusthreealphadecay = o;
    } else if (o.type() == ChemicalDataObject::betaplusthreealphaDecayLikeliness) {
        m_betaplusthreealphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::alphabetaminusDecay) {
        m_alphabetaminusdecay = o;
    } else if (o.type() == ChemicalDataObject::alphabetaminusDecayLikeliness) {
        m_alphabetaminuslikeliness = o;
    } else if (o.type() == ChemicalDataObject::protonalphaDecay) {
        m_protonalphadecay = o;
    } else if (o.type() == ChemicalDataObject::protonalphaDecayLikeliness) {
        m_protonalphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::ecprotonDecay) {
        m_ecprotondecay = o;
    } else if (o.type() == ChemicalDataObject::ecprotonDecayLikeliness) {
        m_ecprotonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::ectwoprotonDecay) {
        m_ectwoprotondecay = o;
    } else if (o.type() == ChemicalDataObject::ectwoprotonDecayLikeliness) {
        m_ectwoprotonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::ecthreeprotonDecay) {
        m_ecthreeprotondecay = o;
    } else if (o.type() == ChemicalDataObject::ecthreeprotonDecayLikeliness) {
        m_ecthreeprotonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::ecalphaDecay) {
        m_ecalphadecay = o;
    } else if (o.type() == ChemicalDataObject::ecalphaDecayLikeliness) {
        m_ecalphalikeliness = o;
    } else if (o.type() == ChemicalDataObject::ecalphaprotonDecay) {
        m_ecalphaprotondecay = o;
    } else if (o.type() == ChemicalDataObject::ecalphaprotonDecayLikeliness) {
        m_ecalphaprotonlikeliness = o;
    } else if (o.type() == ChemicalDataObject::spontfissionDecay) {
        m_spontfissiondecay = o;
    } else if (o.type() == ChemicalDataObject::spontfissionDecayLikeliness) {
        m_spontfissionlikeliness = o;
    }
}

double Isotope::mass() const
{
    return m_mass.value().toDouble();
}

QString Isotope::errorMargin() const
{
    return m_mass.errorValue().toString();
}

int Isotope::parentElementNumber() const
{
    return m_identifier.value().toInt();
}

QString Isotope::spin() const
{
    return m_spin.value().toString();
}

QString Isotope::magmoment() const
{
    return m_magmoment.value().toString();
}

QString Isotope::abundance() const
{
    return m_abundance.value().toString();
}

double Isotope::halflife() const
{
    return m_halflife.value().toDouble();
}

QString Isotope::halflifeUnit() const
{
    return m_halflife.unitAsString();
}

double Isotope::betaminusfissiondecay() const
{
    return m_betaminusfissiondecay.value().toDouble();
}

double Isotope::betaminusfissionlikeliness() const
{
    return m_betaminusfissionlikeliness.value().toDouble();
}

double Isotope::alphadecay() const
{
    return m_alphadecay.value().toDouble();
}
double Isotope::alphalikeliness() const
{
    return m_alphalikeliness.value().toDouble();
}
double Isotope::protondecay() const
{
    return m_protondecay.value().toDouble();
}
double Isotope::protonlikeliness() const
{
    return m_protonlikeliness.value().toDouble();
}
double Isotope::twoprotondecay() const
{
    return m_twoprotondecay.value().toDouble();
}
double Isotope::twoprotonlikeliness() const
{
    return m_twoprotonlikeliness.value().toDouble();
}
double Isotope::neutrondecay() const
{
    return m_neutrondecay.value().toDouble();
}
double Isotope::neutronlikeliness() const
{
    return m_neutronlikeliness.value().toDouble();
}
double Isotope::twoneutrondecay() const
{
    return m_twoneutrondecay.value().toDouble();
}
double Isotope::twoneutronlikeliness() const
{
    return m_twoneutronlikeliness.value().toDouble();
}
double Isotope::ecdecay() const
{
    return m_ecdecay.value().toDouble();
}
double Isotope::eclikeliness() const
{
    return m_eclikeliness.value().toDouble();
}
double Isotope::twoecdecay() const
{
    return m_twoecdecay.value().toDouble();
}
double Isotope::twoeclikeliness() const
{
    return m_twoeclikeliness.value().toDouble();
}
double Isotope::betaminusdecay() const
{
    return m_betaminusdecay.value().toDouble();
}
double Isotope::betaminuslikeliness() const
{
    return m_betaminuslikeliness.value().toDouble();
}
double Isotope::twobetaminusdecay() const
{
    return m_twobetaminusdecay.value().toDouble();
}
double Isotope::twobetaminuslikeliness() const
{
    return m_twobetaminuslikeliness.value().toDouble();
}
double Isotope::betaplusdecay() const
{
    return m_betaplusdecay.value().toDouble();
}
double Isotope::betapluslikeliness() const
{
    return m_betapluslikeliness.value().toDouble();
}
double Isotope::twobetaplusdecay() const
{
    return m_twobetaplusdecay.value().toDouble();
}
double Isotope::twobetapluslikeliness() const
{
    return m_twobetapluslikeliness.value().toDouble();
}
double Isotope::betaminusneutrondecay() const
{
    return m_betaminusneutrondecay.value().toDouble();
}
double Isotope::betaminusneutronlikeliness() const
{
    return m_betaminusneutronlikeliness.value().toDouble();
}
double Isotope::betaminustwoneutrondecay() const
{
    return m_betaminustwoneutrondecay.value().toDouble();
}
double Isotope::betaminustwoneutronlikeliness() const
{
    return m_betaminustwoneutronlikeliness.value().toDouble();
}
double Isotope::betaminusthreeneutrondecay() const
{
    return m_betaminusthreeneutrondecay.value().toDouble();
}
double Isotope::betaminusthreeneutronlikeliness() const
{
    return m_betaminusthreeneutronlikeliness.value().toDouble();
}
double Isotope::betaminusfourneutrondecay() const
{
    return m_betaminusfourneutrondecay.value().toDouble();
}
double Isotope::betaminusfourneutronlikeliness() const
{
    return m_betaminusfourneutronlikeliness.value().toDouble();
}
double Isotope::betaminusalphaneutrondecay() const
{
    return m_betaminusalphaneutrondecay.value().toDouble();
}
double Isotope::betaminusalphaneutronlikeliness() const
{
    return m_betaminusalphaneutronlikeliness.value().toDouble();
}
double Isotope::betaminusalphadecay() const
{
    return m_betaminusalphadecay.value().toDouble();
}
double Isotope::betaminusalphalikeliness() const
{
    return m_betaminusalphalikeliness.value().toDouble();
}
double Isotope::betaminustwoalphadecay() const
{
    return m_betaminustwoalphadecay.value().toDouble();
}
double Isotope::betaminustwoalphalikeliness() const
{
    return m_betaminustwoalphalikeliness.value().toDouble();
}
double Isotope::betaminusthreealphadecay() const
{
    return m_betaminusthreealphadecay.value().toDouble();
}
double Isotope::betaminusthreealphalikeliness() const
{
    return m_betaminusthreealphalikeliness.value().toDouble();
}
double Isotope::betaplusprotondecay() const
{
    return m_betaplusprotondecay.value().toDouble();
}
double Isotope::betaplusprotonlikeliness() const
{
    return m_betaplusprotonlikeliness.value().toDouble();
}
double Isotope::betaplustwoprotondecay() const
{
    return m_betaplustwoprotondecay.value().toDouble();
}
double Isotope::betaplustwoprotonlikeliness() const
{
    return m_betaplustwoprotonlikeliness.value().toDouble();
}
double Isotope::betaplusalphadecay() const
{
    return m_betaplusalphadecay.value().toDouble();
}
double Isotope::betaplusalphalikeliness() const
{
    return m_betaplusalphalikeliness.value().toDouble();
}
double Isotope::betaplustwoalphadecay() const
{
    return m_betaplustwoalphadecay.value().toDouble();
}
double Isotope::betaplustwoalphalikeliness() const
{
    return m_betaplustwoalphalikeliness.value().toDouble();
}
double Isotope::betaplusthreealphadecay() const
{
    return m_betaplusthreealphadecay.value().toDouble();
}
double Isotope::betaplusthreealphalikeliness() const
{
    return m_betaplusthreealphalikeliness.value().toDouble();
}
double Isotope::alphabetaminusdecay() const
{
    return m_alphabetaminusdecay.value().toDouble();
}
double Isotope::alphabetaminuslikeliness() const
{
    return m_alphabetaminuslikeliness.value().toDouble();
}
double Isotope::protonalphadecay() const
{
    return m_protonalphadecay.value().toDouble();
}
double Isotope::protonalphalikeliness() const
{
    return m_protonalphalikeliness.value().toDouble();
}
double Isotope::ecprotondecay() const
{
    return m_ecprotondecay.value().toDouble();
}
double Isotope::ecprotonlikeliness() const
{
    return m_ecprotonlikeliness.value().toDouble();
}
double Isotope::ectwoprotondecay() const
{
    return m_ectwoprotondecay.value().toDouble();
}
double Isotope::ectwoprotonlikeliness() const
{
    return m_ectwoprotonlikeliness.value().toDouble();
}
double Isotope::ecthreeprotondecay() const
{
    return m_ecthreeprotondecay.value().toDouble();
}
double Isotope::ecthreeprotonlikeliness() const
{
    return m_ecthreeprotonlikeliness.value().toDouble();
}
double Isotope::ecalphadecay() const
{
    return m_ecalphadecay.value().toDouble();
}
double Isotope::ecalphalikeliness() const
{
    return m_ecalphalikeliness.value().toDouble();
}
double Isotope::ecalphaprotondecay() const
{
    return m_ecalphaprotondecay.value().toDouble();
}
double Isotope::ecalphaprotonlikeliness() const
{
    return m_ecalphaprotonlikeliness.value().toDouble();
}
double Isotope::spontfissiondecay() const
{
    return m_spontfissiondecay.value().toDouble();
}
double Isotope::spontfissionlikeliness() const
{
    return m_spontfissionlikeliness.value().toDouble();
}

QString Isotope::parentElementSymbol() const
{
    return m_parentElementSymbol.value().toString();
}

void Isotope::setNucleons(int number)
{
    m_numberOfNucleons = number;
}

int Isotope::nucleons() const
{
    return m_numberOfNucleons;
}

Isotope::Nucleons Isotope::nucleonsAfterDecay(Decay kind)
{
    Isotope::Nucleons n;
    int protons = m_identifier.value().toInt();
    int neutrons = m_numberOfNucleons - protons;
    n.protons = protons;
    n.neutrons = neutrons;

    switch (kind) {
    case ALPHA:
        n.protons -= 2;
        break;
    case ALPHABETAMINUS:
        n.protons -= 1;
        n.neutrons -= 1;
        break;
    case BETAMINUS:
        n.protons += 1;
        n.neutrons -= 1;
        break;
    case BETAPLUS:
        n.protons -= 1;
        break;
    case EC:
        n.protons -= 1;
        n.neutrons += 1;
        break;
    case NEUTRON:
        n.neutrons -= 1;
        break;
    case PROTON:
        n.protons -= 1;
        break;
    }

    return n;
}
