[@deriving sexp]
type operator = Operators.Typ.t;

[@deriving sexp]
type t = opseq
and opseq = OpSeq.t(operand, operator)
and operand =
  | Hole
  | Unit
  | Int
  | Float
  | Bool
  | Parenthesized(t)
  | List(t);

[@deriving sexp]
type skel = OpSeq.skel(operator);
[@deriving sexp]
type seq = OpSeq.seq(operand, operator);

let get_prod_elements: skel => list(skel);

let is_EmptyHole: operand => bool;

let unwrap_parentheses: operand => t;

let associate: seq => Skel.t(Operators.Typ.t);

let mk_OpSeq: OpSeq.seq(operand, operator) => OpSeq.t(operand, operator);

let contract: HTyp.t => t;

let expand: t => HTyp.t;

let expand_opseq: t => HTyp.t;

let expand_skel:
  (OpSeq.skel(operator), OpSeq.seq(operand, operator)) => HTyp.t;

let expand_operand: operand => HTyp.t;

let is_complete_operand: operand => bool;

let is_complete_skel: (skel, seq) => bool;

let is_complete: t => bool;