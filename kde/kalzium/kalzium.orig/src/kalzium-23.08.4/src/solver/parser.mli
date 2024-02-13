type token =
  | INT of (int)
  | PLUS
  | MINUS
  | LPAREN
  | RPAREN
  | LBRACKET
  | RBRACKET
  | EOF
  | CAPITAL of (string)
  | MINOR of (string)
  | ARROW

val main :
  (Lexing.lexbuf  -> token) -> Lexing.lexbuf -> Chemset.listitems
