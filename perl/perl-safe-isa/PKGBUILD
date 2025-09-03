# Maintainer:

pkgname=perl-safe-isa
pkgver=1.000010
pkgrel=2
pkgdesc='Call isa, can, does and DOES safely on things that may not be objects'
arch=(any)
license=(PerlArtistic)
url='https://metacpan.org/dist/Safe-Isa'
depends=(perl)
options=('!emptydirs')
source=("https://cpan.metacpan.org/authors/id/E/ET/ETHER/Safe-Isa-${pkgver}.tar.gz")
sha512sums=('121288c7c59d97f4e48c1e50795d835cac0638a1edb1116876813cc2fe955efced9916222f6b16e4c1dbd5149c9d68c19bc77584f999c411e4c22e2f28ea1838')

build() {
  cd Safe-Isa-$pkgver
  perl Makefile.PL INSTALLDIRS=vendor
  make
}

check() {
  cd Safe-Isa-$pkgver
  make test
}

package() {
  cd Safe-Isa-$pkgver
  make DESTDIR="$pkgdir" install
}
