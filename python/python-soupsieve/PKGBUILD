# Maintainer: George Rawlinson <grawlinson@archlinux.org>
# Contributor: Felix Yan <felixonmars@archlinux.org>

pkgname=python-soupsieve
pkgver=2.8
pkgrel=1
pkgdesc='A CSS4 selector implementation for Beautiful Soup'
arch=(any)
url='https://github.com/facelessuser/soupsieve'
license=(MIT)
depends=(python python-beautifulsoup4)
makedepends=(
  git
  python-build
  python-installer
  python-hatchling
)
checkdepends=(
  python-pytest
  python-html5lib
  python-lxml
)
source=("$pkgname::git+$url#tag=$pkgver")
sha512sums=('258ed00d4596127dee27029becc07ffd42f8e1e674e72e68bfa938eb7a07a6a41189a1ec7873e1120ef564b2aa15c7e4fe5ae9379258f939b0b028fa5d2380b9')
b2sums=('08bcfa1a9215ed7157552a11ae7600ed23743724689bf4d08ed80ecd67d44a83d59020349e34412d5026e42467d815435b519138f1d3c805a370062d349530af')

build() {
  cd "$pkgname"

  python -m build --wheel --no-isolation
}

check() {
  cd "$pkgname"

  # https://gitlab.gnome.org/GNOME/libxml2/-/issues/312
  pytest \
    --deselect tests/test_extra/test_soup_contains.py::TestSoupContains::test_contains_cdata_html \
    --deselect tests/test_extra/test_soup_contains_own.py::TestSoupContainsOwn::test_contains_own_cdata_html
}

package() {
  cd "$pkgname"

  python -m installer --destdir="$pkgdir" dist/*.whl

  # symlink license file
  local site_packages=$(python -c "import site; print(site.getsitepackages()[0])")
  install -d "$pkgdir/usr/share/licenses/$pkgname"
  ln -s "$site_packages/${pkgname#python-}-$pkgver.dist-info/licenses/LICENSE.md" \
    "$pkgdir/usr/share/licenses/$pkgname/LICENSE.md"
}
