# Maintainer: Levente Polyak <anthraxx[at]archlinux[dot]org>
# Maintainer: Daniel M. Capella <polyzen@archlinux.org>
# Contributor: Felix Yan <felixonmars@archlinux.org>
# Contributor: Massimiliano Torromeo <massimiliano.torromeo@gmail.com>

pkgname=python-requests
_name=${pkgname#python-}
pkgver=2.32.5
pkgrel=3
pkgdesc='Python HTTP for Humans'
arch=(any)
url=https://requests.readthedocs.io/
license=(Apache-2.0)
depends=(
  ca-certificates
  python-charset-normalizer
  python-idna
  python-urllib3
)
makedepends=(
  git
  python-build
  python-installer
  python-setuptools
  python-wheel
)
checkdepends=(
  python-pysocks
  python-pytest-httpbin
  python-trustme
)
optdepends=(
  'python-chardet: alternative character encoding library'
  'python-pysocks: SOCKS proxy support'
)
source=(
  "git+https://github.com/psf/$_name.git#tag=v$pkgver"
  certs.patch
)
b2sums=('ae8446a060643d1f877339e1b4dee103d4999fb6e1cc55d149c0a99158affc7938f0f69177c4fb647a621460d441fdedff7fabc17004ba812baf744f596e69ee'
        '30fc6f283f2416318a1011bffab1ee23b0551188704eeacac77b28f5709f42fc33755a14a2eeb3ba2dccb2904a97a87021cff1423fe9149c78f2b9560998308d')
validpgpkeys=(87227E29AD9CFF5CFAC3EA6A44D3FF97B80DC864) # Nathanael Prewitt <nate.prewitt@gmail.com>

prepare() {
  cd "$_name"
  sed -i '/certifi/d' setup.py
  patch -p1 -i ../certs.patch
}

build() {
  cd "$_name"
  python -m build --wheel --no-isolation
}

check() {
  cd "$_name"
  # test_unicode_header_name hangs
  PYTHONPATH="$PWD/src" pytest -v tests --deselect tests/test_requests.py::TestRequests::test_unicode_header_name
}

package() {
  cd "$_name"
  python -m installer --destdir="$pkgdir" dist/*.whl
}

# vim: ts=2 sw=2 et:
