type chemtbl = (string, int) Hashtbl.t
and chemrecord = { mutable hashtbl : chemtbl; mutable formula : string; }
and item = { ikey : string; itbl : chemrecord; mutable sign : int; }
and listitems = item list
val chem_addsym : chemtbl -> string -> int -> unit
val chem_add : chemrecord -> chemrecord -> chemrecord
val chem_mult : chemrecord -> int -> chemrecord
val createchem : string -> int -> chemrecord
val chem_negate : listitems -> unit
val chem_printitem : item -> unit
