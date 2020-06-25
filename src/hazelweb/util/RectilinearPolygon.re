module Vdom = Virtual_dom.Vdom;

type cmd =
  | Dx(float)
  | Dy(float);
// invariant: nonempty
type t = list(cmd);

type point = {x: float, y: float};

type rect = {
  min: point,
  width: float,
  height: float,
};

// private types
type linked_edge = {
  src: point,
  dst: point,
  mutable next: option(linked_edge),
};

let cmd_of_linked_edge = edge => {
  let (x1, y1) = edge.src;
  let (x2, y2) = edge.dst;
  x1 == x2
  ? V(y2 -. y1)
  : H(x2 -. x1);
};

let inc = (d, cmd) =>
  switch (cmd) {
  | H(dx) => H(dx +. d)
  | V(dy) => V(dy +. d)
  };

let string_of_cmd =
  fun
  | H(dx) => Printf.sprintf("h %s ", FloatUtil.to_string_zero(dx))
  | V(dy) => Printf.sprintf("v %s ", FloatUtil.to_string_zero(dy));

let get_scalar =
  fun
  | Dx(f)
  | Dy(f) => f;

let mk_svg = (
  ~attrs: list(Vdom.Attr.t),
  ~corner_radii as (rx, ry): (float, float),
  rects: list(rect),
): t => {
  assert(rects != []);

  let is_left_side = (edge: linked_edge): bool => {
    let (_, y_src) = edge.src;
    let (_, y_dst) = edge.dst;
    y_src > y_dst;
  };

  let sorted_vertical_sides =
    rects
    |> List.concat_map(({min, width, height}) => {
      let max_x = min.x +. width;
      let max_y = min.y +. height;
      [
        // left sides point in negative direction
        {src: (min.x, max_y), dst: (min.x, min.y), next: None},
        // right sides point in positive direction
        {src: (max_x, min.y), dst: (max_x, max_y), next: None},
      ]
    })
    |> List.sort((v1, v2) => {
      if (v1.src.x < v2.src.x) {
        -1;
      } else if (v1.src.x > v2.src.x) {
        1;
      } else {
        // for vertical sides of equal abscissa,
        // need to sort left sides before right sides
        let is_left1 = is_left_side(v1);
        let is_left2 = is_left_side(v2);
        if (is_left1 && !is_left2) {
          -1;
        } else if (!is_left1 && is_left2) {
          1;
        } else {
          0;
        };
      };
    });

  let vertical_contour_edges =
    sorted_vertical_sides
    |> ListUtil.map_with_accumulator((tree, v) => {
      let ys = (v.src.y, v.dst.y);
      let mk_contour_edge = ((y_src, y_dst)) => {
        {
          src: {x, y: y_src},
          dst: {x, y: y_dst},
          next: None,
        };
      };
      if (is_left_side(v)) {
        let new_contour_edges =
          SegmentTree.contribution(ys, tree)
          |> List.map(mk_contour_edge);
        let updated_tree = SegmentTree.insert(ys, tree);
        (updated_tree, new_contour_edges);
      } else {
        let updated_tree = SegmentTree.delete(ys, tree);
        let new_contour_edges =
          SegmentTree.contribution(ys, updated_tree)
          |> List.map(mk_contour_edge);
        (updated_tree, new_contour_edges);
      }
    })
    |> snd
    |> List.flatten;

  vertical_contour_edges
  |> List.concat_map(v => [(false, v), (true, v)])
  |> List.sort(((is_src1, v1), (is_src2, v2)) => {
    let pt1 = is_src1 ? v1.src : v2.dst;
    let pt2 = is_src2 ? v2.src : v2.dst;
    if (pt1.y < pt2.y) {
      -1;
    } else if (pt2.y > pt1.y) {
      1;
    } else {
      Float.compare(pt1.x, pt2.x);
    };
  })
  |> ListUtil.disjoint_pairs
  |> List.iter((((is_src1, v1), (_, v2))) => {
    let (x_src, x_dst, y, prev, next) =
      is_src1
      ? (v2.dst.x, v1.src.x, v1.src.y, v2, v1)
      : (v1.dst.x, v2.src.x, v2.src.y, v1, v2);
    let h = {
      src: {x: x_src, y},
      dst: {x: x_dst, y},
      next,
    };
    prev.next = h;
  });

  let start = List.hd(vertical_contour_edges);
  let rec build_path = (edge: linked_edge): list(cmd) => {
    switch (edge.next) {
    | None => failwith("acyclic path")
    | Some(next) =>
      next == start
      ? [cmd_of_linked_edge(start)]
      : [cmd_of_linked_edge(next), ...build_path(next)]
    }
  };
  let path = build_path(start);

  // TODO refine estimate
  let buffer = Buffer.create(List.length(path) * 20);
  path
  |> List.concat_map(
    fun
    | H(dx) => [H(dx *. 0.5), H(dx *. 0.5)]
    | V(dy) => [V(dy *. 0.5), V(dx *. 0.5)]
  )
  |> ListUtil.rotate
  |> ListUtil.disjoint_pairs(((cmd1, cmd2)) => {
    switch (cmd1, cmd2) {
    | (H(_), H(_))
    | (V(_), V(_)) => assert(false)
    | (H(dx), V(dy)) =>
      let rx_min = min(rx, Float.abs(dx));
      let ry_min = min(ry, Float.abs(dy));
      let (rx, ry) =
        ry_min *. rx >= rx_min *. ry
          ? (rx_min, rx_min *. ry /. rx) : (ry_min *. rx /. ry, ry_min);
      let clockwise = Float.sign(dx) == Float.sign(dy);
      Printf.sprintf(
        "h %s a %s %s 0 0 %s %s %s v %s ",
        FloatUtil.to_string_zero(Float.copy_sign(Float.abs(dx) -. rx, dx)),
        FloatUtil.to_string_zero(rx),
        FloatUtil.to_string_zero(ry),
        clockwise ? "1" : "0",
        FloatUtil.to_string_zero(Float.copy_sign(rx, dx)),
        FloatUtil.to_string_zero(Float.copy_sign(ry, dy)),
        FloatUtil.to_string_zero(Float.copy_sign(Float.abs(dy) -. ry, dy)),
      );
    | (V(dy), H(dx)) =>
      let rx_min = min(rx, dx);
      let ry_min = min(ry, dy);
      let (rx, ry) =
        ry_min *. rx >= rx_min *. ry
          ? (rx_min, rx_min *. ry /. rx) : (ry_min *. rx /. ry, ry_min);
      let clockwise = Float.sign(dy) != Float.sign(dx);
      Printf.sprintf(
        "v %s a %s %s 0 0 %s %s %s h %s ",
        FloatUtil.to_string_zero(Float.copy_sign(Float.abs(dy) -. ry, dy)),
        FloatUtil.to_string_zero(rx),
        FloatUtil.to_string_zero(ry),
        clockwise ? "1" : "0",
        FloatUtil.to_string_zero(Float.copy_sign(rx, dx)),
        FloatUtil.to_string_zero(Float.copy_sign(ry, dy)),
        FloatUtil.to_string_zero(Float.copy_sign(Float.abs(dx) -. rx, dy)),
      );
    };
  })
  |> List.iter(cmd_str => Buffer.add(buffer, cmd_str));

  Vdom.(
    Node.create_svg(
      "path",
      [Attr.create("d", Buffer.contents(buffer)), ...attrs],
      [],
    )
  );
};

/**
 * Merges consecutive collinear segments. Result
 * is a list alternating between `Dx` and `Dy`.
 */
let rec compress = (path: t): t =>
  switch (path) {
  | [] => []
  | [Dx(dx1), Dx(dx2), ...path] => compress([Dx(dx1 +. dx2), ...path])
  | [Dy(dy1), Dy(dy2), ...path] => compress([Dy(dy1 +. dy2), ...path])
  | [d, ...path] => [d, ...compress(path)]
  };

let h = (f: float): string =>
  Printf.sprintf("h %s ", FloatUtil.to_string_zero(f));
let v = (f: float): string =>
  Printf.sprintf("v %s ", FloatUtil.to_string_zero(f));

let rounded_corner = (
  (rx: float, ry: float),
  enter: cmd,
  exit: cmd,
): string => {
  let (dx, dy) =
    switch (enter, exit) {
    | (Dx(_), Dx(_))
    | (Dy(_), Dy(_)) => failwith("invalid argument")
    | (Dx(x), Dy(y))
    | (Dy(y), Dx(x)) =>
      (
        Float.copy_sign(rx, x),
        Float.copy_sign(ry, y),
      )
    };
  let sweep =
    switch (enter, exit) {
    | (Dx(_), Dx(_))
    | (Dy(_), Dy(_)) => failwith("invalid argument")
    | (Dx(x), Dy(y)) =>
      (x >= 0.0) != (y >= 0.0)
    | (Dy(y), Dx(x)) =>
      (x >= 0.0) == (y >= 0.0)
    };
  Printf.sprintf(
    "a %s %s 0 0 %s %s %s ",
    FloatUtil.to_string_zero(rx),
    FloatUtil.to_string_zero(ry),
    sweep ? "1" : "0",
    FloatUtil.to_string_zero(dx),
    FloatUtil.to_string_zero(dy),
  );
};

let to_svg = (
  ~attrs: list(Vdom.Attr.t),
  ~corner_radii as (rx, ry): (float, float),
  path: t
): Vdom.Node.t => {
  let compressed_and_split =
    path
    |> compress
    |> List.map(
      fun
      | Dx(dx) => {
          let half = dx *. 0.5;
          [Dx(half), Dx(half)];
        }
      | Dy(dy) => {
          let half = dy *. 0.5;
          [Dy(half), Dy(half)]
        }
    )
    |> List.flatten;

  let buffer = Buffer.create(List.length(compressed_and_split) * 20);
  let rec fill_buffer = (compressed_and_split: t): unit =>
    switch (compressed_and_split) {
    | [] => ()
    | [_]
    | [Dx(_), Dx(_), ..._]
    | [Dy(_), Dy(_), ..._] => assert(false)
    | [Dx(dx) as enter, Dy(dy) as exit, ...path]
    | [Dy(dy) as enter, Dx(dx) as exit, ...path] =>
      let dx' = Float.abs(dx);
      let dy' = Float.abs(dy);
      // Corner rounding cuts into the lengths of the entering and
      // exiting edges. Find the maximum (proportionally scaled)
      // radii possible given lengths of entering and exiting edges.
      let (rx', ry') as radii = {
        let rx_min = min(rx, dx');
        let ry_min = min(ry, dy');
        ry_min *. rx >= rx_min *. ry
          ? (rx_min, rx_min *. ry /. rx) : (ry_min *. rx /. ry, ry_min);
      };
      let edge_svg =
        fun
        | Dx(dx) => h(Float.copy_sign(dx' -. rx', dx))
        | Dy(dy) => v(Float.copy_sign(dy' -. ry', dy));
      let corner_svg = rounded_corner(radii, enter, exit);
      Buffer.add_string(buffer, edge_svg(enter));
      Buffer.add_string(buffer, corner_svg);
      Buffer.add_string(buffer, edge_svg(exit));
      fill_buffer(path);
    };
  fill_buffer(compressed_and_split);

  Vdom.(
    Node.create_svg(
      "path",
      [Attr.create("d", Buffer.contents(buffer)), ...attrs],
      [],
    )
  );
};