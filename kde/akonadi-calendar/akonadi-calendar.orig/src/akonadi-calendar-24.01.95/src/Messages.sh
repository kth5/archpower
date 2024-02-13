#! /bin/sh
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC `find . -name \*.ui -o -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.h"` -o $podir/libakonadi-calendar6.pot
rm -f rc.cpp
