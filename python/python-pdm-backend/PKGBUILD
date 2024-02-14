# Maintainer: David Runge <dvzrv@archlinux.org>

# set to 0 to use vendored sources
_devendored=1
_name=pdm-backend
pkgname=python-pdm-backend
pkgver=2.1.8
pkgrel=1
pkgdesc="The build backend used by PDM that supports latest packaging standards"
arch=(any)
url="https://github.com/pdm-project/pdm-backend"
license=(MIT)
depends=(
  python
)
if (( _devendored == 1 )); then
  # NOTE: devendored from sources
  depends+=(
    python-packaging
    python-pyproject-metadata
    python-tomli-w
  )
fi
makedepends=(
  python-build
  python-installer
  python-wheel
)
checkdepends=(
  git
  python-editables
  python-pytest
  python-pytest-cov
  python-pytest-xdist
  python-setuptools
)
optdepends=(
  'python-setuptools: for setuptools support'
  'python-editables: for editables backend support'
)
source=(
  $_name-$pkgver.tar.gz::$url/archive/refs/tags/$pkgver.tar.gz
  $pkgname-2.0.7-devendor.patch
)
sha256sums=('bf512fad97a1bbbe4408934d4a4a67a6a55d65a108f7fc42f731f3f7eda2f773'
            'de1e95cdfab8dd24ed7c20ba9e7ed9647421645eb11bff75a185c77dfef75190')
b2sums=('5baf6db7193f27de5aa89e41852c714de7d2aa7fd9f75de9b1f1ebd455052bc9d9d47d95dbbed3436e9e9dae79c9a1d0ec54d71a4437036869de38aa035f0642'
        'fb045b6f061fed51046cda246ed6603969627c0d13a0c6a5dbc0a8b2d4c07d54ec3fd64b7c8ff9c97373dbda6b20a25172036ba248ac3ae0b95fc5fcae036486')

prepare() {
  if (( _devendored == 1 )); then
    patch -Np1 -d $_name-$pkgver -i ../$pkgname-2.0.7-devendor.patch
    rm -frv $_name-$pkgver/src/pdm/backend/_vendor
  fi
}

build() {
  cd $_name-$pkgver
  PDM_BUILD_SCM_VERSION="$pkgver" python -m build --wheel --skip-dependency-check --no-isolation
}

check() {
  local pytest_options=(
    -vv
    # https://github.com/pdm-project/pdm-backend/issues/164
    --deselect tests/test_api.py::test_build_with_cextension
    --deselect tests/test_api.py::test_build_with_cextension_in_src
  )
  local site_packages=$(python -c "import site; print(site.getsitepackages()[0])")

  cd $_name-$pkgver
  # install to temporary location, as importlib is used
  python -m installer --destdir=test_dir dist/*.whl
  export PYTHONPATH="test_dir/$site_packages:$PYTHONPATH"

  # set default git config for test
  git config --global user.email "you@example.com"
  git config --global user.name "Your Name"
  pytest "${pytest_options[@]}"
}

package() {
  cd $_name-$pkgver
  python -m installer --destdir="$pkgdir" dist/*.whl
  install -vDm 644 LICENSE -t "$pkgdir/usr/share/licenses/$pkgname/"
  install -vDm 644 README.md -t "$pkgdir/usr/share/doc/$pkgname/"
}
