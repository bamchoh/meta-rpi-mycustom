DESCRIPTION = "Auto-load ads7846 module at boot"
LICENSE = "CLOSED"

SRC_URI = "file://ads7846.conf"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${sysconfdir}/modules-load.d
    install -m 0644 ${WORKDIR}/ads7846.conf ${D}${sysconfdir}/modules-load.d/
}

FILES:${PN} = "${sysconfdir}/modules-load.d/ads7846.conf"
