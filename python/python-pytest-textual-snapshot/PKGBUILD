# Maintainer: Leonidas Spyropoulos <artafinde@archlinux.org>

_base=pytest-textual-snapshot
pkgname=python-${_base}
pkgver=1.1.0
pkgrel=1
pkgdesc="A pytest plugin for snapshot testing Textual applications"
arch=(any)
url="https://github.com/Textualize/${_base}"
license=(MIT)
depends=(
  python
  python-jinja
  python-pytest
  python-rich
  python-syrupy
  python-textual
)
makedepends=(
  git
  python-poetry-core
  python-build
  python-installer
)
source=("git+$url.git#tag=v${pkgver}")
sha512sums=('0612f85fb5b729581e7ffd388285b5e1bb895a0a7af4548bccf69e662f381e2815fd2ad9ae1e9500fb5308b02f14c01dfb34d7b108cca79515cca43c5e2492a1')

build() {
  cd ${_base}
  python -m build --wheel --no-isolation
}

package() {
  cd ${_base}
  install -D -m644 LICENSE -t "${pkgdir}/usr/share/licenses/${pkgname}"
  python -m installer --destdir="${pkgdir}" dist/*.whl
}

# vim: ts=2 sw=2 et:
