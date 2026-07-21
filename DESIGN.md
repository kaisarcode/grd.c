# grd.c Design

## Purpose

`grd.c` computes nested rectangular layouts from integer bounds, split axes,
gaps, minimum sizes, and proportional weights. It is passive: callers decide
when bounds change, when layout runs, and how resulting boxes are used.

## Architecture

A box owns geometry, style insets, and at most one split.
A split owns an ordered child array, parallel weight array, axis, gap,
minimum-size rule, and temporary drag state.

The four source files have fixed responsibilities:

- `src/grd.c` owns CLI parsing and flat split output;
- `src/libgrd.c` owns all tree and geometry behavior;
- `src/libgrd.h` exposes structures and the public contract;
- `src/test.c` contains all tests.

## Axis Model

`KC_GRD_ROW` divides the owner's inner width. Children share y and height while
x advances from left to right.

`KC_GRD_COL` divides the owner's inner height. Children share x and width while
y advances from top to bottom.

Coordinates and dimensions are integer pixels. Negative width and height are
clamped to zero when bounds are assigned.

## Inner Bounds

Each box has outer `x`, `y`, `w`, and `h` plus derived inner bounds. The inset is
the larger of border and padding after negative values are treated as zero.
It is applied once on every side; border and padding are not added together.

Inner width and height never become negative.

## Size Distribution

For a split with `n` children, usable axis space is:

```text
axis_size - (n - 1) * gap
```

Negative usable space becomes zero. Each initial child size is the truncated
floating-point share of usable space. A size below `min_px` is raised to that
minimum.

If total size exceeds usable space, the implementation repeatedly removes one
pixel from later children toward earlier children while remaining above the
minimum. If total size is short, it repeatedly adds one pixel from earlier
children toward later children. The final sizes therefore fill usable space
exactly when the minimum constraint is feasible.

When `n * min_px > usable`, no valid result exists and the current reduction
algorithm cannot converge. This is an implementation limit requiring explicit
validation or a documented degradation rule before accepting such input.

## Recursive Layout

Layout recomputes a box's inner bounds, distributes its immediate children, sets
each child outer bounds, recomputes each child inner bounds, and recursively lays
out descendants.

No geometry is cached beyond public box fields. Mutation does not automatically
trigger layout. Callers explicitly invoke `kc_grd_box_layout()`.

## Ownership

`kc_grd_box_new()` returns caller-owned storage with border and padding set to
one. `kc_grd_split_set()` gives a box one split and destroys any old split and
descendants. `kc_grd_split_add()` transfers a child to the split only on success.

The root owner frees the whole tree recursively. Public pointers into a tree are
borrowed and become invalid when their owning split is replaced, an ancestor is
freed, or close restructures the tree.

## Gap Hit Testing

Each separator occupies a half-open rectangular strip between two adjacent
children. Hit testing checks the current split first, then descendants in child
order. The first match returns its geometry, preceding-child index, and borrowed
split pointer.

Zero-width or zero-height gaps cannot be hit. Hit testing assumes layout fields
are current.

## Drag Resizing

Drag begin records one separator's axis coordinate, adjacent pixel sizes, and
two weights. Drag update applies pointer delta to those pixel sizes. If either
would fall below `min_px`, weights remain unchanged and the operation returns
success.

Otherwise the adjacent pair's total weight is redistributed in proportion to
the proposed pixel sizes. Other children are unchanged. Drag end clears snapshot
state. Updated weights affect geometry only after another layout pass.

## Closing Boxes

A root cannot be closed through `kc_grd_box_close()`. Closing a child frees its
subtree and removes its entry while preserving sibling order.

An empty split is removed. If one child remains, its border, padding, and split
are moved into the parent, descendant parent pointers are repaired, and the
temporary child box is freed. The parent's outer bounds and identity remain.

## CLI Contract

The CLI computes one non-nested split with root border and padding set to zero.
It accepts at most 256 positive weights from one space- or comma-separated
argument and prints one line per child:

```text
index x y w h
```

CLI width and height must be positive; gap and minimum must be non-negative.
Environment options are loaded before command-line overrides.

## Resource and Failure Model

Child arrays and weights grow geometrically. Tree destruction is recursive.
Layout allocates a temporary size array per split; allocation failure leaves that
split's previous child geometry unchanged and returns no error because layout is
void.

The implementation is single-threaded and has no locks, background work,
persistence, platform graphics state, or external dependencies beyond the C
runtime and math support.

## Non-Goals

The project does not render, create windows, process input devices, own an event
loop, style widgets, animate geometry, solve arbitrary constraints, measure text,
manage focus, persist layouts, synchronize trees, expose remote APIs, collect
telemetry, or provide plugins.

These exclusions define the tool rather than an unfinished roadmap.

## Change Criteria

A change must solve a concrete rectangle-layout problem, preserve public tree
ownership, define integer rounding and impossible constraints, maintain both
axis semantics, state pointer invalidation, keep drag behavior explicit, and
avoid absorbing rendering or widget policy.

Changes justified mainly by UI framework parity, generalized scene management,
enterprise platforms, or hypothetical scale should be rejected.

## Core Invariants

The project is defined by public owned box trees, one split per box, x-axis rows,
y-axis columns, weighted integer distribution, explicit gaps and minimums,
manual recursive layout, local drag weight adjustment, and no rendering layer.
