/*

    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the QtCore module of the Qt Toolkit.

    SPDX-License-Identifier: BSD-3-Clause

*/

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QAbstractAnimation>
#include <QAbstractTransition>
#include <QEvent>
#include <QState>
#include <QStateMachine>

class StateSwitchEvent : public QEvent
{
public:
    StateSwitchEvent()
        : QEvent(Type(StateSwitchType))
    {
    }

    StateSwitchEvent(int id)
        : QEvent(Type(StateSwitchType))
        , m_id(id)
    {
    }

    enum { StateSwitchType = QEvent::User + 256 };

    int id() const
    {
        return m_id;
    }

private:
    int m_id;
};

class StateSwitchTransition : public QAbstractTransition
{
public:
    StateSwitchTransition(int id)
        : QAbstractTransition()
        , m_id(id)
    {
    }

protected:
    bool eventTest(QEvent *event) override
    {
        return (event->type() == QEvent::Type(StateSwitchEvent::StateSwitchType)) && (static_cast<StateSwitchEvent *>(event)->id() == m_id);
    }

    void onTransition(QEvent *) override
    {
    }

private:
    int m_id;
};

class StateSwitcher : public QState
{
public:
    explicit StateSwitcher(QStateMachine *machine);

    void addState(QState *state, QAbstractAnimation *animation, int id);

    void switchToState(int n);
};

#endif // STATEMACHINE_H
