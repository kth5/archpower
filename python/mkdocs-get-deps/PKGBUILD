# Maintainer: Maxime Gauduin <alucryd@archlinux.org>

pkgname=mkdocs-get-deps
pkgver=0.2.0
pkgrel=1
pkgdesc='An extra command for MkDocs that infers required PyPI packages from `plugins` in mkdocs.yml'
url=https://www.mkdocs.org
license=(MIT)
arch=(any)
depends=(
  python
  python-mergedeep
  python-platformdirs
  python-yaml
)
makedepends=(
  git
  python-build
  python-hatchling
  python-installer
  python-setuptools
  python-wheel
)
_tag=3d0dd8e3b51197c3f871559c342164c83aa4081a
source=(git+https://github.com/mkdocs/get-deps.git#tag=${_tag})
b2sums=('275913989b2187dd436a88ead035c597c32b5b177b7f9610c526ae5f1f2005891cb0986c601b2741f236c09fe80b80d814d5ca4911f04a2b628e5b0007d34550')

pkgver() {
  cd get-deps
  git describe --tags | sed 's/^v//'
}

build() {
  cd get-deps
  python -m build --wheel --no-isolation
}

package() {
  python -m installer --destdir="${pkgdir}" get-deps/dist/*.whl
  install -Dm 644 get-deps/LICENSE.md -t "${pkgdir}"/usr/share/licenses/mkdocs-get-deps/
}

# vim: ts=2 sw=2 et:
