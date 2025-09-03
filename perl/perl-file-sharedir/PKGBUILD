# Maintainer: Jan Alexander Steffens (heftig) <heftig@archlinux.org>
# Contributor: Caleb Cushing <xenoterracide@gmail.com>

pkgname=perl-file-sharedir
_dist=File-ShareDir
pkgver=1.118
pkgrel=7
pkgdesc="Locate per-dist and per-module shared files"
url="https://metacpan.org/dist/$_dist"
arch=(any)
license=('Artistic-1.0-Perl OR GPL-1.0-or-later')
depends=(
  perl
  perl-class-inspector
)
options=('!emptydirs')
source=("https://cpan.metacpan.org/authors/id/R/RE/REHSACK/$_dist-$pkgver.tar.gz")
b2sums=('bdb2c2d786efda84567d56611fdfbf027665e6d0df21a1e4bcd92824dfa32ac9272cf4b2fb0eb2260317957af150df8a6919027bc664c594ac931ad48cec3f0d')

build() {
  cd $_dist-$pkgver
  perl Makefile.PL INSTALLDIRS=vendor
  make
}

package() {
  cd $_dist-$pkgver
  make DESTDIR="$pkgdir" install
}

# vim:set sw=2 sts=-1 et:
