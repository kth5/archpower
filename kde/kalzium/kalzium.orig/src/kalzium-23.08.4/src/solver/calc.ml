(*
    SPDX-FileCopyrightText: 2004 Thomas Nagy <tnagy2^8@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*)

open Printf;;
open Chemset;;
open Datastruct;;
open Chem;;
open Hashtbl;;

let create_equation str = 
    let lexbuf = Lexing.from_string str in
    let result = Parser.main Lexer.token lexbuf in
    result
;;

exception Not_found;;

let solve_equation (str:string) =
    let eq = new eqtable in
    try
        eq#build (create_equation str);
        try
(*          eq#print_all (); *)
            solve eq;
            eq#get_eq_sol ();

        with | _ -> begin 
            let str = (eq#get_eq_orig ())^" : No solution found" in
            (*cleanup eq;*)
            str
        end
    with | _ -> str^" : Parse Error";
;;

let _ = Callback.register "solve_equation" solve_equation;;
