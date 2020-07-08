module EditAction = Action_common;
module Sexp = Sexplib.Sexp;
open Sexplib.Std;

[@deriving sexp]
type move_input =
  | Key(MoveKey.t)
  | Click(option((MetaVar.t, SpliceName.t)), CaretPosition.t);

[@deriving sexp]
type shift_history_info = {
  group_id: int,
  elt_id: int,
  call_by_mouseenter: bool,
};
[@deriving sexp]
type group_id = int;
[@deriving sexp]
type t =
  | EditAction(EditAction.t)
  | MoveAction(move_input)
  | LivelitAction(MetaVar.t, SerializedAction.t)
  | ToggleLeftSidebar
  | ToggleRightSidebar
  | LoadExample(Examples.id)
  | LoadCardstack(int)
  | NextCard
  | PrevCard
  // Result computation toggles
  | ToggleComputeResults
  | ToggleShowCaseClauses
  | ToggleShowFnBodies
  | ToggleShowCasts
  | ToggleShowUnevaluatedExpansion
  // Time measurement toggles
  | ToggleMeasureTimes
  | ToggleMeasureModel_perform_edit_action
  | ToggleMeasureProgram_get_doc
  | ToggleMeasureLayoutOfDoc_layout_of_doc
  | ToggleMeasureUHCode_view
  | ToggleMeasureCell_view
  | ToggleMeasurePage_view
  | ToggleMeasureHazel_create
  | ToggleMeasureUpdate_apply_action
  //
  | ToggleMemoizeDoc
  | SelectInstance(TaggedNodeInstance.kind, NodeInstance.t)
  | SelectCaseBranch(CursorPath_common.steps, int)
  | InvalidVar(string)
  | FocusCell
  | BlurCell
  | Redo
  | Undo
  | ShiftHistory(shift_history_info)
  | ShiftWhenScroll
  | ToggleHistoryGroup(group_id)
  | ToggleHiddenHistoryAll
  | TogglePreviewOnHover
  | UpdateFontMetrics(FontMetrics.t)
  | UpdateIsMac(bool);