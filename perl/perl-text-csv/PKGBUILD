# Maintainer: Jan Alexander Steffens (heftig) <heftig@archlinux.org>

pkgname=perl-text-csv
_dist=Text-CSV
pkgver=2.06
pkgrel=2
pkgdesc="Comma-separated values manipulator"
url="https://metacpan.org/dist/$_dist"
arch=(any)
license=('Artistic-1.0-Perl OR GPL-1.0-or-later')
depends=(perl)
checkdepends=(perl-test-pod)
options=('!emptydirs')
source=("https://cpan.metacpan.org/authors/id/I/IS/ISHIGAKI/$_dist-$pkgver.tar.gz")
b2sums=('f7b319e489d863c4526cf270d132db25b6f8d4ddb59d33df59106f3f72a53a967a4073ec1c1924a5637d84ee16a3eedb985ddf6c97dd2334e35f0736d19f9dcb')

build() {
  cd $_dist-$pkgver
  perl Makefile.PL INSTALLDIRS=vendor
  make
}

check() {
  cd $_dist-$pkgver
  make test
}

package() {
  cd $_dist-$pkgver
  make DESTDIR="$pkgdir" install
}

# vim:set sw=2 sts=-1 et:
