# chromium — Arch POWER (ppc64le) packaging notes

Downstream fork of Arch's [chromium PKGBUILD](https://gitlab.archlinux.org/archlinux/packaging/packages/chromium) for ppc64le. In sync with Arch's `151.0.7922.108-1`.

## ppc64le-specific patches added

Three items on top of Arch upstream, guarded by `[[ $CARCH == powerpc64le ]]`:

1. **`chromium-ppc64le-patches-r2.tar.gz`** — 41 patches mirrored verbatim from Debian's official chromium package at [salsa.debian.org/chromium-team/chromium](https://salsa.debian.org/chromium-team/chromium) (`debian/patches/ppc64le/`, ordered per `debian/patches/series`). Extracts to `$srcdir/ppc64le-patches/`; PKGBUILD reads `debian-series` and applies only the `ppc64le/` entries.

2. **`swiftshader-ppc-xcoff-baseclasses.patch`** — swiftshader's bundled LLVM 10.0 fails to link its PPC XCOFF vtables without the base-class `.cpp` files.

3. **`chromium-151-dawn-cipd-add-ppc64le.patch`** — teaches dawn's `cipd_deps.py` about ppc64le so tint's `generate_sources` action finds the go symlink at build time. Paired with a `linux-ppc64le` go symlink dir in `prepare()`.

## Other divergences from Arch upstream

- `arch=(x86_64 powerpc64le)` — adds ppc64le
- Local gn build in `prepare()`, used in `build()`. Arch POWER's packaged `gn` (0.2324) lacks dotfile/tool attributes chromium 151 uses. Drop once the `gn` package is refreshed.
- Launcher `LAUNCHER_VERSION` pinned via `sed` in `build()` — upstream Makefile falls through to `git describe`, which walks up into any parent git repo.

## Refreshing the ppc64le patch set for a new Chromium release

```
curl -sL "https://salsa.debian.org/chromium-team/chromium/-/raw/master/debian/patches/series" \
  > ppc64le-patches/debian-series
# fetch each ppc64le/* patch listed in the series (raw URL:
#   https://salsa.debian.org/chromium-team/chromium/-/raw/master/debian/patches/<path>)
tar czf chromium-ppc64le-patches-r$NEW.tar.gz ppc64le-patches/
```

The tarball is a convenience — if you'd rather ship the patches as individual `source=` entries, un-tar the file, add each patch to `source=` with its sha256, and adjust the `while read` loop in `prepare()` accordingly. Functionally equivalent.
