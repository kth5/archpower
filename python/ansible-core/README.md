# ansible-core

## Maintaining ansible-core and ansible as one package group to avoid compatibility issues

To avoid eventual compatibility issues and as a matter of precaution, it is appropriate to maintain both `ansible-core` and `ansible` as one package group (e.g. package, release and move new versions accross repositories at the same time).  
See the [ansible package README](https://gitlab.archlinux.org/archlinux/packaging/packages/ansible) for more details.
