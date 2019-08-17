open GeneralUtil;
open SemanticsCommon;

[@deriving sexp]
type op =
  | Arrow
  | Prod
  | Sum;

[@deriving sexp]
type skel_t = Skel.t(op);

[@deriving sexp]
type t =
  /* outer nodes */
  | Hole
  | Unit
  | Num
  | Bool
  /* inner nodes */
  | Parenthesized(t)
  | List(t)
  | OpSeq(skel_t, opseq)
and opseq = OperatorSeq.opseq(t, op);

exception SkelInconsistentWithOpSeq(skel_t, opseq);

let bidelimited = (uty: t): bool =>
  switch (uty) {
  /* outer nodes */
  | Hole
  | Unit
  | Num
  | Bool => true
  /* inner nodes */
  | Parenthesized(_) => true
  | List(_) => true
  | OpSeq(_, _) => false
  };

/* TODO fix this to only parenthesize when necessary */
let rec contract = (ty: HTyp.t): t => {
  let mk_opseq = (op', a, b) => {
    let ph = n => Skel.Placeholder(n);
    let skel = Skel.BinOp(NotInHole, op', ph(0), ph(1));
    Parenthesized(OpSeq(skel, ExpOpExp(a, op', b)));
  };
  /* Save it for another day
     match (a, b) with
       | (OpSeq skelA opseqA, OpSeq skelB opseqB) ->
       | (OpSeq skelA opseqA, _) ->
       | (_, OpSeq skelB opseqB) ->
       | (_, _) ->
         OpSeq (Skel.BinOp NotInHole op' ?? ??) (OperatorSeq.ExpOpExp a op' b)
     end
     */

  switch (ty) {
  | Hole => Hole
  | Unit => Unit
  | Num => Num
  | Bool => Bool
  | Arrow(ty1, ty2) => mk_opseq(Arrow, contract(ty1), contract(ty2))
  | Prod(ty1, ty2) => mk_opseq(Prod, contract(ty1), contract(ty2))
  | Sum(ty1, ty2) => mk_opseq(Sum, contract(ty1), contract(ty2))
  | List(ty1) => List(contract(ty1))
  };
};

let rec elab = (uty: t): HTyp.t =>
  switch (uty) {
  | Hole => Hole
  | Unit => Unit
  | Num => Num
  | Bool => Bool
  | Parenthesized(uty1) => elab(uty1)
  | List(uty1) => List(elab(uty1))
  | OpSeq(skel, seq) => elab_skel(skel, seq)
  }
and elab_skel = (skel: skel_t, seq: opseq): HTyp.t =>
  switch (skel) {
  | Placeholder(n) =>
    switch (OperatorSeq.nth_tm(n, seq)) {
    | Some(uty_n) => elab(uty_n)
    | None => Hole /* should never happen */
    }
  | BinOp(_, Arrow, skel1, skel2) =>
    let uty1 = elab_skel(skel1, seq);
    let uty2 = elab_skel(skel2, seq);
    Arrow(uty1, uty2);
  | BinOp(_, Prod, skel1, skel2) =>
    let uty1 = elab_skel(skel1, seq);
    let uty2 = elab_skel(skel2, seq);
    Prod(uty1, uty2);
  | BinOp(_, Sum, skel1, skel2) =>
    let uty1 = elab_skel(skel1, seq);
    let uty2 = elab_skel(skel2, seq);
    Sum(uty1, uty2);
  };

let rec max_degree =
  fun
  | Hole
  | Unit
  | Num
  | Bool => 0
  | Parenthesized(body)
  | List(body) => max(1, max_degree(body))
  | OpSeq(_, seq) =>
    OperatorSeq.tms(seq)
    |> List.map(max_degree)
    |> List.fold_left(max, OperatorSeq.seq_length(seq));

let child_indices =
  fun
  | Hole
  | Unit
  | Num
  | Bool => []
  | Parenthesized(_)
  | List(_) => [0]
  | OpSeq(_, seq) => range(OperatorSeq.seq_length(seq));

let favored_child: t => option((child_index, t)) =
  fun
  | Hole
  | Unit
  | Num
  | Bool
  | OpSeq(_, _) => None
  | Parenthesized(ty)
  | List(ty) => Some((0, ty));
