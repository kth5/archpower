# Maintainer: Felix Yan <felixonmars@archlinux.org>

pkgname=python-ini2toml
pkgver=0.15
pkgrel=2
pkgdesc="Automatically conversion of .ini/.cfg files to TOML equivalents"
url="https://github.com/abravalheri/ini2toml"
license=('MPL')
arch=('any')
# The default installation is broken. Adding [full] flavor dependencies here.
depends=('python-packaging' 'python-setuptools' 'python-configupdater' 'python-tomlkit')
makedepends=('git' 'python-setuptools-scm' 'python-build' 'python-installer' 'python-wheel')
checkdepends=('python-pytest-randomly' 'python-tomli-w' 'python-validate-pyproject' 'python-tomli')
source=("git+https://github.com/abravalheri/ini2toml.git#tag=v$pkgver")
sha512sums=('982cd47e78cadfd8ccf4d20af70d9e5828bc62e4d96d247d7713a1f6a700d4a94e9f7c28fb5ee53d32dd2e006b4e5606c5ce52019c0a66e854bb40686ae11b6c')

prepare() {
  cd ini2toml
  sed -i 's/--cov ini2toml --cov-report term-missing//' setup.cfg
}

build() {
  cd ini2toml
  python -m build --wheel --no-isolation
}

check() {
  cd ini2toml
  python -m venv --system-site-packages local-env
  local-env/bin/python -m installer dist/*.whl
  # TODO
  local-env/bin/python -m pytest --deselect tests/test_cli.py::test_auto_formatting
}

package() {
  cd ini2toml
  python -m installer --destdir="$pkgdir" dist/*.whl
}
