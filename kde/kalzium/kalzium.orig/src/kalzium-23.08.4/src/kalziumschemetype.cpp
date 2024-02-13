/*
    SPDX-FileCopyrightText: 2005, 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalziumschemetype.h"

#include "kalziumdataobject.h"
#include "prefs.h"

#ifdef HAVE_OPENBABEL2
#include <openbabel/mol.h>
#endif
#ifdef HAVE_OPENBABEL3
#include <openbabel/elements.h>
#endif

#include "kalzium_debug.h"
#include <QStandardPaths>

#include <KLocalizedString>

KalziumSchemeTypeFactory::KalziumSchemeTypeFactory()
{
    m_schemes << KalziumMonoColorSchemeType::instance();
    m_schemes << KalziumBlocksSchemeType::instance();
    m_schemes << KalziumIconicSchemeType::instance();
    m_schemes << KalziumFamilySchemeType::instance();
    m_schemes << KalziumGroupsSchemeType::instance();
#ifdef HAVE_OPENBABEL
    m_schemes << KalziumColorSchemeType::instance();
#endif
}

KalziumSchemeTypeFactory *KalziumSchemeTypeFactory::instance()
{
    static KalziumSchemeTypeFactory kstf;
    return &kstf;
}

KalziumSchemeType *KalziumSchemeTypeFactory::build(int id) const
{
    if ((id < 0) || (id >= m_schemes.count())) {
        return nullptr;
    }

    return m_schemes.at(id);
}

KalziumSchemeType *KalziumSchemeTypeFactory::build(const QByteArray &id) const
{
    for (int i = 0; i < m_schemes.count(); ++i) {
        if (m_schemes.at(i)->name() == id) {
            return m_schemes.at(i);
        }
    }

    return nullptr;
}

QStringList KalziumSchemeTypeFactory::schemes() const
{
    QStringList l;
    for (int i = 0; i < m_schemes.count(); ++i) {
        l << m_schemes.at(i)->description();
    }
    return l;
}

KalziumSchemeType *KalziumSchemeType::instance()
{
    return nullptr;
}

KalziumSchemeType::KalziumSchemeType() = default;

KalziumSchemeType::~KalziumSchemeType() = default;

KalziumMonoColorSchemeType::KalziumMonoColorSchemeType()
    : KalziumSchemeType()
{
}

KalziumMonoColorSchemeType *KalziumMonoColorSchemeType::instance()
{
    static KalziumMonoColorSchemeType kmcst;
    return &kmcst;
}

QByteArray KalziumMonoColorSchemeType::name() const
{
    return "MonoColor";
}

QString KalziumMonoColorSchemeType::description() const
{
    return i18n("Monochrome");
}

QBrush KalziumMonoColorSchemeType::elementBrush(int el) const
{
    Q_UNUSED(el);
    return QBrush(Prefs::noscheme());
}

QColor KalziumMonoColorSchemeType::textColor(int el) const
{
    Q_UNUSED(el);
    return Qt::black;
}

QList<legendPair> KalziumMonoColorSchemeType::legendItems() const
{
    QList<legendPair> ll;
    ll << qMakePair(i18n("All the Elements"), QColor(Prefs::noscheme()));
    return ll;
}

KalziumBlocksSchemeType::KalziumBlocksSchemeType()
    : KalziumSchemeType()
{
}

KalziumBlocksSchemeType *KalziumBlocksSchemeType::instance()
{
    static KalziumBlocksSchemeType kbst;
    return &kbst;
}

QByteArray KalziumBlocksSchemeType::name() const
{
    return "Blocks";
}

QString KalziumBlocksSchemeType::description() const
{
    return i18n("Blocks");
}

QBrush KalziumBlocksSchemeType::elementBrush(int el) const
{
    QString block = KalziumDataObject::instance()->element(el)->dataAsString(ChemicalDataObject::periodTableBlock);

    QColor c;
    if (block == QLatin1String("s")) {
        c = Prefs::block_s();
    } else if (block == QLatin1String("p")) {
        c = Prefs::block_p();
    } else if (block == QLatin1String("d")) {
        c = Prefs::block_d();
    } else if (block == QLatin1String("f")) {
        c = Prefs::block_f();
    } else {
        c = Qt::lightGray;
    }

    return QBrush(c);
}

QColor KalziumBlocksSchemeType::textColor(int el) const
{
    Q_UNUSED(el);
    return Qt::black;
}

QList<legendPair> KalziumBlocksSchemeType::legendItems() const
{
    QList<legendPair> ll;
    ll << qMakePair(i18n("s-Block"), QColor(Prefs::block_s()));
    ll << qMakePair(i18n("p-Block"), QColor(Prefs::block_p()));
    ll << qMakePair(i18n("d-Block"), QColor(Prefs::block_d()));
    ll << qMakePair(i18n("f-Block"), QColor(Prefs::block_f()));
    return ll;
}

/// ICONIC SCHEME///

KalziumIconicSchemeType::KalziumIconicSchemeType()
    : KalziumSchemeType()
{
}

KalziumIconicSchemeType *KalziumIconicSchemeType::instance()
{
    static KalziumIconicSchemeType kist;
    return &kist;
}

QByteArray KalziumIconicSchemeType::name() const
{
    return "Iconic";
}

QString KalziumIconicSchemeType::description() const
{
    return i18n("Iconic");
}

QBrush KalziumIconicSchemeType::elementBrush(int el) const
{
    QPixmap pixmap = KalziumDataObject::instance()->pixmap(el);
    return QBrush(pixmap);
}

QColor KalziumIconicSchemeType::textColor(int) const
{
    return Qt::transparent;
}

QList<legendPair> KalziumIconicSchemeType::legendItems() const
{
    QList<legendPair> ll;
    ll << qMakePair(i18n("Each element is represented by an icon which represents its use."), QColor());
    return ll;
}

/// Family///
KalziumFamilySchemeType::KalziumFamilySchemeType()
    : KalziumSchemeType()
{
}

KalziumFamilySchemeType *KalziumFamilySchemeType::instance()
{
    static KalziumFamilySchemeType kbst;
    return &kbst;
}

QByteArray KalziumFamilySchemeType::name() const
{
    return "Family";
}

QString KalziumFamilySchemeType::description() const
{
    return i18n("Family");
}

QBrush KalziumFamilySchemeType::elementBrush(int el) const
{
    QString family = KalziumDataObject::instance()->element(el)->dataAsString(ChemicalDataObject::family);

    QColor c;

    if (family == QLatin1String("Noblegas")) {
        c = Prefs::noble_gas();
    } else if (family == QLatin1String("Non-Metal")) {
        c = Prefs::nonmetal();
    } else if (family == QLatin1String("Rare_Earth")) {
        c = Prefs::rare();
    } else if (family == QLatin1String("Alkaline_Earth")) {
        c = Prefs::alkaline();
    } else if (family == QLatin1String("Alkali_Earth")) {
        c = Prefs::alkalie();
    } else if (family == QLatin1String("Transition")) {
        c = Prefs::transition();
    } else if (family == QLatin1String("Other_Metal")) {
        c = Prefs::other_metal();
    } else if (family == QLatin1String("Metalloids")) {
        c = Prefs::metalloid();
    } else if (family == QLatin1String("Halogen")) {
        c = Prefs::halogene();
    } else {
        c = Qt::lightGray;
    }

    return QBrush(c);
}

QColor KalziumFamilySchemeType::textColor(int) const
{
    return Qt::black;
}

QList<legendPair> KalziumFamilySchemeType::legendItems() const
{
    QList<legendPair> ll;
    ll << qMakePair(i18n("Alkaline"), QColor(Prefs::alkalie()));
    ll << qMakePair(i18n("Rare Earth"), QColor(Prefs::rare()));
    ll << qMakePair(i18n("Non-Metals"), QColor(Prefs::nonmetal()));
    ll << qMakePair(i18n("Alkalie Metal"), QColor(Prefs::alkaline()));
    ll << qMakePair(i18n("Other Metal"), QColor(Prefs::other_metal()));
    ll << qMakePair(i18n("Halogen"), QColor(Prefs::halogene()));
    ll << qMakePair(i18n("Transition Metal"), QColor(Prefs::transition()));
    ll << qMakePair(i18n("Noble Gas"), QColor(Prefs::noble_gas()));
    ll << qMakePair(i18n("Metalloid"), QColor(Prefs::metalloid()));

    return ll;
}

/// GROUPS///
KalziumGroupsSchemeType::KalziumGroupsSchemeType()
    : KalziumSchemeType()
{
}

KalziumGroupsSchemeType *KalziumGroupsSchemeType::instance()
{
    static KalziumGroupsSchemeType kbst;
    return &kbst;
}

QByteArray KalziumGroupsSchemeType::name() const
{
    return "Groups";
}

QString KalziumGroupsSchemeType::description() const
{
    return i18n("Groups");
}

QBrush KalziumGroupsSchemeType::elementBrush(int el) const
{
    QString group = KalziumDataObject::instance()->element(el)->dataAsString(ChemicalDataObject::group);

    QColor c;

    if (group == QLatin1String("1")) {
        c = Prefs::group_1();
    } else if (group == QLatin1String("2")) {
        c = Prefs::group_2();
    } else if (group == QLatin1String("3")) {
        c = Prefs::group_3();
    } else if (group == QLatin1String("4")) {
        c = Prefs::group_4();
    } else if (group == QLatin1String("5")) {
        c = Prefs::group_5();
    } else if (group == QLatin1String("6")) {
        c = Prefs::group_6();
    } else if (group == QLatin1String("7")) {
        c = Prefs::group_7();
    } else if (group == QLatin1String("8")) {
        c = Prefs::group_8();
    } else {
        c = Qt::lightGray;
    }

    return QBrush(c);
}

QColor KalziumGroupsSchemeType::textColor(int) const
{
    return Qt::black;
}

QList<legendPair> KalziumGroupsSchemeType::legendItems() const
{
    QList<legendPair> ll;
    ll << qMakePair(i18n("Group 1"), QColor(Prefs::group_1()));
    ll << qMakePair(i18n("Group 2"), QColor(Prefs::group_2()));
    ll << qMakePair(i18n("Group 3"), QColor(Prefs::group_3()));
    ll << qMakePair(i18n("Group 4"), QColor(Prefs::group_4()));
    ll << qMakePair(i18n("Group 5"), QColor(Prefs::group_5()));
    ll << qMakePair(i18n("Group 6"), QColor(Prefs::group_6()));
    ll << qMakePair(i18n("Group 7"), QColor(Prefs::group_7()));
    ll << qMakePair(i18n("Group 8"), QColor(Prefs::group_8()));

    return ll;
}

#ifdef HAVE_OPENBABEL
/// OpenBabel Color///
KalziumColorSchemeType::KalziumColorSchemeType()
    : KalziumSchemeType()
{
}

KalziumColorSchemeType *KalziumColorSchemeType::instance()
{
    static KalziumColorSchemeType kbst;
    return &kbst;
}

QByteArray KalziumColorSchemeType::name() const
{
    return "Color";
}

QString KalziumColorSchemeType::description() const
{
    return i18n("Colors");
}

QBrush KalziumColorSchemeType::elementBrush(int el) const
{
    QColor c;

#ifdef HAVE_OPENBABEL2
    std::vector<double> color = OpenBabel::etab.GetRGB(el);
    c.setRgbF(color[0], color[1], color[2]);
#endif
#ifdef HAVE_OPENBABEL3
    double red, green, blue;
    OpenBabel::OBElements::GetRGB(el, &red, &green, &blue);
    c.setRgbF(red, green, blue);
#endif

    return QBrush(c);
}

QColor KalziumColorSchemeType::textColor(int) const
{
    return Qt::black;
}

QList<legendPair> KalziumColorSchemeType::legendItems() const
{
    QList<legendPair> ll;
    ll << qMakePair(i18n("Nice colors without meaning. (From the Open Babel project)"), QColor());
    return ll;
}
#endif

/// CRYSTAL///
// X KalziumCrystalSchemeType::KalziumCrystalSchemeType()
// X   : KalziumSchemeType()
// X {
// X }
// X
// X KalziumCrystalSchemeType* KalziumCrystalSchemeType::instance()
// X {
// X     static KalziumCrystalSchemeType kbst;
// X     return &kbst;
// X }
// X
// X QByteArray KalziumCrystalSchemeType::name() const
// X {
// X     return "Crystal";
// X }
// X
// X QString KalziumCrystalSchemeType::description() const
// X {
// X     return i18n("Crystal Structures");
// X }
// X
// X QBrush KalziumCrystalSchemeType::elementBrush(int el, const QRect& elrect) const
// X {
// X     QString crystal = KalziumDataObject::instance()->element(el)->dataAsString(ChemicalDataObject::crystalstructure);
// X
// X     qCDebug(KALZIUM_LOG) << "crystal is " << crystal;
// X
// X     static QString resourcepath;
// X     if (resourcepath.isEmpty()) {
// X         resourcepath = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "data/latticeicons/");
// X     }
// X
// X     QString filename;
// X     if (crystal == "bcc") {
// X         filename = "ci.png";
// X     } else if (crystal == "ccp") {
// X         filename = "cp.png";
// X     } else if (crystal ==  "fcc") {
// X         filename = "cf.png";
// X     } else if (crystal ==  "hcp") {
// X         filename = "hp.png";
// X     } else if (crystal ==  "rh") {
// X         filename = "hr.png";//Rhombohedral primitive
// X     } else if (crystal ==  "or") {
// X         filename = "op.png";//Orthorhombic primitive
// X     } else if (crystal ==  "mono") {
// X         filename = "ms.png";//Monoclinic primitive
// X     } else if (crystal ==  "tri") {
// X         filename = "ap.png";//Triclinic
// X     } else if (crystal ==  "tp") {
// X         filename = "tp.png";//Tetragonal primitive
// X     }
// X
// X         filename.prepend(resourcepath);
// X
// X     QBrush ret;
// X     if (!filename.isEmpty()) {
// X         qCDebug(KALZIUM_LOG) << el << ": FILENAME is not EMPTY... " << filename;
// X         QPixmap pixmap(resourcepath + filename);
// X         ret = QBrush(pixmap.scaled(elrect.size(), Qt::KeepAspectRatio));
// X     } else {
// X             qCDebug(KALZIUM_LOG) << el << ": FILENAME EMPTY... " << filename;
// X         ret.setColor(Qt::gray);
// X     }
// X
// X     return ret;
// X }
// X
// X QColor KalziumCrystalSchemeType::textColor(int) const
// X {
// X     return Qt::black;
// X }
// X
// X QList<legendPair> KalziumCrystalSchemeType::legendItems() const
// X {
// X     static QString resourcepath;
// X     if (resourcepath.isEmpty())
// X     {
// X         resourcepath = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "data/latticeicons/");
// X     }
// X
// X     QList<legendPair> ll;
// X     ll << qMakePair(i18n("bcc, body centered cubic"), QColor(QPixmap(resourcepath + "ci.png")));
// X     ll << qMakePair(i18n("ccp, cubic close packed"), QColor(QPixmap(resourcepath + "cp.png")));
// X     ll << qMakePair(i18n("fcc, face centered cubic"), QColor(QPixmap(resourcepath + "cf.png")));
// X     ll << qMakePair(i18n("hcp, hexagonal close packed"), QColor(QPixmap(resourcepath + "hp.png")));
// X     ll << qMakePair(i18n("rh, rhombohedral"), QColor(QPixmap(resourcepath + "hr.png")));
// X     ll << qMakePair(i18n("or, orthorhombic primitive"), QColor(QPixmap(resourcepath + "op.png")));
// X     ll << qMakePair(i18n("ms, monoclinic"), QColor(QPixmap(resourcepath + "ms.png")));
// X     ll << qMakePair(i18n("ap, triclinic"), QColor(QPixmap(resourcepath + "ap.png")));
// X     ll << qMakePair(i18n("tp, tetragonal primitive"), QColor(QPixmap(resourcepath + "tp.png")));
// X
// X     return ll;
// X }

////
// X KalziumDiscoverymapSchemeType::KalziumDiscoverymapSchemeType()
// X   : KalziumSchemeType()
// X {
// X }
// X
// X KalziumDiscoverymapSchemeType* KalziumDiscoverymapSchemeType::instance()
// X {
// X     static KalziumDiscoverymapSchemeType kbst;
// X     return &kbst;
// X }
// X
// X QByteArray KalziumDiscoverymapSchemeType::name() const
// X {
// X     return "Crystal";
// X }
// X
// X QString KalziumDiscoverymapSchemeType::description() const
// X {
// X     return i18n("Discovery Country");
// X }
// X
// X QBrush KalziumDiscoverymapSchemeType::elementBrush(int el, const QRect& elrect) const
// X {
// X     QString map = KalziumDataObject::instance()->element(el)->dataAsString(ChemicalDataObject::discoveryCountry);
// X
// X     static QString resourcepath;
// X     if (resourcepath.isEmpty()) {
// X         resourcepath = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "data/maps/");
// X     }
// X
// X     QString filename;
// X     if (map == "se") {
// X         filename = "se.png";
// X     } else if (map == "uk") {
// X         filename = "uk.png";
// X     } else if (map == "us") {
// X         filename = "us.png";
// X     } else if (map == "ru") {
// X         filename = "ru.png";
// X     } else if (map == "it") {
// X         filename = "it.png";
// X     } else if (map == "de") {
// X         filename = "de.png";
// X     } else if (map == "dk") {
// X         filename = "dk.png";
// X     } else if (map == "fr") {
// X         filename = "fr.png";
// X     } else if (map == "fi") {
// X         filename = "fi.png";
// X     } else if (map == "es") {
// X         filename = "es.png";
// X     } else if (map == "ancient") {
// X         return QBrush(Qt::lightGray);
// X     } else if (map == "uk,fr") {
// X         filename = "ukfr.png";
// X     } else if (map == "se,uk") {
// X         filename = "ukse.png";
// X     } else if (map == "ru,us") {
// X         filename = "ruus.png";
// X     } else {
// X       return QBrush(Qt::blue);
// X     }
// X
// X     QBrush ret;
// X     if (!filename.isEmpty()) {
// X         QPixmap pixmap(resourcepath + filename);
// X         ret = QBrush(pixmap.scaled(elrect.size(), Qt::KeepAspectRatio));
// X     } else {
// X         ret.setColor(Qt::gray);
// X     }
// X
// X     return ret;
// X }
// X
// X QColor KalziumDiscoverymapSchemeType::textColor(int) const
// X {
// X     return Qt::black;
// X }
// X
// X QList<legendPair> KalziumDiscoverymapSchemeType::legendItems() const
// X {
// X     static QString resourcepath;
// X     if (resourcepath.isEmpty()) {
// X         resourcepath = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "data/maps/");
// X     }
// X
// X     QList<legendPair> ll;
// X     ll << qMakePair(i18n("Germany"), QColor(QPixmap(resourcepath + "de.png")));
// X     ll << qMakePair(i18n("United Kindom"), QColor(QPixmap(resourcepath + "uk.png")));
// X     ll << qMakePair(i18n("Sweden"), QColor(QPixmap(resourcepath + "se.png")));
// X     ll << qMakePair(i18n("USA"), QColor(QPixmap(resourcepath + "us.png")));
// X     ll << qMakePair(i18n("Russia"), QColor(QPixmap(resourcepath + "ru.png")));
// X     ll << qMakePair(i18n("Italy"), QColor(QPixmap(resourcepath + "it.png")));
// X     ll << qMakePair(i18n("Denmark"), QColor(QPixmap(resourcepath + "dk.png")));
// X     ll << qMakePair(i18n("France"), QColor(QPixmap(resourcepath + "fr.png")));
// X     ll << qMakePair(i18n("Finland"), QColor(QPixmap(resourcepath + "fi.png")));
// X     ll << qMakePair(i18n("Spain"), QColor(QPixmap(resourcepath + "es.png")));
// X
// X     return ll;
// X }
