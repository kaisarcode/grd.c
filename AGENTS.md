# grd.c — Proportional Grid Layout Engine

## Overview
Passive hierarchical grid layout engine using recursive axis-aligned splits. Boxes contain splits (row or column), which distribute child boxes proportionally across the parent's inner area. Supports gap separators, minimum child sizes, drag-to-resize, tree restructuring on child close, and recursive layout. Designed as a composable native primitive for window tiling, UI layout, and spatial partitioning.

## Architecture
Three-file split: `grd.h` exposes `kc_grd_box_t`, `kc_grd_split_t`, `kc_grd_gap_t`. `libgrd.c` implements the tree (box create/free, split set/add/weight, layout recursion, gap hit-testing, drag begin/update/end, close promotion). `grd.c` is the CLI — parses weights, dimensions, kind, gap, min; builds a single-level tree; prints one `index x y w h` line per child. Layout is recursive and reentrant: `kc_grd_box_layout` calls `inner()` to compute the box's inset content area, then `layout_split` distributes child pixel sizes within that area. The split uses no external storage beyond the box/split tree — all state is in the caller-owned nodes.

## Algorithm
Layout proceeds in four phases per split. First, the usable pixel span along the split axis is computed as `(inner_size) - ((count - 1) * gap)`, clamped to zero. Second, each child receives a preliminary pixel size via `(usable * weight_i) / sum(weights)`, clamped to `min_px`. Third, any over- or under-use relative to the usable span is corrected: overshoot reduces the largest children (from last to first) down to `min_px`; undershoot adds to children from first to last. Fourth, each child's outer bounds are set along the split axis with gap separation, `inner()` is called to compute the child's content inset (using `max(border, padding)` as the inset), and `kc_grd_box_layout` recurses into any sub-split. Row splits operate on `inner_w`, col splits on `inner_h` — the axis-agnostic `axis()` helper selects the relevant coordinate, so the same code path handles both directions.

## Directory Layout
| Path | Contents |
|------|----------|
| `src/grd.h` | Public API — box, split, gap types; function declarations |
| `src/libgrd.c` | Library implementation — tree ops, layout, drag, close promotion |
| `src/grd.c` | CLI entry point — argv parsing, split subcommand, output |
| `Makefile` | Cross-compilation builder (17 targets) via CMake + Ninja |
| `CMakeLists.txt` | CMake project definition |
| `test.sh` | Shell test suite — split arithmetic, gap, min, error cases |
| `README.md` | Project documentation and usage examples |
| `LICENSE` | GPL v3.0 |
| `.kcsignore` | KCS exclusion list |

## Data Model
### Internal Structures
| Symbol | Type | Role |
|--------|------|------|
| `kc_grd_box_t` | `struct kc_grd_box` | Tree node: parent, split, bounds, inner area, border, padding |
| `kc_grd_split_t` | `struct kc_grd_split` | Split engine: owner, kind, children array, weights array, count/cap, gap, min_px, drag state |
| `kc_grd_gap_t` | `struct` | Gap descriptor: position, size, adjacent child index, owning split |
| `kc_grd_kind_t` | `enum` | `KC_GRD_ROW` (1) or `KC_GRD_COL` (2) |

### Hard Limits
| Limit | Value | Symbol |
|-------|-------|--------|
| Max CLI weights | 256 | `KC_GRD_WEIGHTS_CAP` |
| Initial children capacity | 4 | `next` in `kc_grd_split_add` |
| Children capacity growth | doubles | `s->cap * 2` |
| Weight text buffer | 4096 bytes | `buffer` in `kc_grd_parse_weights` |
| Default border | 1 | `b->border` |
| Default padding | 1 | `b->padding` |
| Default gap (API) | 1 | `s->gap` |
| Default gap (CLI) | 0 | `gap` default |
| Default min_px (API) | 4 | `s->min_px` |
| Default min_px (CLI) | 1 | `min_px` default |

## Public API
| Function | Returns | Description |
|----------|---------|-------------|
| `kc_grd_box_new(void)` | `kc_grd_box_t *` | Allocate box with border=1, padding=1; caller owns until split_add |
| `kc_grd_box_free(kc_grd_box_t *b)` | `void` | Recursively free box, split, and all descendants |
| `kc_grd_split_set(kc_grd_box_t *b, kc_grd_kind_t kind)` | `kc_grd_split_t *` | Attach new split, replacing existing; sets gap=1, min_px=4 |
| `kc_grd_split_add(kc_grd_split_t *s, kc_grd_box_t *child, float weight)` | `int` | Add child with weight; transfers ownership; 0 OK, -1 fail |
| `kc_grd_split_weight(kc_grd_split_t *s, int index, float weight)` | `int` | Update child weight; 0 OK, -1 on invalid index |
| `kc_grd_box_bounds(kc_grd_box_t *b, int x, int y, int w, int h)` | `void` | Set outer bounds and recompute inner area |
| `kc_grd_box_layout(kc_grd_box_t *b)` | `void` | Recursively recompute layout for box and children |
| `kc_grd_split_gap(kc_grd_split_t *s, int gap, int min_px)` | `void` | Configure gap and minimum child pixel size |
| `kc_grd_gap_hit(kc_grd_box_t *b, int x, int y, kc_grd_gap_t *out)` | `int` | Test point against gap separators; 1 hit, 0 miss |
| `kc_grd_drag_begin(const kc_grd_gap_t *gap, int x, int y)` | `int` | Start drag-resize; records anchor and initial pixel sizes |
| `kc_grd_drag_update(kc_grd_split_t *s, int x, int y)` | `int` | Update weights during drag; 0 OK, -1 no drag, 0 if min_px violated |
| `kc_grd_drag_end(kc_grd_split_t *s)` | `void` | End drag and reset drag state |
| `kc_grd_box_close(kc_grd_box_t *b)` | `int` | Remove box from parent; promotes single-child splits |
| `kc_grd_split_at(const kc_grd_split_t *s, int i)` | `kc_grd_box_t *` | Return child at index, or NULL |

## CLI
| Argument | Description |
|----------|-------------|
| `split` | Subcommand — compute proportional grid splits |
| `-w`, `--width <n>` | Root width in pixels (required) |
| `-H`, `--height <n>` | Root height in pixels (required) |
| `-k`, `--kind row\|col` | Split direction (default: row) |
| `-W`, `--weights <...>` | Space or comma separated positive floats (required, min 2) |
| `-g`, `--gap <n>` | Gap between children in pixels (default: 0) |
| `-m`, `--min <n>` | Minimum child size in pixels (default: 1) |
| `-h`, `--help` | Show help and exit 0 |
| `-v`, `--version` | Show version and exit 0 |

Output: one `index x y w h` line per child to stdout.

### Exit Codes
| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Error (missing/invalid flags, allocation failure, split failure) |

## Build
| Target | Description |
|--------|-------------|
| `make` (default: `native`) | Build for host arch/platform |
| `make all` | Build full 17-target cross-compilation matrix |
| `make <arch>/<platform>` | Build specific target (x86_64/linux, i686/windows, etc.) |
| `make test` | Run `sh test.sh` |
| `make clean` | Remove `.build/` |

## Error Handling
| Condition | Stderr | Exit Code |
|-----------|--------|-----------|
| Missing `--width` | `"Missing required --width."` + usage | 1 |
| Missing `--height` | `"Missing required --height."` + usage | 1 |
| Missing `--weights` | `"Missing required --weights."` + usage | 1 |
| Fewer than 2 weights | `"At least two weights are required."` + usage | 1 |
| Invalid value for any flag | `"Invalid value for <flag>."` + usage | 1 |
| Unknown command | `"Unknown command."` + usage | 1 |
| Unknown argument | `"Unknown argument."` + usage | 1 |
| Invalid kind (not row/col) | `"Invalid value for --kind. Use row or col."` + usage | 1 |
| Allocation failure | `"Error: allocation failed."` | 1 |
| Split setup failure | `"Error: split setup failed."` | 1 |
| Split add failure | `"Error: split add failed."` | 1 |

## Constraints
- Single-level CLI: always creates one split under a borderless/paddingless root — no recursive CLI input format.
- Weights must be positive (> 0); zero or negative clamped to 1.0 internally.
- Drag preserves total weight sum of the two adjacent children; min_px enforcement returns 0 without updating on violation.
- Box close promotes single-child splits into the parent (absorbing border/padding) — the child box replaces the parent in the tree.
- Gap hit searches depth-first and returns the first match.
- No thread-safety guarantees on concurrent mutation of the same tree.
- POSIX-only CLI (`strtok_r`, `strtol`, `strtof`); library is portable C99.
