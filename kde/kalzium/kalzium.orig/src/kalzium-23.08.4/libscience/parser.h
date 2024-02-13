/*
    SPDX-FileCopyrightText: 2005 Inge Wallin <inge@lysator.liu.se>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSER_H
#define PARSER_H

#include "science_export.h"

#include <QString>

/**
 * @class Parser
 * This is a general purpose parser originally written by Inge Wallin.
 *
 * It is intended to be subclassed; see MoleculeParser.
 *
 * @author Inge Wallin
 */
class SCIENCE_EXPORT Parser
{
public:
    /**
     * Constructor
     */
    Parser();

    /**
     * Constructor
     *
     * @param _str @ref start the parsing with @p _str
     */
    explicit Parser(const QString &_str);

    /**
     * Destructor
     */
    virtual ~Parser();

    /**
     * Start a new parse.
     */
    void start(const QString &_str);

    /**
     * Peek at the next character;
     */
    int nextChar() const
    {
        return m_nextChar;
    }

    /**
     * Peek at the next token.
     */
    int nextToken() const
    {
        return m_nextToken;
    }

    /**
     * Get the value stored for different types of tokens.
     */
    int intVal() const
    {
        return m_intVal;
    }
    float floatVal() const
    {
        return m_floatVal;
    }

private:
    // Try to parse some special datatypes.
    bool parseInt(int *_result);
    bool parseSimpleFloat(double *_result);

protected:
    /**
     * All characters are their own token value per default.
     * Extend this list in your subclass to make a more advanced parser.
     */
    static const int INT_TOKEN = 257;

    /**
     * All characters are their own token value per default.
     * Extend this list in your subclass to make a more advanced parser.
     */
    static const int FLOAT_TOKEN = 258;

    /**
     * Make the next character the current one.
     */
    int getNextChar();

    /**
     * Make the next non-space character the current one.
     */
    int skipWhitespace();

    /**
     * Fetches the next token.
     */
    virtual int getNextToken();

private:
    QString m_str;
    int m_index;
    int m_nextChar;

protected:
    // Lexical analysis and token handling.  These members need to be
    // protected instead of private since we want to be able to
    // reimplement getNextToken().

    /**
     * The next token to be used in the parser.
     */
    int m_nextToken;

    // Values for the respective token.  These could be made into a
    // union, but I don't think it is necessary to bother, since they
    // are so few and we don't instantiate a lot of copies of the
    // parser.

    /**
     * Valid if m_nextToken == INT_TOKEN
     */
    int m_intVal; // Valid if m_nextToken == INT_TOKEN

    /**
     * Valid if m_nextToken == FLOAT_TOKEN
     */
    double m_floatVal; // Valid if m_nextToken == FLOAT_TOKEN
};

#endif // PARSER_H
