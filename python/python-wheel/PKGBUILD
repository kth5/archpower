# Maintainer: Felix Yan <felixonmars@archlinux.org>
# Maintainer: Morten Linderud <foxboron@archlinux.org>
# Contributor: Lance Chen <cyen0312@gmail.com>

_bootstrap=0
_bootstrap_version=3.14.0
pkgname=python-wheel
_name=${pkgname#python-}
pkgver=0.48.0
pkgrel=1
pkgdesc="A built-package format for Python"
arch=(any)
url="https://pypi.python.org/pypi/wheel"
license=('MIT')
depends=('python')
if (( _bootstrap == 0 )); then
  depends+=('python-packaging')
fi
optdepends=('python-keyring: for wheel.signatures'
            'python-xdg: for wheel.signatures'
            'python-setuptools: for legacy bdist_wheel subcommand')
if (( _bootstrap == 0 )); then
  makedepends=('python-build' 'python-flit-core' 'python-installer')
else
  makedepends=('git')
fi
checkdepends=('python-jsonschema' 'python-pytest' 'python-keyring' 'python-keyrings-alt'
              'python-xdg' 'python-setuptools')
if (( _bootstrap == 0 )); then
  source=(
    "$pkgname-$pkgver.tar.gz::https://github.com/pypa/wheel/archive/$pkgver.tar.gz"
  )
else
  source=(
    python-bootstrap::git+https://gitlab.archlinux.org/archlinux/python-bootstrap.git#tag=$_bootstrap_version
    python-build::git+https://github.com/pypa/build.git
    python-flit::git+https://github.com/pypa/flit.git
    python-installer::git+https://github.com/pypa/installer.git
    python-wheel::git+https://github.com/pypa/wheel.git
    python-packaging::git+https://github.com/pypa/packaging
    python-pyproject-hooks::git+https://github.com/pypa/pyproject-hooks
    python-setuptools::git+https://github.com/pypa/setuptools.git
  )
fi
sha512sums=('2eb1b9d8f24b9f7221d24a49b54352f001df778172828d7d58626fd5b5463109cef7fbbdc48548c128b8cdf99a4b76a833f4092d758239375ede7285dfa7c701')

prepare() {
  if (( _bootstrap == 0 )); then
    cd wheel-$pkgver
  else
    cd python-bootstrap
    git submodule init

    git config submodule."external/build".url ../python-build
    git config submodule."external/flit".url ../python-flit
    git config submodule."external/installer".url ../python-installer
    git config submodule."external/wheel".url ../python-wheel
    git config submodule."external/packaging".url ../python-packaging
    git config submodule."external/pyproject-hooks".url ../python-pyproject-hooks
    git config submodule."external/setuptools".url ../python-setuptools

    git -c protocol.file.allow=always submodule update
    git submodule update --init --recursive
  fi
}

build() {
  if (( _bootstrap == 0 )); then
    cd wheel-$pkgver
    python -m build --wheel --no-isolation
  else
    cd python-bootstrap
    python -m bootstrap.build
  fi
}

check() {
  # Hack entry points by installing it

  cd wheel-$pkgver
  python -m installer --destdir="$PWD/tmp_install" dist/*.whl
  local site_packages=$(python -c "import site; print(site.getsitepackages()[0])")
  PYTHONPATH="$PWD/tmp_install/$site_packages" pytest
}

package() {
  if (( _bootstrap == 0 )); then
    cd wheel-$pkgver
    python -m installer --destdir="$pkgdir" dist/*.whl
    install -Dm644 LICENSE.txt -t "$pkgdir"/usr/share/licenses/$pkgname/
  else
    cd python-bootstrap
    python -m bootstrap.install dist/$_name-*-py3-none-any.whl -d "$pkgdir"
    install -Dm644 external/$_name/LICENSE.txt -t "$pkgdir"/usr/share/licenses/$pkgname/
  fi
}
