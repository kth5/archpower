# Maintainer:

pkgname=perl-cpan-requirements-dynamic
_pkgname=CPAN-Requirements-Dynamic
pkgver=0.002
pkgrel=2
pkgdesc='Dynamic prerequisites in meta files'
arch=(any)
license=(PerlArtistic GPL)
options=(!emptydirs)
depends=(perl
         perl-extutils-config)
url='https://metacpan.org/dist/CPAN-Requirements-Dynamic'
source=(https://cpan.metacpan.org/authors/id/L/LE/LEONT/$_pkgname-$pkgver.tar.gz)
sha512sums=('595b1e367eeeca94fd611f23f37217b8b46d50b829a6c4083c1350ecfb82d1adaf492e63dc5582feab310d67cfaf3f736b17b2eecafeb6c1d9bddeebe7205205')

build() {
  cd $_pkgname-$pkgver
  PERL_MM_USE_DEFAULT=1 perl Makefile.PL INSTALLDIRS=vendor
  make
}

check() {
  cd $_pkgname-$pkgver
  export PERL_MM_USE_DEFAULT=1 PERL5LIB=""
  make test
}

package() {
  cd $_pkgname-$pkgver
  make install DESTDIR="${pkgdir}"
}
