# Maintainer: Caleb Maclennan <caleb@alerque.com>
# Maintainer: Daniel M. Capella <polyzen@archlinux.org>
# Contributor: Angel Velasquez <angvp@archlinux.org>
# Contributor: Felix Yan <felixonmars@archlinux.org>
# Contributor: Letu Ren <fantasquex@gmail.com>

_pyname=astroid
pkgname=python-$_pyname
pkgver=3.3.11
pkgrel=1
pkgdesc='A common base representation of python source code'
arch=(any)
url="https://github.com/pylint-dev/$_pyname"
license=(LGPL-2.1-or-later)
depends=(python)
makedepends=(python-{build,installer,wheel}
             python-setuptools)
checkdepends=(mypy
              python-attrs
              python-dateutil
              python-pyqt6
              python-regex
              python-urllib3
              python-pip
              python-pytest)
replaces=(python-logilab-astng)
conflicts=(python-logilab-astng)
_archive="$_pyname-$pkgver"
source=("$url/archive/v$pkgver/$_archive.tar.gz")
sha256sums=('5bb714e739c650e7e31186e1765f7edd33171b2f423ddbddb43b1c37cb6cfbb5')

build() {
	cd "$_archive"
	python -m build -wn
}

check() {
	cd "$_archive"
	# Test failure due to pkg_resources deprecation
	pytest --deselect tests/test_manager.py::AstroidManagerTest::test_identify_old_namespace_package_protocol
}

package() {
	cd "$_archive"
	python -m installer -d "$pkgdir" dist/*.whl
}
