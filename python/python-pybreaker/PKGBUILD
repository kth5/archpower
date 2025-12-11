# Maintainer: George Rawlinson <grawlinson@archlinux.org>

pkgname=python-pybreaker
pkgver=1.4.1
pkgrel=1
pkgdesc='Python implementation of the Circuit Breaker pattern'
arch=(any)
url='https://github.com/danielfm/pybreaker'
license=(BSD-3-Clause)
depends=(python)
makedepends=(
  git
  python-build
  python-flit-core
  python-installer
)
checkdepends=(
  python-pytest
  python-tornado
  python-redis
  python-fakeredis
)
optdepends=(
  'python-tornado: tornado support'
  'python-redis: redis support'
)
source=("$pkgname::git+$url#tag=v$pkgver")
sha512sums=('a1bcb52acfdc66b3516b8964a8e2d49b637c55595405a6968eb34a3ea58d0c44871843bf55043cd78038f0c8f0087bbaa83080da59ef684c40669e1241cabb4d')
b2sums=('290bc07188dab837a272db6d38d9dc8ed556dc2254acd2301e75ef9006a94b04a53de5c3b02d247e6f0f7b2ffb3ef8f111defea495e4fa50276c072e567b2d3d')

build() {
  cd "$pkgname"

  python -m build --wheel --no-isolation
}

check() {
  cd "$pkgname"

  # temporary install
  python -m installer --destdir="$(pwd)/tmp" dist/*.whl
  local site_packages=$(python -c "import site; print(site.getsitepackages()[0])")
  export PYTHONPATH="$(pwd)/tmp/$site_packages"

  pytest -v
}

package() {
  cd "$pkgname"

  python -m installer --destdir="$pkgdir" dist/*.whl

  # license
  install -vDm644 -t "$pkgdir/usr/share/licenses/$pkgname" LICENSE
}
