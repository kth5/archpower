/*
    SPDX-FileCopyrightText: 2005, 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMGRADIENTTYPE_H
#define KALZIUMGRADIENTTYPE_H

class KalziumGradientType;

#include <QByteArray>
#include <QColor>

/**
 * Factory for KalziumGradientType classes.
 *
 * @author Pino Toscano
 */
class KalziumGradientTypeFactory
{
public:
    enum KalziumGradientTypes {
        SOMGradientType = 0,
        CovalentRadiusGradientType,
        VanDerWaalsRadiusGradientType,
        MassGradientType,
        BoilingPointGradientType,
        MeltingPointGradientType,
        ElectronaffinityGradientType,
        DiscoverydateGradientType,
        IonizationGradientType
    };

    /**
     * Get the instance of this factory.
     */
    static KalziumGradientTypeFactory *instance();

    /**
     * Returns the KalziumGradientType with the @p id specified.
     * It will gives 0 if none found.
     */
    KalziumGradientType *build(int id) const;
    /**
     * Returns the KalziumGradientType whose name is the @p id
     * specified.
     * It will gives 0 if none found.
     */
    KalziumGradientType *build(const QByteArray &id) const;

    /**
     * Returns a list with the names of the gradients we support.
     */
    QStringList gradients() const;

    void setCurrentGradient(int newGradient);

private:
    KalziumGradientTypeFactory();

    QList<KalziumGradientType *> m_gradients;
};

/**
 * Base class representing a gradient.
 * Inherit it and add its instance to the factory to add it globally.
 *
 * @author Pino Toscano
 */
class KalziumGradientType
{
public:
    /**
     * Get its instance.
     */
    static KalziumGradientType *instance();

    virtual ~KalziumGradientType();

    /**
     * Returns the ID of this gradient.
     * Mainly used when saving/loading.
     */
    virtual QByteArray name() const = 0;
    /**
     * Returns the description of this gradient.
     * Used in all the visible places.
     */
    virtual QString description() const = 0;

    /**
     * Calculate the coefficient of the element with atomic number
     * @p el according to this gradient. The calculated coefficient
     * will be always in the range [0, 1].
     */
    virtual double elementCoeff(int el) const;
    /**
     * Return the value, related to the current gradient, of the
     * element with atomic number @p el.
     * It will return -1 if the data is not available.
     */
    virtual double value(int el) const = 0;
    /**
     * Gives back the unit of the current value.
     */
    virtual QString unit() const = 0; // TODO How can i get the unit from?
    /**
     * Returns the minimum value of the data this gradient
     * represents.
     */
    virtual double minValue() const = 0;
    /**
     * Returns the maximum value of the data this gradient
     * represents.
     */
    virtual double maxValue() const = 0;
    /**
     * Returns the numbers of decimal in which the gradient values
     * look the best
     */
    virtual int decimals() const = 0;
    /**
     * Returns whether to use a logarithmic gradient
     * instead of a linear one.
     */
    virtual bool logarithmicGradient() const = 0;
    /**
     * Returns the first color of the gradient.
     */
    virtual QColor firstColor() const;
    /**
     * Returns the second color of the gradient.
     */
    virtual QColor secondColor() const;
    /**
     * Returns the color used to represent an element whose data is
     * not available.
     */
    virtual QColor notAvailableColor() const;

    /**
     * Calculates the color of an element which has a @p coeff which
     * is a percentage of the maximum value.
     * @param coeff is the coefficient in the range [0, 1], usually
     * calculated with elementCoeff()
     */
    QColor calculateColor(const double coeff) const;

protected:
    KalziumGradientType();
};

/**
 * The gradient by covalent radius.
 *
 * @author Pino Toscano
 */
class KalziumCovalentRadiusGradientType : public KalziumGradientType
{
public:
    static KalziumCovalentRadiusGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumCovalentRadiusGradientType();
};

/**
 * The gradient by van Der Waals radius.
 *
 * @author Pino Toscano
 */
class KalziumVanDerWaalsRadiusGradientType : public KalziumGradientType
{
public:
    static KalziumVanDerWaalsRadiusGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumVanDerWaalsRadiusGradientType();
};

/**
 * The gradient by atomic mass.
 *
 * @author Pino Toscano
 */
class KalziumMassGradientType : public KalziumGradientType
{
public:
    static KalziumMassGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumMassGradientType();
};

/**
 * The gradient by boiling point.
 *
 * @author Pino Toscano
 */
class KalziumBoilingPointGradientType : public KalziumGradientType
{
public:
    static KalziumBoilingPointGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumBoilingPointGradientType();
};

/**
 * The gradient by melting point.
 *
 * @author Pino Toscano
 */
class KalziumMeltingPointGradientType : public KalziumGradientType
{
public:
    static KalziumMeltingPointGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumMeltingPointGradientType();
};

/**
 * The gradient for SOM Widget
 *
 * @author Etienne Rebetez
 */
class KalziumSOMGradientType : public KalziumGradientType
{
public:
    static KalziumSOMGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumSOMGradientType();
};

/**
 * The gradient by electronegativity.
 *
 * @author Pino Toscano
 */
class KalziumElectronegativityGradientType : public KalziumGradientType
{
public:
    static KalziumElectronegativityGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumElectronegativityGradientType();
};

/**
 * The gradient by discoverydate.
 *
 * @author Carsten Niehaus
 */
class KalziumDiscoverydateGradientType : public KalziumGradientType
{
public:
    static KalziumDiscoverydateGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumDiscoverydateGradientType();
};

/**
 * The gradient by electronaffinity.
 *
 * @author Carsten Niehaus
 */
class KalziumElectronaffinityGradientType : public KalziumGradientType
{
public:
    static KalziumElectronaffinityGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumElectronaffinityGradientType();
};

/**
 * The gradient by the first ionization energy.
 *
 * @author Carsten Niehaus
 */
class KalziumIonizationGradientType : public KalziumGradientType
{
public:
    static KalziumIonizationGradientType *instance();

    QByteArray name() const override;
    QString description() const override;

    double value(int el) const override;
    QString unit() const override;

    double minValue() const override;
    double maxValue() const override;
    int decimals() const override;

    bool logarithmicGradient() const override;

private:
    KalziumIonizationGradientType();
};

#endif // KALZIUMGRADIENTTYPE_H
