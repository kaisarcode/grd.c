# grd.c - Proportional Grid Layout Engine

`grd.c` is a minimalist C library and CLI for computing proportional grid splits. It provides a passive layout engine for hierarchical boxes and splits, designed as a composable native primitive for the KaisarCode ecosystem.

---

## CLI

Compute proportional splits for a given root area and set of weights.

### Examples

Split a 1920x1080 area into three rows with 1:2:1 proportions:

```bash
./bin/x86_64/linux/grd split -w 1920 -H 1080 -k row -W "1 2 1"
```

Split an 800x600 area into four equal columns with a 4px gap:

```bash
./bin/x86_64/linux/grd split -w 800 -H 600 -k col -W "1 1 1 1" -g 4
```

---

### Parameters

| Flag | Description |
| :--- | :--- |
| `-w <n>` | Root width in pixels (required) |
| `-H <n>` | Root height in pixels (required) |
| `-k row\|col` | Split direction (default: row) |
| `-W <weights>` | Space or comma separated weights (required) |
| `-g <n>` | Gap between children in pixels (default: 0) |
| `-m <n>` | Minimum child size in pixels (default: 1) |
| `-h`, `--help` | Show help and usage |
| `-v`, `--version` | Show version |

---

### Output

One line per child box, in the format `index x y w h`:

```
0 0 0 1920 270
1 0 270 1920 540
2 0 810 1920 270
```

---

## Public API

```c
#include "grd.h"

kc_grd_box_t *root = kc_grd_box_new();
kc_grd_split_t *s = kc_grd_split_set(root, KC_GRD_ROW);

kc_grd_split_add(s, kc_grd_box_new(), 1.0f);
kc_grd_split_add(s, kc_grd_box_new(), 2.0f);

kc_grd_box_bounds(root, 0, 0, 1920, 1080);
kc_grd_box_layout(root);

kc_grd_box_free(root);
```

---

## Lifecycle

- `kc_grd_box_new()` - allocates a new box. The caller owns the box until it is added to a split.
- `kc_grd_split_set()` - attaches a split engine to a box.
- `kc_grd_split_add()` - adds a child box to a split. Ownership of the child transfers to the parent box.
- `kc_grd_box_free()` - recursively releases a box and all its descendants.

---

## Build

```bash
make
make all
make x86_64/linux
make x86_64/windows
make i686/linux
make i686/windows
make aarch64/linux
make aarch64/android
make armv7/linux
make armv7/android
make armv7hf/linux
make riscv64/linux
make powerpc64le/linux
make mips/linux
make mipsel/linux
make mips64el/linux
make s390x/linux
make loongarch64/linux
make clean
```

Artifacts are generated under:

```
bin/{arch}/{platform}/
```

---

**Author:** KaisarCode

**Email:** <kaisar@kaisarcode.com>

**Website:** [https://kaisarcode.com](https://kaisarcode.com)

**License:** [GNU GPL v3.0](https://www.gnu.org/licenses/gpl-3.0.html)

© 2026 KaisarCode
