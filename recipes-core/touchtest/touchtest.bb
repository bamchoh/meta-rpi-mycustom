DESCRIPTION = "dashboard"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
"

S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o touchtest main.c -lm
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 touchtest ${D}${bindir}
}
