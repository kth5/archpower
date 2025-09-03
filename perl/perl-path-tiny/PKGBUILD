# Maintainer: George Rawlinson <grawlinson@archlinux.org>
# Contributor: Florian Pritz <bluewind@xinu.at>

pkgname=perl-path-tiny
pkgver=0.150
pkgrel=1
pkgdesc='File path utility'
arch=(any)
url='https://github.com/dagolden/Path-Tiny'
license=(Apache-2.0)
depends=(perl)
makedepends=(git)
source=("$pkgname::git+$url#tag=release-$pkgver")
sha512sums=('82b0e0266547f4ec2eeeb0648ebb6f9371dc2540b662511cadd70e66fc2285b9736acbe106df4c45ff4df8b15d5d1ff41f690e6e9e80d8e0a1603eb9d99928b7')
b2sums=('0f2febb51efea18e7c624cda04f45a2687e4a0caa1d7aa914599518603b2144f597adf7bf6a34b9d2cefa5322f160551d5366036790454d16f0028dcd025bd84')

build() {
  cd "$pkgname"

  export PERL_MM_USE_DEFAULT=1 PERL_AUTOINSTALL=--skipdeps
  unset PERL5LIB PERL_MM_OPT
  perl Makefile.PL
  make
}

check() {
  cd "$pkgname"

  export PERL_MM_USE_DEFAULT=1
  unset PERL5LIB
  make test
}

package() {
  cd "$pkgname"

  make install INSTALLDIRS=vendor DESTDIR="$pkgdir"
}
# vim:set ts=2 sw=2 et:
