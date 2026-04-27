# grd.c - Grid | Region | Divide

`grd.c` is a zero-dependency passive geometric calculus engine for dynamic
layouts written in pure C.

It computes proportional bounding boxes for tiling layouts. It handles
splitting, weight-based sizing, gap spacing, hit-testing, drag-resize math, and
dynamic restructuring without rendering or event handling.

## Layout

```text
src/grd.c
src/libgrd.c
src/grd.h
bin/{arch}/{platform}/
```

`src/grd.c` contains the CLI entry point. `src/libgrd.c` contains the library
implementation. `src/grd.h` is the public API.

## Build

```bash
make all
make x86_64/linux
make test
make clean
```

Each target writes artifacts to `bin/{arch}/{platform}/`:

- `grd`
- `libgrd.a`
- `libgrd.so`

Windows targets produce `grd.exe`, `libgrd.a`, `libgrd.dll.a`, and
`libgrd.dll`.

`make clean` removes only `.build/`; it does not delete `bin/`.

Native optimization is disabled by default. Enable it only for local native
builds with:

```bash
cmake -S . -B .build/native -DGRD_NATIVE=ON
```

## Usage

```bash
./bin/x86_64/linux/grd split [options]
```

Compute proportional child bounds for a root box:

```bash
./bin/x86_64/linux/grd split -w 1920 -H 1080 -k row -W "1 2 1"
```

Output is one line per child:

```text
0 0 0 480 1080
1 480 0 960 1080
2 1440 0 480 1080
```

Options:

- `--width`, `-w <n>`: Root width in pixels.
- `--height`, `-H <n>`: Root height in pixels.
- `--kind`, `-k <row|col>`: Split direction.
- `--weights`, `-W <...>`: Space or comma separated weights.
- `--gap`, `-g <n>`: Gap between children in pixels.
- `--min`, `-m <n>`: Minimum child size in pixels.
- `--version`, `-v`: Show version.
- `--help`, `-h`: Show help.

## Public API

```c
typedef enum {
    KC_GRD_ROW = 1,
    KC_GRD_COL = 2
} kc_grd_kind_t;
```

Box lifecycle:

```c
kc_grd_box_t *kc_grd_box_new(void);
void kc_grd_box_free(kc_grd_box_t *b);
```

`kc_grd_box_new()` returns a caller-owned box with `border=1` and `padding=1`.
After `kc_grd_split_add()` succeeds, ownership of the child transfers to the
split and the caller must not free the child directly.

Split setup:

```c
kc_grd_split_t *kc_grd_split_set(kc_grd_box_t *b, kc_grd_kind_t kind);
int kc_grd_split_add(kc_grd_split_t *s, kc_grd_box_t *child, float weight);
int kc_grd_split_weight(kc_grd_split_t *s, int index, float weight);
void kc_grd_split_gap(kc_grd_split_t *s, int gap, int min_px);
kc_grd_box_t *kc_grd_split_at(const kc_grd_split_t *s, int i);
```

Layout:

```c
void kc_grd_box_bounds(kc_grd_box_t *b, int x, int y, int w, int h);
void kc_grd_box_layout(kc_grd_box_t *b);
```

Hit-testing and drag-resize:

```c
int kc_grd_gap_hit(kc_grd_box_t *b, int x, int y, kc_grd_gap_t *out);
int kc_grd_drag_begin(const kc_grd_gap_t *gap, int x, int y);
int kc_grd_drag_update(kc_grd_split_t *s, int x, int y);
void kc_grd_drag_end(kc_grd_split_t *s);
```

Dynamic restructuring:

```c
int kc_grd_box_close(kc_grd_box_t *b);
```

`kc_grd_box_close()` removes a box from its parent. When one child remains, that
sibling is promoted into the parent and the split is dissolved. The caller must
not use the removed box after a successful call.

## Threading

The library has no internal locking and no global mutable state. Operations on
the same tree must be serialized by the caller. Separate trees may be operated
on concurrently by caller-created threads.

The CLI is a single command process and does not create worker threads.

## Validation

```bash
make all
make test
kc kcs .
```

---

**Author:** KaisarCode

**Email:** kaisarcode@gmail.com

**Website:** [https://kaisarcode.com](https://kaisarcode.com)

**License:** [GNU GPL v3.0](https://www.gnu.org/licenses/gpl-3.0.html)

© 2026 KaisarCode
