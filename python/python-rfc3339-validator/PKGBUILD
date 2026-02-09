# Maintainer: Felix Yan <felixonmars@archlinux.org>

pkgname=python-rfc3339-validator
pkgver=0.1.4
pkgrel=8
pkgdesc="A pure python RFC3339 validator"
url="https://github.com/naimetti/rfc3339-validator"
license=('MIT')
arch=('any')
depends=('python-six')
makedepends=('python-setuptools' 'python-build' 'python-installer' 'python-wheel')
checkdepends=('python-hypothesis' 'python-pytest' 'python-strict-rfc3339')
source=("https://github.com/naimetti/rfc3339-validator/archive/v$pkgver/$pkgname-$pkgver.tar.gz")
sha512sums=('ed593b31c4984cdbc313e42dc0432173eaa649712e29627ab462dfa262efb30cee97b74f589d3d211c2193623fac3baca560a52a7a3f718071a63482e06c4203')

build() {
  cd rfc3339-validator-$pkgver
  python -m build --wheel --no-isolation
}

check() {
  cd rfc3339-validator-$pkgver
  pytest
}

package() {
  cd rfc3339-validator-$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl

  install -Dm644 LICENSE -t "$pkgdir"/usr/share/licenses/$pkgname/
}
