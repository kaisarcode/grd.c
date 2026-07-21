# AGENTS.md

## Project Context

`grd.c` is a small passive geometry library and CLI for recursively dividing
integer rectangles by proportional weights.

It computes boxes, gaps, and drag-adjusted weights. It does not render, own
windows, process pointer events, manage widgets, or provide a UI framework.

Read `README.md` and `DESIGN.md` before modifying the project.

## Core Invariants

- Layout is a tree of boxes with at most one split per box.
- `KC_GRD_ROW` divides the x-axis into side-by-side children.
- `KC_GRD_COL` divides the y-axis into stacked children.
- Child sizes are integer pixels derived from positive proportional weights.
- Gaps consume axis space before child distribution.
- Integer remainder is distributed from earlier children forward.
- Border and padding produce one inset equal to their non-negative maximum.
- Adding a child transfers ownership to the parent split.
- Freeing a root recursively frees every owned descendant.
- Gap hit-testing and drag updates remain pure geometry operations.
- Closing a child may dissolve its split and promote the sole survivor.
- No renderer, event loop, platform window, or persistent state is required.

## Geometry Contract

Preserve the axis meaning, coordinate origin, half-open hit tests, gap placement,
rounding order, and exact CLI output `index x y w h`.

Weights at most zero are normalized to `1.0` by the library. Negative bounds,
gap, border, padding, and minimum size are clamped where current functions define
that behavior. The CLI is stricter and accepts only positive weights and root
dimensions.

Layout currently assumes minimum sizes are feasible. If
`child_count * min_px` exceeds usable axis space, the size-reduction loop cannot
satisfy both constraints. Callers must not rely on impossible layouts. A fix
must fail or degrade deterministically rather than loop indefinitely.

Do not silently change rounding, minimum-size priority, or overflow behavior.

## Tree Ownership

Public box and split structures are compatibility boundaries. A child is
caller-owned until `kc_grd_split_add()` succeeds, then owned by the split. A box
owned by a split must not be freed directly.

`kc_grd_split_set()` recursively destroys an existing split before installing a
new one. `kc_grd_box_close()` destroys the target and invalidates its pointer;
when one sibling remains, that sibling's style and split are promoted into the
parent and the sibling pointer is freed.

Changes must preserve parent pointers, split owners, child ordering, recursive
cleanup, and explicit pointer invalidation. Do not add reference counting,
garbage collection, hidden registries, or shared ownership.

## Drag Boundary

Gap hit-testing searches the current laid-out tree and returns a borrowed split
pointer plus adjacent-child index. Drag begin snapshots the adjacent pixel sizes
and weights. Drag update changes only those two weights while preserving their
combined weight and respecting `min_px`. Callers invoke layout afterward.

The library does not capture devices, transform coordinates, debounce input,
draw cursors, or own gesture state beyond one active drag per split.

## Public API and Concurrency

Treat `src/libgrd.h` as a compatibility boundary. Its structures, enums, fields,
ownership rules, defaults, return values, and pointer lifetimes are public.

Trees are mutable and not thread-safe. Layout, mutation, drag, close, and free
must not run concurrently on the same tree. Stop state does not cancel layout;
do not claim asynchronous operation.

## Resource Model

Memory grows with boxes, splits, child arrays, and weights.
Layout allocates one temporary integer-size array per visited split and recurses
through the tree. Deep trees can consume call stack.

Keep allocation failure, integer arithmetic, recursion depth, impossible
constraints, and cleanup explicit. Do not add render caches, retained scenes,
worker threads, remote layout services, or generic object systems.

## Source Layout

Preserve exactly:

- `src/grd.c` for CLI parsing and geometry output;
- `src/libgrd.c` for tree, layout, gap, drag, and reusable behavior;
- `src/libgrd.h` for public structures and API;
- `src/test.c` for all tests, including deep-tree, stress, platform, and
    integration cases.

Do not create additional source, header, layout, geometry, drag, tree, or test
files. Extend only the existing four files.

## Forbidden Default Recommendations

Do not add renderers, widget toolkits, styling systems, scene graphs, animation
engines, window managers, browser layout models, constraint solvers, GPU APIs,
event buses, plugins, telemetry, dashboards, cloud layout services, accounts, or
enterprise UI abstractions.

Do not justify changes through framework parity, hypothetical scale, managed UI
platforms, ecosystem growth, or enterprise readiness.

## Testing

All tests remain in `src/test.c`. Behavioral changes should cover both axes,
integer remainder, gaps, border and padding, zero space, feasible and impossible
minimums, invalid and extreme weights, nested layout, allocation failure, gap
boundaries, drag limits, child removal, sole-child promotion, parent pointers,
ownership, deep trees, and exact CLI rows.

Do not weaken tests to accommodate an implementation change.

## Build and Completion

For documentation-only changes run `kcs .`. For behavior changes use the
repository build and tests without cleaning unless authorized.

A change is complete when geometry, rounding, ownership, drag state, failure
behavior, tests, and documentation agree.

The goal is one small, deterministic rectangle-divider.
