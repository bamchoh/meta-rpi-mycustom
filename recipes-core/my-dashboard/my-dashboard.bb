DESCRIPTION = "dashboard"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://font8x8_basic.h \
           file://my-dashboard.service \
"

S = "${WORKDIR}"

inherit systemd

# systemd にサービス名を伝える
SYSTEMD_SERVICE:${PN} = "my-dashboard.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o my-dashboard main.c -lm
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 my-dashboard ${D}${bindir}

    # サービスファイルのインストール
    install -D -m 0644 ${WORKDIR}/my-dashboard.service ${D}${systemd_system_unitdir}/my-dashboard.service
}
