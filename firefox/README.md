# firefox — Arch POWER (ppc64le) packaging notes

Downstream of Arch's [firefox PKGBUILD](https://gitlab.archlinux.org/archlinux/packaging/packages/firefox) with the archpower ppc64le patch stack. Bumped for firefox **153.0.3** in sync with Arch's `153.0.3-2`.

## Changes vs. the 152.0.4-2 archpower baseline

### Patches added
- `0002-Bug-2057577-DOM-Media-Add-FFmpeg-63-support.-r-alwu-.patch` — from Arch upstream. Adds firefox 153's FFmpeg 63 header tree so system libavcodec.so.63 loads.

### Patches removed (obsolete for 153)
- `ffmpeg-cleanup.patch` — old T2 fix; the new Arch ffmpeg-63 patch supersedes it.
- `hotfix-tailcalls.patch` — T2 fix for i386/sparc/s390 `clang::musttail`. Firefox 153 rewrote `port_def.inc` to an aarch64/x86_64 **allowlist**, so ppc64le / i386 / sparc / s390 are already excluded upstream. Patch no longer needed.
- `hotfix-wgpu-atomicu64.patch` — T2 fix swapping `AtomicU64` → `AtomicUsize` for 32-bit ppc. ppc64le has native 64-bit atomics — not needed. Void and Chimera don't carry it.
- `0001-Add-VSX-instructions-for-SKIA.patch` — runlevel5/firefox-ppc64 VSX SIMD accel for skia. **Dropped for 153 pending rebase** — the skia `opts/` file layout changed enough to break the hunks. Performance-only, not correctness. Void ships firefox 153 on ppc64le without it and works.

### mozconfig changes (in the `powerpc64le` case block)
- `--with-system-nss` removed — firefox 153 requires nss ≥ 3.125; Arch POWER's `nss` (3.124) is short. Falls back to bundled nss (which is newer). **Re-enable when system nss ≥ 3.125 lands.**
- `--with-system-libevent` removed — firefox 153 uses a new `CHECK_EVENT_SIZEOF(TIME_T, time_t)` static assert that requires `EVENT__SIZEOF_TIME_T` from libevent 2.2.x. Arch POWER has libevent 2.1.13 which pre-dates that define. Falls back to bundled libevent. **Re-enable when system libevent ≥ 2.2 lands.**

## POWER8 safety

Firefox 153 does NOT hard-drop POWER8. Scanned the source:
- The only POWER9-tagged flag set is `PPC_VSX3_FLAGS = -mvsx -mcpu=power9` in `build/moz.configure/toolchain.configure`. **Zero consumers** in the source tree — pure infrastructure, no components opt into it.
- `mozglue/static/lz4/xxhash.h` uses `__POWER9_VECTOR__` as a compile-time gate; on a POWER8 build with `-mcpu=power8` it falls through to a POWER7/8-safe path. No runtime traps.

archpower buildbot with POWER8 CFLAGS produces a POWER8-safe firefox binary.

## Rustup / rustc

Firefox's mach shells out `rustup which rustc` during configure. On a clean archpower buildbot (no rustup installed) mach falls back to the system rustc on PATH — no intervention needed. If a developer has a rustup shim on PATH that doesn't handle `which` correctly, they can bypass with `env RUSTC=/usr/bin/rustc CARGO=/usr/bin/cargo makepkg -e`.

## Build

```
makepkg --noconfirm
```

## Runtime note (preserved from prior README)

If you don't have a choice and need to run outside a chroot or container, remember to uninstall Firefox prior to its build. It'll otherwise sometimes pull in old SO links from its own installation (libffi).
