# Maintainer: Caleb Maclennan <caleb@alerque.com>
# Contributor: JakobDev<jakobdev at gmx dot de>

pkgname=python-sphinx-reredirects
_pkgname=${pkgname#python-}
pkgver=1.1.0
pkgrel=1
pkgdesc='Handles redirects for moved pages in Sphinx documentation projects'
arch=(any)
url="https://github.com/documatt/$_pkgname"
license=("BSD")
depends=(python
         python-sphinx)
makedepends=(python-{build,installer}
             python-flit-core
             python-wheel)
checkdepends=(python-defusedxml
              python-pytest)
_archive="$_pkgname-$pkgver"
source=("$url/archive/v$pkgver/$_archive.tar.gz")
sha256sums=('e62bb51f804b82ac0383bb4dd840a53fd0ad3f992b4325f9773164ae04622a7f')

prepare() {
	cd "$_archive"
	sed -i -e '/flit_core/s/,<4//' pyproject.toml
}

build() {
	cd "$_archive"
	python -m build -wn
}

check() {
	cd "$_archive"
	pytest -k 'not test_linkcheck'
}

package() {
	cd "$_archive"
	python -m installer -d "$pkgdir" dist/*.whl
}
