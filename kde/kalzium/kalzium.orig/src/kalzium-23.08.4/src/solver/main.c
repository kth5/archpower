/*
    SPDX-FileCopyrightText: 2004 Thomas Nagy <tnagy2^8@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    caml_startup(argv);

    char *eq = " a CH3(CH2)3COOH + b O2 -> c H2O + d CO2";
    char *eq2 = " a CH3(CH2)3COOH + b O2 -> c H2O + d CO";
    char *result = solve_equation(eq);
    char *result2 = solve_equation(eq2);

    printf("solution : %s\n", result);

    printf("solution : %s\n", result2);
    free(result2);

    result2 = solve_equation(eq);
    printf("solution : %s\n", result2);

    free(result);
    free(result2);

    return 0;
}
