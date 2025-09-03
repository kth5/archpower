# Maintainer: Jan Alexander Steffens (heftig) <heftig@archlinux.org>
# Contributor: Caleb Cushing <xenoterracide@gmail.com>

pkgname=perl-capture-tiny
_dist=Capture-Tiny
pkgver=0.50
pkgrel=3
pkgdesc="Capture STDOUT and STDERR from Perl, XS or external programs"
url="https://metacpan.org/dist/$_dist"
arch=(any)
license=(Apache-2.0)
depends=(perl)
options=('!emptydirs')
source=("https://cpan.metacpan.org/authors/id/D/DA/DAGOLDEN/$_dist-$pkgver.tar.gz")
b2sums=('a4d857e6e7238b599afc500516d13d10a7eb060922981d5eb0d6be24c98875ba490c4f2d66b340339d61d5731cf663da0222dc02793a00bf304f3b20146da826')

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
