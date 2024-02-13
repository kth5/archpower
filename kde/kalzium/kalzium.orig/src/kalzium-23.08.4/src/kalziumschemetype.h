/*
    SPDX-FileCopyrightText: 2005, 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMSCHEMETYPE_H
#define KALZIUMSCHEMETYPE_H

class KalziumSchemeType;

#include "kalziumgradienttype.h"

#include <config-kalzium.h>

#include <QBrush>
#include <QByteArray>
#include <QColor>
#include <QPair>
#include <QRect>

using legendPair = QPair<QString, QColor>;

/**
 * Factory for KalziumSchemeType classes.
 *
 * @author Pino Toscano
 */
class KalziumSchemeTypeFactory
{
public:
    /**
     * Get the instance of this factory.
     */
    static KalziumSchemeTypeFactory *instance();

    /**
     * Returns the KalziumSchemeType with the @p id specified.
     * It will gives 0 if none found.
     */
    KalziumSchemeType *build(int id) const;
    /**
     * Returns the KalziumSchemeType whose name is the @p id
     * specified.
     * It will gives 0 if none found.
     */
    KalziumSchemeType *build(const QByteArray &id) const;

    /**
     * Returns a list with the names of the schemes we support.
     */
    QStringList schemes() const;

private:
    KalziumSchemeTypeFactory();
    QList<KalziumSchemeType *> m_schemes;
};

/**
 * This is the base class representing a colour scheme.
 *
 * @author Pino Toscano
 */
class KalziumSchemeType
{
public:
    /**
     * Get its instance.
     */
    static KalziumSchemeType *instance();

    virtual ~KalziumSchemeType();

    /**
     * Returns the ID of this scheme.
     * Mainly used when saving/loading.
     */
    virtual QByteArray name() const = 0;
    /**
     * Returns the description of this scheme.
     * Used in all the visible places.
     */
    virtual QString description() const = 0;

    /**
     * Returns the brush with which the element with number
     * @p el should be painted.
     *
     * @param el Element number
     * @param elrect is the rect designed for the element
     *
     * @return A brush for painting in @p elrect
     */
    virtual QBrush elementBrush(int el) const = 0;
    /**
     * Returns the color which will be used to draw the texts for
     * the element with atomic number @p el.
     */
    virtual QColor textColor(int el) const = 0;

    /**
     * Returns a list with the legend of the current scheme.
     */
    virtual QList<legendPair> legendItems() const = 0;

protected:
    KalziumSchemeType();
};

/**
 * Class representing the all-one-colour scheme.
 *
 * @author Pino Toscano
 */
class KalziumMonoColorSchemeType : public KalziumSchemeType
{
public:
    static KalziumMonoColorSchemeType *instance();

    QByteArray name() const override;
    QString description() const override;

    QBrush elementBrush(int el) const override;
    QColor textColor(int el) const override;

    QList<legendPair> legendItems() const override;

private:
    KalziumMonoColorSchemeType();
};

/**
 * The scheme by element block.
 *
 * @author Pino Toscano
 */
class KalziumBlocksSchemeType : public KalziumSchemeType
{
public:
    static KalziumBlocksSchemeType *instance();

    QByteArray name() const override;
    QString description() const override;

    QBrush elementBrush(int el) const override;
    QColor textColor(int el) const override;

    QList<legendPair> legendItems() const override;

private:
    KalziumBlocksSchemeType();
};

/**
 * The scheme for iconic representation.
 *
 * @author Carsten Niehaus
 */
class KalziumIconicSchemeType : public KalziumSchemeType
{
public:
    static KalziumIconicSchemeType *instance();

    QByteArray name() const override;
    QString description() const override;

    QBrush elementBrush(int el) const override;
    QColor textColor(int el) const override;

    QList<legendPair> legendItems() const override;

private:
    KalziumIconicSchemeType();
};

/**
 * The scheme for family representation.
 *
 * @author Carsten Niehaus
 */
class KalziumFamilySchemeType : public KalziumSchemeType
{
public:
    static KalziumFamilySchemeType *instance();

    QByteArray name() const override;
    QString description() const override;

    QBrush elementBrush(int el) const override;
    QColor textColor(int el) const override;

    QList<legendPair> legendItems() const override;

private:
    KalziumFamilySchemeType();
};

/**
 * The scheme for groups representation.
 *
 * @author Carsten Niehaus
 */
class KalziumGroupsSchemeType : public KalziumSchemeType
{
public:
    static KalziumGroupsSchemeType *instance();

    QByteArray name() const override;
    QString description() const override;

    QBrush elementBrush(int el) const override;
    QColor textColor(int el) const override;

    QList<legendPair> legendItems() const override;

private:
    KalziumGroupsSchemeType();
};

#ifdef HAVE_OPENBABEL
/**
 * The scheme for color
 *
 * @author Etienne Rebetez
 */
class KalziumColorSchemeType : public KalziumSchemeType
{
public:
    static KalziumColorSchemeType *instance();

    QByteArray name() const override;
    QString description() const override;

    QBrush elementBrush(int el) const override;
    QColor textColor(int el) const override;

    QList<legendPair> legendItems() const override;

private:
    KalziumColorSchemeType();
};
#endif

// X /**
// X  * The scheme for the crystal structures.
// X  *
// X  * @author Carsten Niehaus
// X  */
// X class KalziumCrystalSchemeType : public KalziumSchemeType
// X {
// X     public:
// X         static KalziumCrystalSchemeType* instance();
// X
// X         QByteArray name() const;
// X         QString description() const;
// X
// X         QBrush elementBrush(int el, const QRect& elrect) const;
// X         QColor textColor(int el) const;
// X
// X         QList<legendPair> legendItems() const;
// X
// X     private:
// X         KalziumCrystalSchemeType();
// X };

// X /**
// X  * @author Carsten Niehaus
// X  */
// X class KalziumDiscoverymapSchemeType : public KalziumSchemeType
// X {
// X     public:
// X         static KalziumDiscoverymapSchemeType* instance();
// X
// X         QByteArray name() const;
// X         QString description() const;
// X
// X         QBrush elementBrush(int el, const QRect& elrect) const;
// X         QColor textColor(int el) const;
// X
// X         QList<legendPair> legendItems() const;
// X
// X     private:
// X         KalziumDiscoverymapSchemeType();
// X };

#endif // KALZIUMSCHEMETYPE_H
