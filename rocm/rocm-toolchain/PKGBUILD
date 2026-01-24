# Maintainer: Torsten Keßler <tpkessler at archlinux dot org>
pkgname=rocm-toolchain
pkgver=0.4.0
pkgrel=1
pkgdesc="Scripts for ROCm packaging"
arch=('any')
url="https://gitlab.archlinux.org/tpkessler/rocm-toolchain"
license=('GPL-3.0-or-later')
depends=('bash')
makedepends=('git')
source=("$pkgname::git+$url#tag=v$pkgver")
sha256sums=('47fb4a2aaec74527ae03531cd61d90c46e183158b39515b53d2036b8f9a31215')

package() {
	cd $pkgname
	install -Dm755 rocm-supported-gfx "$pkgdir"/usr/bin/rocm-supported-gfx
}
