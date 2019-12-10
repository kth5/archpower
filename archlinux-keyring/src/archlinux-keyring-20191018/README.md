# Arch Linux Keyring

Repository for the Arch Linux keyring package.

## Addition/Removal/Update of a packaging key

1. Get the keyid from the bugreport in the [keyring
   project](https://bugs.archlinux.org/index.php?project=7&do=index&switch=1)
2. Add the keyid to `packager-keyids` in alphabetic order, following this
   format: full size keyid, a tab, nickname.

## Revoking a packager key

1. Create a key removal task in the [keyring
   project](https://bugs.archlinux.org/index.php?project=7&do=index&switch=1).
2. Remove the keyid of the revoked user from `packager-keyids`.
3. Add the removed keyid to `packager-revoked-keyids`, in alphabetic order,
   following this format: full size keyid, a tab, nickname, a tab and reason of
   revocation.

## Keyring release

1. bump the version in the Makefile
2. Run update-keys
4. git add the new .asc file in the packager directory.
4. Commit everything as 'Update keyring'
5. Create a new tag ```git tag -s $(date +"%Y%m%d")```
6. Push changes
7. Upload the source tarball with ```make dist upload```
8. Update the package
