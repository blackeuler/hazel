open Virtual_dom;

/**
 * Raised when a selected hole instance does not exist.
 * Indicates a bug.
 */
exception InvalidInstance;

/**
 * Panel showing the typing context and environment at the cursor.
 */
let view:
  (
    ~inject: ModelAction.t => Vdom.Event.t,
    ~selected_instance: option(HoleInstance.t),
    ~compute_results: Model.compute_results,
    Program.t
  ) =>
  Vdom.Node.t;
