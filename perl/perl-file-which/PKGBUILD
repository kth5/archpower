# Maintainer: Jan Alexander Steffens (heftig) <heftig@archlinux.org>
# Contributor: Charles Mauch <cmauch@gmail.com>

pkgname=perl-file-which
_dist=File-Which
pkgver=1.27
pkgrel=7
pkgdesc="Portable implementation of which"
url="https://metacpan.org/dist/$_dist"
arch=(any)
license=('Artistic-1.0-Perl OR GPL-1.0-or-later')
depends=(perl)
options=('!emptydirs')
source=("https://cpan.metacpan.org/authors/id/P/PL/PLICEASE/$_dist-$pkgver.tar.gz")
b2sums=('9252da514e45eac03bc415a0c5eede6731f0cd05df0a7894b975edccdac338e711f7793da75a94419e1ae737034c612f595a312d20a59244b2f6572eda2822cb')

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
