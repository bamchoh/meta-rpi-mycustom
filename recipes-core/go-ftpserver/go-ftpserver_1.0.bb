LICENSE = "CLOSED"

SRC_URI = "https://go.dev/dl/go1.24.6.linux-amd64.tar.gz \
           git://github.com/fclairamb/ftpserver.git;branch=main;protocol=https \
           file://${BPN}.json \
           file://${BPN}.service \
"

SRC_URI[sha256sum] = "bbca37cc395c974ffa4893ee35819ad23ebb27426df87af92e93a9ec66ef8712"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

do_compile() {
    export GOOS=linux
    export GOARCH=arm
    export GOARM=7
    export CGO_ENABLED=0

    export PATH=${WORKDIR}/go/bin:$PATH
    export GOPATH=${WORKDIR}/gopath
    export GOCACHE=${WORKDIR}/gocache
    export GOMODCACHE=${WORKDIR}/gomodcache

    mkdir -p ${GOPATH} ${GOCACHE} ${GOMODCACHE}

    cd ${S}
    go mod tidy
    go mod download

    # ビルド
    go build -o ${B}/${BPN} .
}

inherit systemd

# systemd にサービス名を伝える
SYSTEMD_SERVICE:${BPN} = "${BPN}.service"
SYSTEMD_AUTO_ENABLE:${BPN} = "enable"

do_install() {
    # サービスファイルのインストール
    install -D -m 0644 ${WORKDIR}/${BPN}.service ${D}${systemd_system_unitdir}/${BPN}.service

    install -d ${D}${bindir}
    install -m 0755 ${B}/${BPN} ${D}${bindir}

    install -d ${D}${sysconfdir}/${BPN}
    install -m 0644 ${WORKDIR}/${BPN}.json ${D}${sysconfdir}/${BPN}
}
