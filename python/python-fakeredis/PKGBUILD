# Maintainer: Caleb Maclennan <caleb@alerque.com>
# Contributor: Eli Schwartz <eschwartz@archlinux.org>

_pyname=fakeredis
pkgname=python-fakeredis
pkgver=2.32.0
pkgrel=1
pkgdesc='Fake implementation of redis API (redis-py) for testing purposes'
arch=(any)
url="https://github.com/dsoftwareinc/$_pyname-py"
license=(BSD-3-Clause)
depends=(python-redis
         python-sortedcontainers)
makedepends=(python-{build,installer,wheel}
             python-hatchling
             python-lupa
             python-packaging)
checkdepends=(python-hypothesis
              python-pytest
              python-pytest-asyncio
              python-pytest-mock)
optdepends=('python-packaging: for aioredis support'
            'python-lupa: for lua scripting support')
# _archive="$_pyname-py-$pkgver"
# source=("$url/archive/v$pkgver/$_archive.tar.gz")
_archive="$_pyname-$pkgver"
source=("https://files.pythonhosted.org/packages/source/${_pyname::1}/$_pyname/$_archive.tar.gz")
sha256sums=('63d745b40eb6c8be4899cf2a53187c097ccca3afbca04fdbc5edc8b936cd1d59')

build(){
	cd "$_archive"
	python -m build -wn
}

check() {
	cd "$_archive"
	export PYTHONPATH="$PWD"
	# Upstream tests are pretty borked, especially the PyPi sources. Skip for now.
	# pytest
}

package() {
	cd "$_archive"
	python -m installer -d "$pkgdir" dist/*.whl
	install -Dm0644 -t "$pkgdir/usr/share/licenses/$pkgname/" LICENSE
}
