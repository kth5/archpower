# Maintainer:
# Contributor: Felix Yan <felixonmars@archlinux.org>

_name=pytest-socket
pkgname=python-pytest-socket
pkgver=0.7.0
pkgrel=6
pkgdesc='Pytest Plugin to disable socket calls during tests'
arch=(any)
license=(MIT)
url="https://github.com/miketheman/pytest-socket"
depends=(
  python
  python-pytest
)
makedepends=(
  python-build
  python-installer
  python-poetry-core
  python-wheel
)
checkdepends=(
  python-httpx
  python-pytest-httpbin
  python-requests
  python-starlette
)
source=($url/archive/$pkgver/$pkgname-$pkgver.tar.gz)
sha512sums=('fe8888819fef0f192f88a7509fe153b992c12a1fd1d56ae69c844a592fc403e3b5b13c4c3c4be89c8bf114f18d84fb6d6e8a8e3604d7fe454d090272689136f1')
b2sums=('1609a75907f743e8cbd38e48c6df74bb9ce329cda68bde02fd2a02fa7105a688853723b22a57259c1050e33ed41b23ad34a9ca37365b71939b44834c5d02e6db')

build() {
  cd $_name-$pkgver
  python -m build --wheel --no-isolation
}

check() {
  local pytest_options=(
    -vv
    --deselect tests/test_socket.py::test_urllib_succeeds_by_default
    --deselect tests/test_socket.py::test_enabled_urllib_succeeds
  )
  local site_packages=$(python -c "import site; print(site.getsitepackages()[0])")

  cd $_name-$pkgver
  # install to temporary location, as importlib is used
  python -m installer --destdir=test_dir dist/*.whl
  export PYTHONPATH="test_dir/$site_packages:$PYTHONPATH"
  pytest "${pytest_options[@]}"
}

package() {
  cd $_name-$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl
  install -vDm644 LICENSE -t "$pkgdir"/usr/share/licenses/$pkgname/
}

# vim:set ts=2 sw=2 et:
