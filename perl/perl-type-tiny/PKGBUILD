# Maintainer: Morten Linderud <foxboron@archlinux.org>
# Maintainer: Bruno Pagani <archange@archlinux.org>

_pkg=Type-Tiny
pkgname=perl-${_pkg,,}
pkgver=2.008002
pkgrel=2
pkgdesc="Tiny, yet Moo(se)-compatible type constraint"
arch=(any)
url="https://metacpan.org/release/${_pkg}"
license=(PerlArtistic GPL)
options=(!emptydirs)
depends=(perl perl-exporter-tiny)
source=(https://cpan.metacpan.org/authors/id/T/TO/TOBYINK/${_pkg}-${pkgver}.tar.gz)
sha512sums=('5781060cbd001650cf78a4fe1691caa653e0f18063ce797d8760b27fb5c55aae7bfb9978b4ebf37484991288083456ef0081361b173fd742cb81d1ae4d33b244')

build() {
    cd ${_pkg}-${pkgver}
    export PERL_MM_USE_DEFAULT=1 PERL_AUTOINSTALL=--skipdeps
    perl Makefile.PL
    make
}

check() {
    cd ${_pkg}-${pkgver}
    make test
}

package() {
    cd ${_pkg}-${pkgver}
    make INSTALLDIRS=vendor DESTDIR="${pkgdir}" install
}
