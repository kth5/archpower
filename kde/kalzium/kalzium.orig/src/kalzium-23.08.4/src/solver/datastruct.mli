class eqtable :
  object
    val mutable m_middle : int
    val mutable m_solved : bool
    val mutable numtbl : int array array
    val mutable soltbl : int array
    val mutable strtbl : string array
    val mutable vartbl : string array
    method build : Chemset.listitems -> unit
    method clear : unit -> unit
    method get_eq_orig : unit -> string
    method get_eq_sol : unit -> string
    method getformula : int -> string
    method getline : int -> int array
    method getsize_i : unit -> int
    method getsize_j : unit -> int
    method getsol : int -> int
    method getvar : int -> string
    method private init : int -> int -> unit
    method isSolved : unit -> bool
    method print_all : unit -> unit
    method setsol : int -> int -> unit
  end
