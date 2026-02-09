# Maintainer: Felix Yan <felixonmars@archlinux.org>

pkgname=python-apispec
pkgver=6.8.0
pkgrel=3
pkgdesc="A pluggable API specification generator. Currently supports the OpenAPI Specification."
url="https://github.com/marshmallow-code/apispec"
license=('MIT')
arch=('any')
depends=(
  'python'
  'python-packaging'
)
optdepends=(
  'python-marshmallow: for marshmallow support'
  'python-yaml: for yaml support'
)
makedepends=('python-build' 'python-flit-core' 'python-installer' 'python-wheel')
checkdepends=('python-tornado' 'python-bottle' 'python-marshmallow'
              'python-flask' 'python-yaml'
              'python-openapi-spec-validator' 'python-pytest')
source=("$pkgname-$pkgver.tar.gz::https://github.com/marshmallow-code/apispec/archive/$pkgver.tar.gz")
sha512sums=('bec209878a88298ec35c6e68b10f02409ec3d8fdcbbe6d50f99e3e47ff79c2884d0aef1911e713bf7de9f59bad2fd2788698922566bab5d091dd1920548966e8')

build() {
  cd apispec-$pkgver
  python -m build --wheel --no-isolation
}

check() {
  cd apispec-$pkgver
  PYTHONPATH=src pytest -k 'not test_schema_instance_with_different_modifers_custom_resolver' .
}

package() {
  cd apispec-$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl
  install -Dm644 LICENSE "$pkgdir"/usr/share/licenses/$pkgname/LICENSE
}
