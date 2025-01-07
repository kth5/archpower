# ansible-core

## Maintaining ansible-core and ansible as one package group to avoid compatibility issues

To avoid eventual compatibility issues and as a matter of precaution, it is appropriate to maintain both `ansible-core` and `ansible` as one package group (e.g. package, release and move new versions of those packages accross repositories together).  
See the [ansible package README](https://gitlab.archlinux.org/archlinux/packaging/packages/ansible) for more details.

## Specific / strict upper version bound for (python-)resolvelib

Upstream seems [very cautious](https://github.com/ansible/ansible/blob/devel/requirements.txt#L10) with `resolvelib` bumps and use very specific / strict version bounds for it in their dependencies definitions (used by `ansible-galaxy`).

As such, it may happen that the `build` and / or the `check` processes require an outdated version of [python-resolvelib](https://archlinux.org/packages/extra/any/python-resolvelib/) compared to the one available in our repository.
In such cases, patching sources to [increase the upper version bound for `resolvelib`](https://gitlab.archlinux.org/archlinux/packaging/packages/ansible-core/-/commit/55405bd0324c46b5e63a6965cac151149829a9d1) is *most likely* enough to build the package succesfully (while waiting for upstream to do the change on their side).  
Despite upstream carefulness on that matter (apparently for past / historic reasons), we should not expect any breaking changes in **minor** `resolvelib` bumps.
