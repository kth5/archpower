# Maintainer: Jan Alexander Steffens (heftig) <heftig@archlinux.org>

pkgname=gi-docgen
pkgver=2024.1
pkgrel=1
pkgdesc="Documentation generator for GObject-based libraries"
url="https://gnome.pages.gitlab.gnome.org/gi-docgen/"
arch=(any)
license=("Apache-2.0 OR GPL-3.0-or-later")
depends=(
  python
  python-jinja
  python-magic
  python-markdown
  python-markupsafe
  python-packaging
  python-pygments
  python-typogrify
)
makedepends=(
  git
  python-build
  python-installer
  python-setuptools
  python-wheel
)
source=("git+https://gitlab.gnome.org/GNOME/gi-docgen.git#tag=$pkgver")
b2sums=('027fe6fb1bf4e64e75baa5406a6b39ff4be00c4f06937e521bfc0d7fda48da44f4a1151b3e245a0fb097b686b80d20618ccde139293fa07904c5494192f4df4a')
validpgpkeys=(
  53EF3DC3B63E2899271BD26322E8091EEA11BBB7 # Emmanuele Bassi (GNOME) <ebassi@gnome.org>
)

prepare() {
  cd $pkgname
}

build() {
  cd $pkgname
  python -m build --wheel --no-isolation
}

package() {
  cd $pkgname
  python -m installer --destdir="$pkgdir" dist/*.whl
}

# vim:set sw=2 sts=-1 et:
