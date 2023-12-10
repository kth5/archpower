# Maintainer: David Runge <dvzrv@archlinux.org>

_name=sphinx_lv2_theme
pkgname=python-sphinx-lv2-theme
pkgver=1.4.2
pkgrel=1
pkgdesc="A minimal static theme for Sphinx"
arch=(any)
url="https://gitlab.com/lv2/sphinx_lv2_theme"
license=(0BSD)
depends=(
  python
  python-sphinx
)
makedepends=(
  python-build
  python-installer
  python-setuptools
  python-wheel
)
source=($url/-/archive/v$pkgver/$_name-v$pkgver.tar.gz)
sha256sums=('6214f1e212d705ee913f55b4a9275077dd575d555eaffa43917b01b8c91d429c')
b2sums=('25779ef9f0118dd4610edb29eeb81d2555b1b0cf3cbbc81345ab1bf842ff8b2c8d1d8c65ffd314281fe40dc77c492c2f90d15cae4ca998054da8185f457cf81b')

build() {
  cd $_name-v$pkgver
  python -m build --wheel --no-isolation
}

package() {
  cd $_name-v$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl
  install -vDm 644 LICENSE -t "$pkgdir/usr/share/licenses/$pkgname/"
  install -vDm 644 {NEWS,README.md} -t "$pkgdir/usr/share/doc/$pkgname/"
}
