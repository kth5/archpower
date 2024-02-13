(*
    SPDX-FileCopyrightText: 2004 Thomas Nagy <tnagy2^8@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*)

(* File lexer.mll*)
{
        open Parser;;
        exception IllegalChar
}

rule token = parse
[' ' '\t' '\n']   {token lexbuf} (* ignore whitespaces *)
| ['0'-'9']+      { INT(int_of_string(Lexing.lexeme lexbuf)) }
| ['A'-'Z']       { CAPITAL(Lexing.lexeme lexbuf) }
| ['a'-'z']+      { MINOR(Lexing.lexeme lexbuf) }
| '+'             { PLUS }
| '-'             { MINUS }
| '('             { LPAREN }
| ')'             { RPAREN }
| '['             { LBRACKET }
| ']'             { RBRACKET }
| "->"            { ARROW }
| _               { raise IllegalChar; }
| eof             { EOF }
