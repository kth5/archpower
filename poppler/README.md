# poppler

## Soname bump on new upstream releases

New upstream releases include a soname bump in `libpoppler.so` (and sometimes in `libpoppler-cpp.so` as well).  
You can run `sogrep` on the built `poppler` package, targeting `libpoppler.so` (same should be done targetting `libpoppler-cpp.so` if it had a soname bump too), to identify the list of packages to rebuilb against it (e.g. `for repo in core extra; do for lib in $(find-libprovides poppler-24.12.0-1-x86_64.pkg.tar.zst | sed 's/=.*//g'); do sogrep -r $repo libpoppler.so; done; done | sort | uniq`).

Creating ToDos to track those rebuilds (in `staging`) is encouraged. For instance: <https://archlinux.org/todo/poppler-24120/>
