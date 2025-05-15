# Maintainer: Maxime Gauduin <alucryd@archlinux.org>
# Contributor: Tatsuyuki Ishi <ishitatsuyuki@gmail.com>

pkgname=python-arrow
_name="${pkgname#python-}"
pkgver=1.3.0
pkgrel=5
pkgdesc='Better dates and times for Python'
arch=(any)
url=https://arrow.readthedocs.io
_url=https://github.com/arrow-py/arrow
license=(Apache-2.0)
depends=(
  python
  python-dateutil
  python-types-python-dateutil
)
makedepends=(
  git
  python-build
  python-flit-core
  python-installer
)
checkdepends=(
  python-pytest
  python-pytest-mock
  python-pytz
  python-simplejson
)
source=($pkgname-$pkgver.tar.gz::$_url/archive/refs/tags/$pkgver.tar.gz)
sha256sums=('108c9d0339dbb06f6a255d1e8399a9bd88ec53ae6ede044b4ca7b3c563184a1b')

build() {
  cd $_name-$pkgver
  python -m build --wheel --no-isolation
}

check() {
  cd $_name-$pkgver
  pytest -vv -o addopts=''
}

package() {
  python -m installer --destdir="${pkgdir}" $_name-$pkgver/dist/*.whl
}

# vim: ts=2 sw=2 et:
