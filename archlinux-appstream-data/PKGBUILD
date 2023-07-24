# Maintainer: Antonio Rojas <arojas@archlinux.org>

pkgname=archlinux-appstream-data
pkgver=20230715
pkgrel=1
pkgdesc='Arch Linux application database for AppStream-based software centers'
arch=(any)
url='https://www.archlinux.org'
license=(GPL)
depends=()
makedepends=()
source=()
noextract=()
for _repo in core extra multilib; do
 source+=($_repo-$pkgver.xml.gz::https://sources.archlinux.org/other/packages/$pkgname/$pkgver/$_repo/Components-x86_64.xml.gz
          $_repo-icons-48x48-$pkgver.tar.gz::https://sources.archlinux.org/other/packages/$pkgname/$pkgver/$_repo/icons-48x48.tar.gz
          $_repo-icons-64x64-$pkgver.tar.gz::https://sources.archlinux.org/other/packages/$pkgname/$pkgver/$_repo/icons-64x64.tar.gz
          $_repo-icons-128x128-$pkgver.tar.gz::https://sources.archlinux.org/other/packages/$pkgname/$pkgver/$_repo/icons-128x128.tar.gz)
 noextract+=($_repo.xml.gz-$pkgver $_repo-icons-{48x48,64x64,128x128}-$pkgver.tar.gz)
done
sha256sums=('f32770b001d90903d45ac0ee05898e2c57d99bde560eac9ae2357b311300e681'
            '7989bb311baa38ef545250282aa065d23281c46dfb8faabe4c653487bdbded5c'
            '198c7aec4989984166174defe7b7eafda54d1f4ab4f83097e2d11ac1a9193fcf'
            '7989bb311baa38ef545250282aa065d23281c46dfb8faabe4c653487bdbded5c'
            'ae65e6fe2272545a91c04e3d59986ebe89cc984be19fc587d8e0afa1b1081cee'
            '943b53894e561cc831d3e9b0ba323a56695befe1d0c2762be92953c54df6c2af'
            '064f05c31e2eb7b2f5c54ee05a4701681860d35b5d376959775ff15382aca82a'
            'c88cc504c99ddc31e34fe6af7a2c84a3ddf8d7573c6b8f4683e065b88c8621a4'
            '9e364b5afec4390bc6aaa8e3ac385e49130b7224340f5c8cc57def89b5b85fa1'
            '2e6562ca40e3c8f8fcd22c6a6b3e3aab513d3fa9f45ad36c74aa16a4466b61b1'
            '4369ccd47698a55da9cfa441322b8b8250955683de4e6d496a568df4162e3625'
            '242bacebf03766e06f6683c509a2f31e7e96e6878f1b2e936d0d2e16441e43a9')

package() {
  mkdir -p "$pkgdir"/usr/share/app-info/{icons/archlinux-arch-{core,extra,multilib}/{48x48,64x64,128x128},xmls}
  for _repo in core extra multilib; do
   tar -xzf $_repo-icons-48x48-$pkgver.tar.gz -C "$pkgdir"/usr/share/app-info/icons/archlinux-arch-$_repo/48x48
   tar -xzf $_repo-icons-64x64-$pkgver.tar.gz -C "$pkgdir"/usr/share/app-info/icons/archlinux-arch-$_repo/64x64
   tar -xzf $_repo-icons-128x128-$pkgver.tar.gz -C "$pkgdir"/usr/share/app-info/icons/archlinux-arch-$_repo/128x128
   install -m644 $_repo-$pkgver.xml.gz "$pkgdir"/usr/share/app-info/xmls/$_repo.xml.gz
  done
}
