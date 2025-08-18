SYSTEMD_SERVICE:${PN} = "usbmuxd.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

SRC_URI += "file://usbmuxd.service"

do_install:append() {
    install -D -m 0644 ${WORKDIR}/usbmuxd.service ${D}${systemd_system_unitdir}/usbmuxd.service
}
