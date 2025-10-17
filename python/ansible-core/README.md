# ansible-core

## Maintaining ansible-core and ansible as one package group to avoid compatibility issues

To avoid eventual compatibility issues and as a matter of precaution, it is appropriate to maintain both `ansible-core` and `ansible` as one package group (e.g. package, release and move new versions of those packages accross repositories together).  
See the [ansible package README](https://gitlab.archlinux.org/archlinux/packaging/packages/ansible) for more details.

## Strict upper bound version requirements for build and runtime dependencies

Upstream generally applies very strict upper bounds version requirements for some build and runtime dependencies (most notably `setuptools`, `wheel` and `resolvelib`).

This is because they define their upper bounds version requirements to match the latest version of dependencies they tested at release (which is often lower than the ones we have in our repositories).  
As such, we simply bump / relax version requirements for said dependencies through patches.
