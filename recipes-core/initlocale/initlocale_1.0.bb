DESCRIPTION = "Initialization Locale"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://vconsole.conf"

do_install() {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/vconsole.conf ${D}${sysconfdir}/vconsole.conf
}
