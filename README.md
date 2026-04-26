# grd.c - Grid | Region | Divide

`grd.c` is a zero-dependency passive geometric calculus engine for dynamic layouts written in pure C.

It computes proportional bounding boxes for tiling layouts. It handles splitting, weight-based sizing, gap spacing, hit-testing, and drag-resize math — entirely without rendering or event handling.

## Interface

`grd.c` follows the kclibs flat-library pattern:

- `grd.h` — public API
- `libgrd.c` — implementation
- `grd.c` — CLI wrapper

The CLI starts from:

```bash
grd split [options]
```

## Split

Compute proportional child bounds for a root box:

```bash
grd split -w 1920 -H 1080 -k row -W "1 2 1"
```

Output: one line per child — `index x y w h`

```
0 0 0 480 1080
1 480 0 960 1080
2 1440 0 480 1080
```

Column split with gap:

```bash
grd split -w 800 -H 600 -k col -W "1 1 1 1" -g 4
```

## Parameters

| Flag | Description | Default |
| :--- | :--- | :--- |
| `--width, -w` | Root width in pixels | required |
| `--height, -H` | Root height in pixels | required |
| `--kind, -k` | Split direction: `row` or `col` | `row` |
| `--weights, -W` | Space or comma separated weights (min 2) | required |
| `--gap, -g` | Gap between children in pixels | `0` |
| `--min, -m` | Minimum child size in pixels | `1` |
| `--version, -v` | Show binary version | |
| `--help, -h` | Show help | |

## Library API

### Types

```c
typedef enum { KC_GRD_ROW = 1, KC_GRD_COL = 2 } kc_grd_kind_t;
```

### Box lifecycle

```c
kc_grd_box_t *kc_grd_box_new(void);
void     kc_grd_box_free(kc_grd_box_t *b);
```

`kc_grd_box_new` returns a box with `border=1` and `padding=1`. After `kc_grd_split_add`, the split owns the child and `kc_grd_box_free` must not be called on it directly.

### Split setup

```c
kc_grd_split_t *kc_grd_split_set(kc_grd_box_t *b, kc_grd_kind_t kind);
int        kc_grd_split_add(kc_grd_split_t *s, kc_grd_box_t *child, float weight);
int        kc_grd_split_weight(kc_grd_split_t *s, int index, float weight);
void       kc_grd_split_gap(kc_grd_split_t *s, int gap, int min_px);
kc_grd_box_t   *kc_grd_split_at(const kc_grd_split_t *s, int i);
```

### Layout

```c
void kc_grd_box_bounds(kc_grd_box_t *b, int x, int y, int w, int h);
void kc_grd_box_layout(kc_grd_box_t *b);
```

Call `kc_grd_box_bounds` to set the root size, then `kc_grd_box_layout` to recursively distribute all children. Read computed positions from `box->x, box->y, box->w, box->h` and inner area from `box->inner_x, box->inner_y, box->inner_w, box->inner_h`.

### Hit-testing and drag-resize

```c
int  kc_grd_gap_hit(kc_grd_box_t *b, int x, int y, kc_grd_gap_t *out);
int  kc_grd_drag_begin(const kc_grd_gap_t *gap, int x, int y);
int  kc_grd_drag_update(kc_grd_split_t *s, int x, int y);
void kc_grd_drag_end(kc_grd_split_t *s);
```

### Dynamic restructuring

```c
int kc_grd_box_close(kc_grd_box_t *b);
```

Removes a box from its parent. When one child remains, that sibling is promoted into the parent and the split is dissolved.

## Threading

**Single-context**: no internal locking. All operations on the same tree must be serialized by the caller.

## Build

```bash
cmake -B build
cmake --build build
```

Native optimization (off by default):

```bash
cmake -B build -DGRD_NATIVE=ON
cmake --build build
```

## Testing

```bash
./test.sh
```

---

**Author:** KaisarCode

**Email:** <kaisar@kaisarcode.com>

**Website:** [https://kaisarcode.com](https://kaisarcode.com)

**License:** [GNU GPL v3.0](https://www.gnu.org/licenses/gpl-3.0.html)

© 2026 KaisarCode
