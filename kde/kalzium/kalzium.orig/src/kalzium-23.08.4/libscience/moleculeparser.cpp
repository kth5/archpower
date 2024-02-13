/*
    SPDX-FileCopyrightText: 2005 Inge Wallin <inge@lysator.liu.se>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "moleculeparser.h"

#include <cctype>

#include "kalzium_libscience_debug.h"
#include <QFile>
#include <QStandardPaths>

// ================================================================
//                    class ElementCountMap

ElementCountMap::ElementCountMap()
{
    m_map.clear();
}

ElementCountMap::~ElementCountMap() = default;

ElementCount *ElementCountMap::search(Element *_element)
{
    for (ElementCount *c : std::as_const(m_map)) {
        if (c->element() == _element) {
            return c;
        }
    }

    return nullptr;
}

void ElementCountMap::add(ElementCountMap &_map)
{
    for (ElementCount *c : std::as_const(_map.m_map)) {
        add(c->m_element, c->m_count);
    }
}

QList<Element *> ElementCountMap::elements() const
{
    QList<Element *> list;

    for (ElementCount *c : std::as_const(m_map)) {
        Element *e = c->m_element;
        if (!list.contains(e)) {
            list << e;
        }
    }

    return list;
}

void ElementCountMap::add(Element *_element, int _count)
{
    ElementCount *elemCount;

    elemCount = search(_element);
    if (elemCount) {
        elemCount->m_count += _count;
    } else {
        m_map.append(new ElementCount(_element, _count));
    }
}

void ElementCountMap::multiply(int _factor)
{
    for (ElementCount *count : std::as_const(m_map)) {
        count->multiply(_factor);
    }
}

// ================================================================
//                    class MoleculeParser

MoleculeParser::MoleculeParser(const QList<Element *> &list)
    : Parser()
{
    m_elementList = list;
    m_aliasList = new QSet<QString>;
}

MoleculeParser::MoleculeParser(const QString &_str)
    : Parser(_str)
{
    m_aliasList = new QSet<QString>;
}

MoleculeParser::~MoleculeParser()
{
    delete m_aliasList;
}

// ----------------------------------------------------------------
//                            public methods

// Try to parse the molecule and get the weight of it.
//
// This method also acts as the main loop.

bool MoleculeParser::weight(const QString &_shortMoleculeString, double *_resultMass, ElementCountMap *_resultMap)
{
    if (_shortMoleculeString.isEmpty()) {
        return false;
    }
    // Clear the list of aliases and start filling it again.

    m_aliasList->clear();
    QString _moleculeString;
    // Clear the result variables and set m_error to false
    _resultMap->clear();
    m_error = false;
    *_resultMass = 0.0;

    // Expand the molecule string
    // Example : MeOH -> (CH3)OH
    qCDebug(KALZIUM_LIBSCIENCE_LOG) << _shortMoleculeString << "is going to be expanded";
    _moleculeString = expandFormula(_shortMoleculeString);
    qCDebug(KALZIUM_LIBSCIENCE_LOG) << _moleculeString << "is the expanded string";

    // Now set the expanded string
    // Initialize the parsing process, and parse te molecule.
    start(_moleculeString);
    parseSubmolecule(_resultMass, _resultMap);

    if (nextToken() != -1) {
        return false;
    }

    if (m_error) { // there was an error in the input...
        return false;
    }

    return true;
}

QSet<QString> MoleculeParser::aliasList()
{
    return *m_aliasList;
}
// ----------------------------------------------------------------
//            helper methods for the public methods

// Parse a submolecule.  This is a list of terms.
//

bool MoleculeParser::parseSubmolecule(double *_resultMass, ElementCountMap *_resultMap)
{
    double subMass = 0.0;
    ElementCountMap subMap;

    *_resultMass = 0.0;
    _resultMap->clear();
    while (parseTerm(&subMass, &subMap)) {
        // qCDebug(KALZIUM_LIBSCIENCE_LOG) << "Parsed a term, weight = " << subresult;

        // Add the mass and composition of the submolecule to the total.
        *_resultMass += subMass;
        _resultMap->add(subMap);
    }

    return true;
}

// Parse a term within the molecule, i.e. a single atom or a
// submolecule within parenthesis followed by an optional number.
// Examples: Bk, Mn2, (COOH)2
//
// Return true if correct, otherwise return false.

// If correct, the mass of the term is returned in *_resultMass, and
// the flattened composition of the molecule in *_resultMap.
//

bool MoleculeParser::parseTerm(double *_resultMass, ElementCountMap *_resultMap)
{
    *_resultMass = 0.0;
    _resultMap->clear();

    if (nextToken() == ELEMENT_TOKEN) {
        // qCDebug(KALZIUM_LIBSCIENCE_LOG) << "Parsed an element: " << m_elementVal->symbol();
        *_resultMass = m_elementVal->dataAsVariant(ChemicalDataObject::mass).toDouble();
        _resultMap->add(m_elementVal, 1);

        getNextToken();
    } else if (nextToken() == '(') {
        // A submolecule.

        getNextToken();
        parseSubmolecule(_resultMass, _resultMap);

        // Must end in a ")".
        if (nextToken() == ')') {
            // qCDebug(KALZIUM_LIBSCIENCE_LOG) << "Parsed a submolecule. weight = " << *_result;
            getNextToken();
        } else {
            return false;
        }
    } else {
        // Neither an element nor a list within ().
        return false;
    }

    // Optional number.
    if (nextToken() == INT_TOKEN) {
        // qCDebug(KALZIUM_LIBSCIENCE_LOG) << "Parsed a number: " << intVal();

        *_resultMass *= intVal();
        _resultMap->multiply(intVal());

        getNextToken();
    }

    qCDebug(KALZIUM_LIBSCIENCE_LOG) << "Weight of term = " << *_resultMass;
    return true;
}

// ----------------------------------------------------------------
//                           protected methods

// Extend Parser::getNextToken with elements.

int MoleculeParser::getNextToken()
{
    QString name;

#if 0
    qCDebug(KALZIUM_LIBSCIENCE_LOG) << "getNextToken(): Next character = "
          << nextChar() << endl;
#endif

    // Check if the token is an element name.
    if ('A' <= nextChar() && nextChar() <= 'Z') {
        name = char(nextChar());
        getNextChar();

        if ('a' <= nextChar() && nextChar() <= 'z') {
            name.append(char(nextChar()));
            getNextChar();
        }

        // Look up the element from the name..
        m_elementVal = lookupElement(name);
        if (m_elementVal) {
            m_nextToken = ELEMENT_TOKEN;
        } else {
            m_nextToken = -1;
        }
    } else {
        return Parser::getNextToken();
    }

    return m_nextToken;
}

// ----------------------------------------------------------------
//                          private methods

Element *MoleculeParser::lookupElement(const QString &_name)
{
    qCDebug(KALZIUM_LIBSCIENCE_LOG) << "looking up " << _name;

    for (Element *e : std::as_const(m_elementList)) {
        if (e->dataAsVariant(ChemicalDataObject::symbol) == _name) {
            qCDebug(KALZIUM_LIBSCIENCE_LOG) << "Found element " << _name;
            return e;
        }
    }

    // if there is an error make m_error true.
    m_error = true;

    qCDebug(KALZIUM_LIBSCIENCE_LOG) << "no such element!: " << _name;

    return nullptr;
}

QString MoleculeParser::expandFormula(const QString &_shortString)
{
    QString _fullString; // the expanded string that will be returned
    QString::const_iterator i; // iterator
    QString temp; // a temporary string that will contain a single element/group
    QString expandedTerm; // expansion of a particular term

    // Go through all letters in the string.
    for (i = _shortString.constBegin(); i != _shortString.constEnd();) {
        temp = QLatin1String("");

        // If a capital letter was found
        if ((*i).category() == QChar::Letter_Uppercase) {
            temp += (*i);
            ++i;

            // A small letter following a capital letter
            if (i != _shortString.end() && (*i).category() == QChar::Letter_Lowercase) {
                temp += (*i);
                ++i;
            }

            // If element is found, append it
            if (lookupElement(temp)) {
                _fullString += temp;
            } else if (!((expandedTerm = expandTerm(temp)).isEmpty())) {
                // If an expansion was made, return the expansion
                qCDebug(KALZIUM_LIBSCIENCE_LOG) << "expanded" << temp << "to" << expandedTerm;
                _fullString += '(' + expandedTerm + ')';
            } else { // invalid term, append it. (Validation is done later anyway.)
                _fullString += temp;
            }
        } else if (*i == '(') { // Return parenthesis as and when found
            _fullString += '(';
            ++i;
        } else if (*i == ')') {
            _fullString += ')';
            ++i;
        } else if (*i == '#') { // If # is found, we have a short-form eg #EDTA#
            ++i; // go to the next character
            // Get the term between # and #
            while (*i != '#' && i != _shortString.constEnd()) {
                temp += *i;
                ++i;
            }
            // If the string ended, just add the part that comes after #
            if (i == _shortString.constEnd()) {
                _fullString += temp;
                break;
            } else if (!temp.isEmpty()) { // else expand the term between # and #
                // if alias is not found, just add without expanding the term
                if ((expandedTerm = expandTerm(temp)).isEmpty()) {
                    _fullString += temp;
                } else { // else add the expanded term
                    _fullString += expandedTerm;
                }
            }
            ++i;
        } else if ((*i).category() == QChar::Number_DecimalDigit) { // If number was found, return it
            _fullString += *i;
            ++i;
        } else { // invalid character, return it, validation is done again later
            _fullString += *i;
            ++i;
            qCDebug(KALZIUM_LIBSCIENCE_LOG) << *i << "invalid character!";
        }
    }

    // Reset all "element not found" errors.
    m_error = false;
    return _fullString;
}

QString MoleculeParser::expandTerm(const QString &_group)
{
    QString shortForm, fullForm; // short form (symbol) and full form (expansion)
    QString temp; // A temporary QString used in Regular expressions

    // Search in User defined aliases.
    QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/symbols2.csv"));
    QFile file(fileName);

    // Check file validity
    if (!(!file.open(QIODevice::ReadOnly | QIODevice::Text))) {
        qCDebug(KALZIUM_LIBSCIENCE_LOG) << fileName << " opened";
        QTextStream in(&file);

        // Get all shortForms and fullForms in the file.
        while (!in.atEnd()) {
            QString line = in.readLine();
            shortForm = line.section(',', 0, 0);
            shortForm.remove(QChar('\"'));
            fullForm = line.section(',', 1, 1);
            fullForm.remove(QChar('\"'));

            // If short term is found, return fullForm
            if (shortForm == _group) {
                *m_aliasList << (_group + " : " + fullForm);
                return (fullForm);
            }
        }
    } else {
        qCDebug(KALZIUM_LIBSCIENCE_LOG) << fileName << " could not be opened!";
    }

    // Find the system defined aliases
    // Open the file
    fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/symbols.csv"));
    QFile file2(fileName);

    // Check file validity
    if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCDebug(KALZIUM_LIBSCIENCE_LOG) << fileName << " opened";
        QTextStream in(&file2);

        // Get all shortForms and fullForms in the file.
        while (!in.atEnd()) {
            QString line = in.readLine();
            shortForm = line.section(',', 0, 0);
            shortForm.remove(QChar('\"'));
            fullForm = line.section(',', 1, 1);
            fullForm.remove(QChar('\"'));

            if (shortForm == _group) {
                *m_aliasList << (_group + " : " + fullForm);
                return (fullForm);
            }
        }
    } else {
        qCDebug(KALZIUM_LIBSCIENCE_LOG) << fileName << " could not be opened!";
    }

    // Sample expansions, work even when file is not found, testing purposes
    if (_group == QLatin1String("Me")) {
        return ("CH3");
    } else if (_group == QLatin1String("Et")) {
        return ("C2H5");
    } else { // If not found return an empty string.
        return ("");
    }
}
