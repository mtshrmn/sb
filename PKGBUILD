pkgname=sb
pkgver=1.0.0
pkgrel=1
pkgdesc="a barebone infinite whiteboard"
arch=('x86_64')
url="https://github.com/mtshrmn/sb"
license=('GPL3')
depends=('cairo' 'sdl2')
makedepends=('gcc')
provides=("${pkgname}")
conflicts=("${pkgname}")
source=('sb'::'git+https://github.com/mtshrmn/sb.git')
md5sums=('SKIP')

build() {
    cd "${srcdir}/${pkgname}"
    make build
}

package() {
    cd "${srcdir}/${pkgname}"
    install -Dm 755 "${srcdir}/${pkgname}/build/${pkgname}" "${pkgdir}/usr/bin/${pkgname}"
    install -Dm 755 "${srcdir}/${pkgname}/LICENSE" "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
    install -Dm 644 "${srcdir}/${pkgname}/sb.desktop" "${pkgdir}/usr/share/applications/sb.desktop"
}
