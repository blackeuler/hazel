module Vdom = Virtual_dom.Vdom;

let span = Vdom.Node.span;
let txt = Vdom.Node.text;

let code = s => span([Vdom.Attr.classes(["code"])], [txt(s)]);

let cardstack: CardStack.t =
  Vdom.[
    {
      caption:
        span(
          [],
          [
            txt(
              "Suppose we are implementing a combat game "
              ++ "and, specifically, defining the function ",
            ),
            Node.p(
              [Attr.create("style", "text-align: center;")],
              [code("damage : (Bool, Num) -> Num"), txt(".")],
            ),
            txt("The input tuple of type "),
            code("(Bool, Num)"),
            txt(
              " represents an enemy attack dealt to the "
              ++ "current player, consisting of a ",
            ),
            code("Bool"),
            txt(" indicating whether the attack is a melee attack, and a "),
            code("Num"),
            txt(
              " representing the critical hit multiplier. The output type of ",
            ),
            code("Num"),
            txt(
              " is the damage points inflicted upon the current player. "
              ++ "Take a moment to understand the current implementation, "
              ++ "then click \'Next\' when you are ready to begin the exercise.",
            ),
          ],
        ),
    },
    {
      caption:
        span(
          [],
          [
            txt("Suppose we have in scope the current player's"),
            code(" defenseScore : Num "),
            txt(
              "and wish to integrate it into the damage calculation. "
              ++ "Modify the body of ",
            ),
            code("damage"),
            txt(" so that the "),
            code("case"),
            txt(" expression is bound to a new variable called "),
            code("attackScore"),
            txt(", then return twice the difference between "),
            code("attackScore"),
            txt(" and "),
            code("defenseScore"),
            txt("."),
          ],
        ),
    },
  ];
