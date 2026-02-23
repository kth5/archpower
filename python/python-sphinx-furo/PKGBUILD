# Maintainer: Filipe Laíns (FFY00) <lains@archlinux.org>
# Maintainer: Daniel M. Capella <polyzen@archlinux.org>

_pkgname=furo
pkgname=python-sphinx-$_pkgname
pkgver=2025.09.25
pkgrel=2
pkgdesc='Clean customizable documentation theme for Sphinx'
arch=(any)
url=https://github.com/pradyunsg/furo
license=(MIT)
depends=(
  python-accessible-pygments
  python-beautifulsoup4
  python-pygments
  python-sphinx-basic-ng
)
makedepends=(
  expac
  git
  nodejs-lts-iron
  npm
  python-build
  python-flit-core
  python-installer
  python-sphinx-theme-builder
)
source=("git+$url.git#tag=$pkgver?signed")
b2sums=('5c9e0b76751245434ff6f730b247b88760f8f95cdf5e17c54ea419ad44c727d0837c6705653e77b1d10765f2835a6e62257add777fb0b1389173a9da878e4f3f')
validpgpkeys=(E2FEFA4CA4BDD28171DB00F3FF99710C4332258E) # Pradyun Gedam (GitHub July 2020 Key) <pradyunsg@users.noreply.github.com>

prepare() {
  cd $_pkgname

  # force use of system nodejs
  local node_ver=$(expac %v nodejs-lts-iron | cut -d - -f 1)
  sed -i "s/18.20.5/$node_ver/" pyproject.toml
}

build() {
  cd $_pkgname

  STB_USE_SYSTEM_NODE=true python -m build -nw

  # docs disabled for now to unblock Python 3.10 update

  # sphinx needs this theme installed because it is uses it.
  # simply injecting the package to sys.path (via PYTHONPATH or similar)
  # is not enough because sphinx looks for themes in the registered
  # entrypoints, this means we actually have to install the package.
  # for this we will create a venv with access to system packages and
  # install the theme there, then we will build the documentation.

  #python -m venv --system-site-packages doc-env
  #doc-env/bin/python setup.py install --skip-build

  #doc-env/bin/python /usr/bin/sphinx-build -b dirhtml -v docs build/docs/html
}

package() {
  cd $_pkgname

  python -m installer --destdir="$pkgdir" dist/*.whl
  install -Dt "$pkgdir"/usr/share/licenses/$pkgname LICENSE

  #install -dm 755 "$pkgdir"/usr/share/doc/$pkgname
  #cp -r -a --no-preserve=ownership build/docs/html "$pkgdir"/usr/share/doc/$pkgname
  #rm -rf "$pkgdir"/usr/share/doc/$pkgname/html/.doctrees
}
