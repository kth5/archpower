(*
    SPDX-FileCopyrightText: 2004 Thomas Nagy <tnagy2^8@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*)

open List;;
open Chemset;;
open Hashtbl;;
open Array;;

class eqtable =
object (self)

(* columns : vars + formula | lines : chemical elements *)
val mutable numtbl = Array.make_matrix 0 0 0
val mutable strtbl = Array.make 0 ""
val mutable vartbl = Array.make 0 ""
val mutable soltbl = Array.make 0 0
val mutable m_solved = false
val mutable m_middle = 0
    
(* val mutable (table:int array array) = [||] *)

(* lines : i : chem element *)
(* columns : vars j *)
method getsize_i () = Array.length numtbl
method getsize_j () = if (self#getsize_i () > 0) then Array.length numtbl.(0) else 0

method getline j = numtbl.(j)

method getformula k = strtbl.(k)
method getvar k = vartbl.(k)
method getsol k = soltbl.(k)

method setsol k v = soltbl.(k) <- v
method isSolved () = m_solved

method get_eq_sol () =
    let str = ref "" in
    for j=0 to (self#getsize_j () -1) do
        if (j == m_middle) then str := (!str)^" -&gt; "
        else if (j>0 && j<self#getsize_j ()) then str := (!str)^" + ";
            
        str := (!str)^"<b>"^string_of_int(self#getsol j)^"</b> "^(self#getformula j);
    done;
    !str

method get_eq_orig () =
    let str = ref "" in
    for j=0 to (self#getsize_j () -1) do
        if (j == m_middle) then str := (!str)^" -&gt; "
        else if (j>0 && j<self#getsize_j ()) then str := (!str)^" + ";
            
        str := (!str)^"<b>"^(self#getvar j)^"</b> "^(self#getformula j);
    done;
    !str
    
method private init i j = numtbl <- Array.make_matrix i j 0;
    strtbl <- Array.make j "";
    vartbl <- Array.make j "";
    soltbl <- Array.make j 0
    
method clear () =
    self#init 0 0;
    
method print_all () =
    Printf.printf "--- start print_all ---\n";
    for i = 0 to (self#getsize_i ())-1 do
        for j = 0 to (self#getsize_j ())-1 do
            Printf.printf "%d " (numtbl.(i).(j));
        done;
        Printf.printf "\n";
    done;
    Printf.printf "--- end print_all ---\n";
    flush_all ()
                      
(* build the matrix to solve *)
method build (lst:listitems) = 
    let nb_symbols = ref 0 in
    let item_array = Array.of_list lst in
    let record:(string, int) Hashtbl.t = Hashtbl.create 10 in
    let nb_items = ref (Array.length item_array) in
    for i=0 to !nb_items-1 do
        Hashtbl.iter (fun sym _ ->
            (* take all chemical elements but simplify ions (+ or -) into + *)
            let symprocessed = if String.contains sym '+' || String.contains sym '-'
                then "+" else sym in
            
            if not (Hashtbl.mem record symprocessed) then begin
                Hashtbl.add record symprocessed !nb_symbols;
                nb_symbols := !nb_symbols+1
            end
            ) item_array.(i).itbl.hashtbl
    done;
    
    (* initialize the matrix *)
    self#init (!nb_symbols) (!nb_items);
    
    (* process each atom*)
    for i=0 to !nb_items-1 do
        (* find the middle (->) - nothing to do with the others things in this loop *)
        if (item_array.(i).sign<0 && i>0) then (if (item_array.(i-1).sign>0) then m_middle<-i);
        
        (* store the molecule formula *)
        vartbl.(i) <- item_array.(i).ikey;
        strtbl.(i) <- item_array.(i).itbl.formula;

        (* for each molecule, process the atoms *)
        Hashtbl.iter (fun sym qte -> 

            if String.contains sym '+' || String.contains sym '-' then begin
                (* it is an electric charge *)
                let chargesign = if String.contains sym '-' then -1 else 1 in
                let line_idx = (Hashtbl.find record "+") in
                numtbl.(line_idx).(i) <- qte * item_array.(i).sign * chargesign
            end
            else begin
                (* check if the atom is already there *)
                let line_idx = (Hashtbl.find record sym) in
                numtbl.(line_idx).(i) <- (qte * item_array.(i).sign)
            end
            ) item_array.(i).itbl.hashtbl
    done

end;;
