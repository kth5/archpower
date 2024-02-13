/*
    SPDX-FileCopyrightText: 2005 Inge Wallin <inge@lysator.liu.se>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MOLECULEPARSER_H
#define MOLECULEPARSER_H

#include "science_export.h"

#include "element.h"
#include "parser.h"

#include <QList>
#include <QMap>

/**
 * @class ElementCount
 * @author Inge Wallin
 */
class SCIENCE_EXPORT ElementCount
{
public:
    /**
     * Constructor
     */
    ElementCount(Element *_element, int _count)
    {
        m_element = _element;
        m_count = _count;
    }

    /**
     * Constructor
     */
    explicit ElementCount(Element *_element)
    {
        m_element = _element;
        m_count = 0;
    }
    /**
     * Destructor
     */
    ~ElementCount();

    /**
     * @return the Element
     */
    Element *element() const
    {
        return m_element;
    }

    /**
     * @return the number of occurrences of the Element
     */
    int count() const
    {
        return m_count;
    }

    /**
     * Add @p _count occurrences of the Element
     * @param _count The number of times the Element occurs
     */
    void add(int _count)
    {
        m_count += _count;
    }
    void multiply(int _factor)
    {
        m_count *= _factor;
    }

    /**
     * The Element of the object
     */
    Element *m_element;
    /**
     * The number of occurrences
     */
    int m_count;
};

/**
 * This class is used to count the elements in the molecule
 * which is being calculated
 *
 * @class ElementCountMap
 * @author Inge Wallin
 */
class SCIENCE_EXPORT ElementCountMap
{
public:
    /**
     * Constructor
     */
    ElementCountMap();

    /**
     * Destructor
     */
    ~ElementCountMap();

    /**
     * Clear the map of ElementCount pointers
     */
    void clear()
    {
        m_map.clear();
    }

    /**
     * @param _element the searched Element
     * @return the Element which is searched
     */
    ElementCount *search(Element *_element);

    /**
     * @param _map
     */
    void add(ElementCountMap &_map);

    /**
     * Returns the elements in the molecule. For example, if the molecule
     * is CO2, a list with C and O will be returned.
     * @return the elements in the molecule
     */
    QList<Element *> elements() const;

    /**
     * @param _element
     * @param _count
     */
    void add(Element *_element, int _count);

    /**
     * @param _factor
     */
    void multiply(int _factor);

    QList<ElementCount *> map()
    {
        return m_map;
    }

private:
    QList<ElementCount *> m_map;
};

/**
 * @class MoleculeParser
 *
 * Parse molecule formulas.
 *
 * Usage:
 * @code
 *   MoleculeParser  parser;
 *   QString         chemical_formula = "C2H5OH";
 *   double          weight;
 *
 *   if (parser.weight(chemical_formula, &weight))
 *     cout << "Weight of " << chemical_formula << " = " << weight << ".\n";
 *   else
 *     cout << "Parse error\n";
 * @endcode
 *
 * If a short form of a compound is specified, it will be expanded.
 * Example :- EtOH -> (C2H5OH)
 * @code
 *   MoleculeParser  parser;
 *   QString         chemical_formula = "EtOH";
 *   double          weight;
 *
 *   if (parser.weight(chemical_formula, &weight))
 *     cout << "Weight of " << chemical_formula << " = " << weight << ".\n";
 *   else
 *     cout << "Parse error\n";
 * @endcode
 *
 * @author Inge Wallin
 * @author Kashyap R Puranik
 */
class SCIENCE_EXPORT MoleculeParser : public Parser
{
public:
    /**
     * @param list This list of chemical elements will be used internally
     * for searching and matching with searched strings
     * Constructor
     */
    explicit MoleculeParser(const QList<Element *> &list);

    /**
     * Constructor
     *
     * @param _str @ref Parser::start the parsing with @p _str
     */
    explicit MoleculeParser(const QString &_str);

    /**
     * Destructor
     */
    ~MoleculeParser() override;

    /**
     * Try to parse the molecule @p molecule and get the weight of it.
     * The calculated weight is stored in @p _result.
     *
     * @param _moleculeString
     * @param _resultMass
     * @param _resultMap
     *
     * @return whether the parsing was successful or not
     */
    bool weight(const QString &_moleculeString, double *_resultMass, ElementCountMap *_resultMap);

    QSet<QString> aliasList();

private:
    // Helper functions
    bool parseSubmolecule(double *_resultMass, ElementCountMap *_resultMap);
    bool parseTerm(double *_resultMass, ElementCountMap *_resultMap);
    // This function expands the molecule string
    // eg expandFormula(EtOH)    returns (C2H5)OH
    QString expandFormula(const QString &_shortMolecularMass);
    // This function expands a term
    // eg expandTerm(Et) returns (C2H5)
    QString expandTerm(const QString &_group);

    QList<Element *> m_elementList;

    static const int ELEMENT_TOKEN = 300;

    Element *lookupElement(const QString &_name);

    QMap<Element *, int> m_elementMap;

    // Contains the list of aliases eg, { "Et - C2H5", "Me - CH3"}
    QSet<QString> *m_aliasList;
    // if this booloean is "true" the parser found an error
    bool m_error;

protected:
    /**
     * Extends the standard tokenizer in Parser::getNextToken().
     */
    int getNextToken() override;

private:
    Element *m_elementVal; // Valid if m_nextToken == ELEMENT_TOKEN
};

#endif // MOLECULEPARSER_H
