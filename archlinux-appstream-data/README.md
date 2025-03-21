**GENERATING METADATA**

1) Customize the `asgen-config.json` configuration file (don't rename it!):
   - `ArchiveRoot` should point to a directory containing a checkout of the binary repos.
   - `Backend` should be `archlinux` for alpm based packages.
   - In `Suites`, `sections` is a list with the repositories that should be indexed.
   - See https://github.com/ximion/appstream-generator/blob/master/docs/asgen-config.md for other options.
2) Run `appstream-generator process arch` (Replace `arch` by the suite name configured in asgen-config.json)
3) Run the `upload` script to move the contents of export/data/arch to sources.archlinux.org for packaging.

**PACKAGING**

4) `Components-${arch}.xml.gz` files should be renamed to include the repo name and installed to `/usr/share/swcatalog/xml/`.

5) `Icons-${size}.tar.gz` should be decompressed and installed to `/usr/share/swcatalog/icons/`.
