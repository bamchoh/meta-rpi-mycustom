DESCRIPTION = "Custom service to mount iPhone"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://mntiphone.service \
           file://mntiphone.sh \
          "

inherit systemd

# systemd にサービス名を伝える
SYSTEMD_SERVICE:${PN} = "mntiphone.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    # サービスファイルのインストール
    install -D -m 0644 ${WORKDIR}/mntiphone.service ${D}${systemd_system_unitdir}/mntiphone.service

    # スクリプトのインストール
    install -D -m 0755 ${WORKDIR}/mntiphone.sh ${D}/usr/local/bin/mntiphone.sh
}

FILES:${PN} += "${systemd_system_unitdir} /usr/local/bin/mntiphone.sh"
