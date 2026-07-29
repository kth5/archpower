# chromium 150 for Arch POWER — ppc64le adaptations

This PKGBUILD is upstream Arch's `chromium` (commit d8dd314, 150.0.7871.128-1)
with the minimum set of changes needed to build on ppc64le. It builds and
runs on POWER9. The x86_64 build path is preserved and functionally
unchanged — all ppc64le-specific logic is guarded by `[[ $CARCH == powerpc64le ]]`.

## Changes at a glance

- `arch=('x86_64' 'powerpc64le')`
- `_manual_clone=0` — depot_tools / cipd can't bootstrap on ppc64le
  (Google's CIPD infrastructure has no ppc64le CPython package). The
  official `-lite.tar.xz` source drop from
  `commondatastorage.googleapis.com/chromium-browser-official/` is
  arch-neutral and works.
- New source: `chromium-ppc64le-patches-r1.tar.gz` — bundle of the
  Debian chromium-team's ppc64le patch set (43 patches, verbatim from
  `salsa.debian.org/chromium-team/chromium/-/tree/master/debian/patches/ppc64le`
  as of Debian 150.0.7871.124-1). Extracts to `$srcdir/ppc64le-patches/`
  and is applied in series order by a small loop in `prepare()`.
- New source: `swiftshader-ppc-xcoff-baseclasses.patch` — adds
  `MCAsmInfoXCOFF.cpp` and `MCXCOFFObjectTargetWriter.cpp` to
  swiftshader's bundled LLVM PPC target. Without these, the PPC XCOFF
  writer inherits from undefined base classes and `libvk_swiftshader.so`
  fails to link. Linux ppc64le uses ELF, but the vtables still need to
  resolve.
- New source: `git+https://gn.googlesource.com/gn#commit=...` — chromium
  150 uses several dotfile / tool attributes (`expand_directory_allowlist`,
  `expand_directory()`, `inputs = rustc_wrapper_inputs` on rust_* tools,
  `c_additional_outputs` on config() blocks) that Arch POWER's current
  `gn` package (0.2324) doesn't recognize. `prepare()` builds a fresh gn
  from source (~1 min, 5 MB source drop) and `build()` invokes it. This
  entire block — the gn source in source=(), the gn build in prepare(),
  and the `_gn=` override in build() — can be removed once Arch POWER's
  `gn` package is refreshed.
- `build()` strips `-mcpu=/-mtune=` from CFLAGS/CXXFLAGS on ppc64le,
  matching the existing aarch64/riscv64 pattern. Per-TU baseline choices
  in libvpx / skia / pffft need to win over a global -mcpu. The chromium
  binary itself has POWER8 baseline set by the Debian
  `Force-baseline-POWER8-AltiVec-VSX` patch.

## Missing pacman packages relative to a stock Arch POWER install

Reviewers building this on a fresh install may need:

```
sudo pacman -S --needed ttf-liberation lld rust-bindgen \
    python-httplib2 python-pyparsing python-six openh264 minizip
```

## Debian patch set provenance

The `ppc64le-patches` tarball was produced from:

- Source: `https://salsa.debian.org/chromium-team/chromium.git`
- Package version at time of sync: `150.0.7871.124-1`
- Content: `debian/patches/ppc64le/` verbatim, plus `debian/patches/series`
  copied to `ppc64le-patches/debian-series` and used as the ordering source.

The delta between Debian's 150.0.7871.124 and Arch's 150.0.7871.128 is
four security point releases. All 43 patches applied cleanly against the
Arch tarball with only fuzz/offset adjustments — no rejects.

## About the bundled gn build

`prepare()` runs:
```
python3 build/gen.py --no-last-commit-position
# writes a stub last_commit_position.h (see PKGBUILD comment)
sed -i 's/-Werror//g' out/build.ninja
ninja -C out gn
```

The `--no-last-commit-position` + stub combination avoids needing a full
git history for `git describe`. The `-Werror` strip covers GCC 16
warnings that gn upstream hasn't caught up to yet (multi-line comment
warning, false-positive libstdc++ array-bounds). Both are self-contained
workarounds and go away when gn upstream tightens up.

The result is a ~5 MB `gn` binary at `$srcdir/gn/out/gn`, used by
`build()` via `local _gn="$srcdir/gn/out/gn"`.

## Open items before this can be canonical upstream

1. **gn version** — the correct long-term fix is bumping Arch POWER's
   `gn` package to a version that recognizes chromium 150's dotfile /
   tool attributes. The in-build gn compile is a temporary shim so that
   this PKGBUILD works today. Once `gn` is refreshed, remove:
   - the `git+https://gn.googlesource.com/gn#commit=...` source entry
     and its `SKIP` sha256
   - the "Building gn..." block in `prepare()`
   - the `local _gn="$srcdir/gn/out/gn"` line in `build()` (revert to bare `gn`)

2. **`fetch-chromium-release` sha256** — the upstream Arch PKGBUILD
   reuses the tarball's sha256 for the script when `_manual_clone=1`.
   Since we set `_manual_clone=0`, this isn't exercised, but the mismatch
   remains in the file for round-trip with upstream Arch. Building
   without `--skipchecksums` on `_manual_clone=1` will fail on this
   checksum. Not a ppc64le regression — pre-existing upstream Arch state.

## Test results

- Built end-to-end on a POWER9 workstation (AC922 board, 128 threads,
  220 GB RAM + 128 GB swap). Build time approximately 4 h with `ninja`
  auto-parallelism.
- prepare() re-tested from scratch after the gn-build shim was added:
  fresh source extract, gn cloned + built from source (~1 min), all 43
  ppc64le patches + swiftshader fix applied cleanly, prepare() completed
  with `==> Sources are ready.`
- Produced `chromium-150.0.7871.128-1-powerpc64le.pkg.tar.zst` (173 MB)
  and `chromium-debug-...` (21 MB).
- Runs and browses the modern web with `--no-sandbox`. Sandbox is
  currently trapping in init on a custom 7.2-rc2 kernel; this appears
  to be a kernel-side missing feature detection (`CONFIG_SECURITY_YAMA`
  was disabled on the test kernel), not a chromium code issue. Sandbox
  debug is left as a follow-up.
