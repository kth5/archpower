# Maintainer: Carl Smedstad <carsme@archlinux.org>
# Contributor: Felix Yan <felixonmars@archlinux.org>

pkgname=python-elasticsearch
_pkgname=elasticsearch-py
pkgver=8.16.0
pkgrel=1
arch=(any)
pkgdesc="Official Python client for Elasticsearch"
url="https://github.com/elastic/elasticsearch-py"
license=(Apache-2.0)
depends=(
  python
  python-elastic-transport
)
makedepends=(
  python-build
  python-hatchling
  python-installer
  python-wheel
)
checkdepends=(
  python-aiohttp
  python-dateutil
  python-numpy
  python-orjson
  python-pandas
  python-pyarrow
  python-pytest
  python-pytest-asyncio
  python-requests
  # Not available in Arch repos (yet)
  # python-simsimd
  python-yaml
)
optdepends=(
  'python-aiohttp: support for asynchronous requests'
  'python-numpy: support for Maximal Marginal Relevance (MMR) for search results'
  'python-orjson: support for faster JSON serialization'
  'python-pyarrow: support for Arrow deserialization'
  'python-requests: support for synchronous requests'
  # 'python-simsimd: support for Maximal Marginal Relevance (MMR) for search results'
)
source=("$pkgname-$pkgver.tar.gz::$url/archive/v$pkgver.tar.gz")
sha256sums=('ff11e054db75c399746d536cde360803880350eaa3c3ae08a04f1b96e50e13f5')

build() {
  cd $_pkgname-$pkgver
  python -m build --wheel --no-isolation
}

check() {
  cd $_pkgname-$pkgver
  pytest --override-ini="addopts="
}

package() {
  cd $_pkgname-$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl
}
