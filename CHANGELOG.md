# CHANGELOG

## v1.1.1

- Fixed `grd_signal_listener` to restore default signal behavior (SIG_DFL)
  when no `on_signal` handler is registered.

## v1.1.0

- Added data-driven configuration with table-driven environment variable loading.
- Added `kc_grd_options_default()`, `kc_grd_options_load_env()`, and `kc_grd_options_free()` to the public API.
- CLI is now decoupled from `libgrd`; configuration is initialized through options, then overridden by flags.
- Env vars: `KC_GRD_WIDTH`, `KC_GRD_HEIGHT`, `KC_GRD_KIND`, `KC_GRD_WEIGHTS`, `KC_GRD_GAP`, `KC_GRD_MIN_PX`.
- Added signal listener lifecycle: `kc_grd_on_signal()`, `kc_grd_raise_signal()`, `kc_grd_listen_signals()`, `kc_grd_listen_signal()`, and `kc_grd_signal_listener()`.

## v1.0.0

- Published the stable baseline release.
- Provided passive grid layout computation with row and column splits.
- Supported CLI layout output and a public C API for box and split operations.
