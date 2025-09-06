SUMMARY = "USB automount using systemd and udev"
DESCRIPTION = "Automatically mount USB storage devices via systemd-mount"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://usb-automount@.service \
           file://usb-automount.sh \
           file://99-automount.rules \
          "

S = "${WORKDIR}"

inherit systemd

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "usb-automount@.service"

do_install() {
    # スクリプト
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/usb-automount.sh ${D}${bindir}/usb-automount.sh

    # udev ルール
    install -d ${D}${sysconfdir}/udev/rules.d
    install -m 0644 ${WORKDIR}/99-automount.rules ${D}${sysconfdir}/udev/rules.d/

    # systemd サービス
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/usb-automount@.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += "${bindir} ${sysconfdir} ${systemd_system_unitdir}"  