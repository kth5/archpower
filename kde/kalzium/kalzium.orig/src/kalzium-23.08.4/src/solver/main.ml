(*
    SPDX-FileCopyrightText: 2004 Thomas Nagy <tnagy2^8@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*)

open Printf;;
open Calc;;


let myeq1 = " a CH3(CH2)3COOH + b O2 -> c H2O + d CO2";;
let myeq2 = " a CH3COOH + b O2 -> c H2O + d CO2";;
let myeq3 = " a CH3(CH2)3COOH + b O2 -> invalid equation ";;
let myeq4 = "2CH4+aC2H6+cO2->eH2O+5CO2";;
let myeq5 = "a NaNO2 + b NH3 -> b NaN3 + e NaNO3";;
let myeq6 = "a HNO2 + b NH3 -> b HN3 + e HNO3";;
let myeq7 = "a H3O[+] + dCO2 + eO2 -> fH2O + dCO3[2+]";;

(* Printf.printf "%s \n" (solve_equation myeq1);;
Printf.printf "%s \n" (solve_equation myeq2);;
Printf.printf "%s \n" (solve_equation myeq3);; *)
(*
Printf.printf "%s \n" (solve_equation myeq5);;
Printf.printf "%s \n" (solve_equation myeq6);;
Printf.printf "%s \n" (solve_equation myeq6);;

Printf.printf "%s \n" (solve_equation myeq6);;*)
Printf.printf "%s \n" (solve_equation myeq7);;
