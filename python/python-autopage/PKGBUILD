# Maintainer:

pkgname=python-autopage
pkgver=0.5.2
pkgrel=4
pkgdesc='A library to provide automatic paging for console output'
arch=(any)
url='https://github.com/zaneb/autopage'
license=(Apache)
depends=(python)
checkdepends=(python-pytest python-fixtures python-testtools)
makedepends=(python-build python-installer python-setuptools python-wheel)
source=(https://pypi.python.org/packages/source/a/autopage/autopage-$pkgver.tar.gz)
sha512sums=('a5449fc2a1011a3936c69784803e9f65a603cfa6df2335b095b029051fc26742109141ef2eb201f567334c0617433f29477efc43d98357d7ee00f266c6546560')

build() {
  cd autopage-$pkgver
  python -m build --wheel --skip-dependency-check --no-isolation
}

check() {
  cd autopage-$pkgver
  unset LESS PAGER
  pytest -v -k 'not test_end_to_end.py'
}

package() {
  cd autopage-$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl
}
