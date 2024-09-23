# libassuan

## Bootstrapping

When libassuan introduces a soname change, the package and all of its direct dependents need to be boostrapped into the staging build environment.
This is due to the fact, that pacman links against `libbassuan.so` via `libgpgme.so` and simply placing an updated libassuan in staging would therefore break pacman.

Only after bootstrapping, the normal way of rebuilding of dependents against the staging repository can commence.

### Update libassuan

Build the new version of libassuan against staging:

```bash
pkgctl build -s
```

### Placeholder package

For bootstrapping purposes it is advised to create a temporary placeholder package for the previous version of libassuan (e.g. `libassuan2`), which only provides the required library and the first symlink (e.g. `/usr/lib/libassuan.so.0.8.7` and `/usr/lib/libassuan.so.0`).
This placeholder package is only needed for bootstrapping and is therefore only temporarily added to the repositories!

Build the package normally against staging:

```bash
pkgctl build -s
```

### Bootstrap direct dependents

Now, all direct dependents (i.e. `gnupg`, `gpgme` and `pinentry`) need to be boostrapped using the updated libassuan and the placeholder package.
The below code blocks use libassuan 3.0.0 and libassuan2 2.5.7 as example, assuming all package source directories are located in the same directory:

```bash
pkgctl build -s --rebuild --install-to-chroot ../libassuan/libassuan-3.0.0-1-x86_64.pkg.tar.zst --install-to-chroot ../libassuan2/libassuan2-2.5.7-1-x86_64.pkg.tar.zst
```

### Releasing bootstrapped packages

Release all direct dependents and the new version of libassuan (i.e. *not* `libassuan2`!) to the staging repository at the same time.
In the libassuan source repository run:

```bash
pkgctl release -s
```

In the source repository of the placeholder package run:

```bash
pkgctl release --repo core-staging -m "Add libassuan2 as bootstrap package for libassuan 3.0.0"
```

In the source repository of each direct dependent run:

```bash
pkgctl release -s -m "Rebuild against libassuan 3.0.0, bootstrap with local libassuan2."
```

Afterwards update the staging repository, so that the new packages are added in a single transaction:

```bash
pkgctl db update
```

### Rebuild bootstrapped packages

To ensure that packages remain reproducible, rebuild all direct dependents in staging.
In the source repository of each direct dependent run:

```bash
pkgctl build -s -r --rebuild -u -m "Rebuild for reproducibility, after bootstrap"
```

### Remove placeholder package

Finally, the placeholder package (`libassuan2`) should be removed from the staging repository:

```bash
pkgctl db remove core-staging libassuan2
```
