/*
    SPDX-FileCopyrightText: 2005, 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalziumnumerationtype.h"

#include "kalziumdataobject.h"

#include <KLocalizedString>

KalziumNumerationTypeFactory::KalziumNumerationTypeFactory()
{
    m_numerations << KalziumNoneNumerationType::instance();
    m_numerations << KalziumIUPACNumerationType::instance();
    m_numerations << KalziumCASNumerationType::instance();
    m_numerations << KalziumOldIUPACNumerationType::instance();
}

KalziumNumerationTypeFactory *KalziumNumerationTypeFactory::instance()
{
    static KalziumNumerationTypeFactory kntf;
    return &kntf;
}

KalziumNumerationType *KalziumNumerationTypeFactory::build(int id) const
{
    if ((id < 0) || (id >= m_numerations.count())) {
        return nullptr;
    }

    return m_numerations.at(id);
}

KalziumNumerationType *KalziumNumerationTypeFactory::build(const QByteArray &id) const
{
    for (int i = 0; i < m_numerations.count(); ++i) {
        if (m_numerations.at(i)->name() == id) {
            return m_numerations.at(i);
        }
    }

    // not found
    return nullptr;
}

QStringList KalziumNumerationTypeFactory::numerations() const
{
    QStringList l;
    for (int i = 0; i < m_numerations.count(); ++i) {
        l << m_numerations.at(i)->description();
    }
    return l;
}

KalziumNumerationType *KalziumNumerationType::instance()
{
    return nullptr;
}

KalziumNumerationType::KalziumNumerationType() = default;

KalziumNumerationType::~KalziumNumerationType() = default;

QString KalziumNumerationType::item(const int num) const
{
    if ((num < 0) || (num >= m_items.count())) {
        return {};
    }

    return m_items.at(num);
}

QStringList KalziumNumerationType::items() const
{
    return m_items;
}

KalziumNoneNumerationType *KalziumNoneNumerationType::instance()
{
    static KalziumNoneNumerationType knnt;
    return &knnt;
}

KalziumNoneNumerationType::KalziumNoneNumerationType()
    : KalziumNumerationType()
{
}

QByteArray KalziumNoneNumerationType::name() const
{
    return "None";
}

QString KalziumNoneNumerationType::description() const
{
    return i18n("No Numeration");
}

QString KalziumNoneNumerationType::item(const int num) const
{
    Q_UNUSED(num);
    return {};
}

QStringList KalziumNoneNumerationType::items() const
{
    return {};
}

KalziumIUPACNumerationType *KalziumIUPACNumerationType::instance()
{
    static KalziumIUPACNumerationType kint;
    return &kint;
}

KalziumIUPACNumerationType::KalziumIUPACNumerationType()
    : KalziumNumerationType()
{
    // cache them
    m_items << QStringLiteral("1");
    m_items << QStringLiteral("2");
    m_items << QStringLiteral("3");
    m_items << QStringLiteral("4");
    m_items << QStringLiteral("5");
    m_items << QStringLiteral("6");
    m_items << QStringLiteral("7");
    m_items << QStringLiteral("8");
    m_items << QStringLiteral("9");
    m_items << QStringLiteral("10");
    m_items << QStringLiteral("11");
    m_items << QStringLiteral("12");
    m_items << QStringLiteral("13");
    m_items << QStringLiteral("14");
    m_items << QStringLiteral("15");
    m_items << QStringLiteral("16");
    m_items << QStringLiteral("17");
    m_items << QStringLiteral("18");
}

QByteArray KalziumIUPACNumerationType::name() const
{
    return "IUPAC";
}

QString KalziumIUPACNumerationType::description() const
{
    return i18n("IUPAC");
}

KalziumCASNumerationType *KalziumCASNumerationType::instance()
{
    static KalziumCASNumerationType kcnt;
    return &kcnt;
}

KalziumCASNumerationType::KalziumCASNumerationType()
    : KalziumNumerationType()
{
    // cache them
    m_items << QStringLiteral("IA");
    m_items << QStringLiteral("IIA");
    m_items << QStringLiteral("IIIB");
    m_items << QStringLiteral("IVB");
    m_items << QStringLiteral("VB");
    m_items << QStringLiteral("VIB");
    m_items << QStringLiteral("VIIB");
    m_items << QStringLiteral("VIII");
    m_items << QStringLiteral("VIII");
    m_items << QStringLiteral("VIII");
    m_items << QStringLiteral("IB");
    m_items << QStringLiteral("IIB");
    m_items << QStringLiteral("IIIA");
    m_items << QStringLiteral("IVA");
    m_items << QStringLiteral("VA");
    m_items << QStringLiteral("VIA");
    m_items << QStringLiteral("VIIA");
    m_items << QStringLiteral("VIIIA");
}

QByteArray KalziumCASNumerationType::name() const
{
    return "CAS";
}

QString KalziumCASNumerationType::description() const
{
    return i18n("CAS");
}

KalziumOldIUPACNumerationType *KalziumOldIUPACNumerationType::instance()
{
    static KalziumOldIUPACNumerationType koint;
    return &koint;
}

KalziumOldIUPACNumerationType::KalziumOldIUPACNumerationType()
    : KalziumNumerationType()
{
    // cache them
    m_items << QStringLiteral("1A");
    m_items << QStringLiteral("2A");
    m_items << QStringLiteral("3A");
    m_items << QStringLiteral("4A");
    m_items << QStringLiteral("5A");
    m_items << QStringLiteral("6A");
    m_items << QStringLiteral("7A");
    m_items << QStringLiteral("8");
    m_items << QStringLiteral("8");
    m_items << QStringLiteral("8");
    m_items << QStringLiteral("1B");
    m_items << QStringLiteral("2B");
    m_items << QStringLiteral("3B");
    m_items << QStringLiteral("4B");
    m_items << QStringLiteral("5B");
    m_items << QStringLiteral("6B");
    m_items << QStringLiteral("7B");
    m_items << QStringLiteral("0");
}

QByteArray KalziumOldIUPACNumerationType::name() const
{
    return "OldIUPAC";
}

QString KalziumOldIUPACNumerationType::description() const
{
    return i18n("Old IUPAC");
}
