# Maintainer:

pkgname=perl-http-cookiejar
pkgver=0.014
pkgrel=4
pkgdesc='A minimalist HTTP user agent cookie jar'
_dist='HTTP-CookieJar'
arch=(any)
url="http://search.cpan.org/dist/HTTP-CookieJar"
license=(Apache)
depends=(perl-http-date)
checkdepends=(perl-test-deep perl-test-requires perl-http-message)
options=(!emptydirs)
source=(http://search.cpan.org/CPAN/authors/id/D/DA/DAGOLDEN/$_dist-$pkgver.tar.gz)
sha256sums=('7094ea5c91f536d263b85e83ab4e9a963e11c4408ce08ecae553fa9c0cc47e73')

build() (
  cd $_dist-$pkgver
  unset PERL5LIB PERL_MM_OPT PERL_LOCAL_LIB_ROOT
  export PERL_MM_USE_DEFAULT=1 PERL_AUTOINSTALL=--skipdeps
  perl Makefile.PL INSTALLDIRS=vendor
  make
)

check() (
  cd $_dist-$pkgver
  unset PERL5LIB PERL_MM_OPT PERL_LOCAL_LIB_ROOT
  export PERL_MM_USE_DEFAULT=1
  make test
)

package() (
  cd $_dist-$pkgver
  unset PERL5LIB PERL_MM_OPT PERL_LOCAL_LIB_ROOT
  make install INSTALLDIRS=vendor DESTDIR="$pkgdir"
)
