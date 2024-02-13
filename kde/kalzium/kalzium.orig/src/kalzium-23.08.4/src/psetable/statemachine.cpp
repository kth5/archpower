/*
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "statemachine.h"

StateSwitcher::StateSwitcher(QStateMachine *machine)
    : QState(machine)
{
}

void StateSwitcher::addState(QState *state, QAbstractAnimation *animation, int id)
{
    auto trans = new StateSwitchTransition(id);
    trans->setTargetState(state);
    addTransition(trans);
    trans->addAnimation(animation);
}

void StateSwitcher::switchToState(int n)
{
    machine()->postEvent(new StateSwitchEvent(n));
}
