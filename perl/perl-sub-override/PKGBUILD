# Maintainer: Levente Polyak <anthraxx[at]archlinux[dot]org>

pkgname=perl-sub-override
_cpanname=Sub-Override
pkgver=0.12
pkgrel=2
pkgdesc='Perl extension for easily overriding subroutines'
url='https://metacpan.org/dist/Sub-Override'
arch=(any)
license=(Artistic-1.0-Perl)
depends=(
  perl
  perl-sub-prototype
)
checkdepends=(
  perl-test-fatal
)
options=('!emptydirs')
source=(https://cpan.metacpan.org/authors/id/M/MV/MVSJES/${_cpanname}-${pkgver}.tar.gz)
sha512sums=('3164764c54435c96b87a28b266bf39d3c1775008292605c17ab9f2077f6695a7d730202fd36d4b97638ca0e6b14db3ad3d62461ec855ddca938d9a31af1ee2c5')
b2sums=('1c07187ad909261c18a5f973d489918cb986b040377a4011821f7a6a0966ad03ba81796a35173ef43ac81778e1fb4601bf71423744187d9594f0907f65056d7f')

build() {
  cd ${_cpanname}-${pkgver}
  export PERL_MM_USE_DEFAULT=1
  export PERL_AUTOINSTALL=--skipdeps
  perl Makefile.PL INSTALLDIRS=vendor
  make
}

check() {
  cd ${_cpanname}-${pkgver}
  export PERL_MM_USE_DEFAULT=1
  make test
}

package() {
  cd ${_cpanname}-${pkgver}
  make DESTDIR="${pkgdir}" install INSTALLDIRS=vendor
  install -Dm 644 README* -t "${pkgdir}/usr/share/doc/${pkgname}"
}

# vim: ts=2 sw=2 et:
