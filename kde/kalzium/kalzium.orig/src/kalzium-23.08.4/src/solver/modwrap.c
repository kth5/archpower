/*
    SPDX-FileCopyrightText: 2004 Thomas Nagy <tnagy2^8@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <stdio.h>
#include <string.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/mlvalues.h>

char *solve_equation(const char *eq)
{
    static value *solve_equation_closure = NULL;
    if (solve_equation_closure == NULL) {
        solve_equation_closure = caml_named_value("solve_equation");
    }

    value v = caml_copy_string(eq);
    return strdup(String_val(caml_callback(*solve_equation_closure, v)));
}
