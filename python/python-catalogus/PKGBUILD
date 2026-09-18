# Maintainer: Carl Smedstad <carsme@archlinux.org>

pkgname=python-catalogus
pkgver=0.1.2
pkgrel=1
pkgdesc="Classes to provide name-to-object registry-like support"
arch=(any)
url="https://github.com/breezy-team/catalogus"
license=(GPL-2.0-or-later)
depends=(python)
makedepends=(
  python-build
  python-installer
  python-setuptools
  python-setuptools-scm
  python-wheel
)
source=("$url/archive/v$pkgver/$pkgname-$pkgver.tar.gz")
b2sums=('38cce467cd1110a5dd3c5c66a1366eb898fc6a6abeac30dce5fbcb60abcb4fd6e6911afd43c35cbd1b4efdb52244d30b4ed9b2649860c2fb2168a1562db0a6fe')

build() {
  cd ${pkgname#python-}-$pkgver
  python -m build --wheel --no-isolation
}

check() {
  cd ${pkgname#python-}-$pkgver
  python -m unittest discover -v -s tests
}

package() {
  cd ${pkgname#python-}-$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl
}
