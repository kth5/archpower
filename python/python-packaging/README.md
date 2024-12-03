# python-packaging

## Bootstrapping with new Python interpreter version

When a new Python interpreter version is released and the updated [python] package is moved to the staging environment, this package needs to be bootstrapped against the new interpreter version.

To do this, the following actions need to be taken:

- align the `_bootstrap_version` with the latest [release of python-bootstrap] for the Python interpreter version to bootstrap for
- set `_bootstrap` to `1`
- build the package against the new Python interpreter using `pkgctl build --rebuild --nocheck --update-pkgsums --staging`
- push the resulting package to the staging environment using `pkgctl release --db-update --staging`
- when rebuilding the package again for devendoring, set `_bootstrap` to `0`
- build the package against the new Python interpreter using `pkgctl build --rebuild --nocheck --update-pkgsums --staging`
- push the resulting package to the staging environment using `pkgctl release --db-update --staging`

[python]: https://gitlab.archlinux.org/archlinux/packaging/packages/python/
[release of python-bootstrap]: https://gitlab.archlinux.org/archlinux/python-bootstrap/#releases
