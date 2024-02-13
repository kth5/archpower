(*
    SPDX-FileCopyrightText: 2004 Thomas Nagy <tnagy2^8@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*)

open Printf;;
open Hashtbl;;
open List;;
open String;;

type chemtbl = (string, int) Hashtbl.t;;
type chemrecord = {mutable hashtbl:chemtbl; mutable formula:string};;
type item = {ikey:string; itbl:chemrecord; mutable sign:int};;
type listitems = (item) list;;


(* add a symbol to a molecule *)
let chem_addsym (tbl:chemtbl) (sym:string) (qte:int) = 
    let prev_qte = ref 0 in
    if Hashtbl.mem tbl sym then prev_qte := Hashtbl.find tbl sym;
    Hashtbl.replace tbl sym (!prev_qte+qte)
;;

(* add merge two sub_molecules *)
let chem_add (tbl1:chemrecord) (tbl2:chemrecord) = 
    Hashtbl.iter (fun sym qte -> chem_addsym tbl1.hashtbl sym qte) tbl2.hashtbl;
    tbl1.formula <- tbl1.formula^tbl2.formula;
    tbl1
;;

(* multiply a sub-molecule (amount of atoms) by an integer value *)
let chem_mult (tbl:chemrecord) (qte:int) = 
    Hashtbl.iter (fun sym old_qte-> Hashtbl.replace tbl.hashtbl sym (old_qte*qte) ) tbl.hashtbl;
    tbl.formula <- "("^tbl.formula^")"^string_of_int(qte);
    tbl
;;

(* creates a small molecule *)
let createchem (sym:string) (qte:int) =

    let prettyformula () =
        if String.contains sym '+' || String.contains sym '-' then begin
            if qte == 1 then "<b><sup>"^sym^"</sup></b>"
            else "<b><sup>"^string_of_int(qte)^sym^"</sup></b>" end 
        else begin
            if qte == 1 then sym
            else sym^"<b><sub>"^string_of_int(qte)^"</sub></b>"
        end
    in
    
    let table = Hashtbl.create 10 in
    Hashtbl.add table sym qte;
    { hashtbl=table ; formula=prettyformula() }
    (*if (qte!=1) then { hashtbl=table ; formula=prettyformula() }
    else { hashtbl=table ; formula=sym }*)
;;

let chem_negate (l:listitems) =
    List.iter (fun i -> i.sign <- -1) l
;;

(* outputs a molecule *)
let chem_printitem (i:item) =
    Printf.printf "item : %s %s %d \n" i.ikey (i.itbl).formula i.sign;
    Hashtbl.iter (fun sym qte -> Printf.printf " * %s %d\n" sym qte) i.itbl.hashtbl
;;
