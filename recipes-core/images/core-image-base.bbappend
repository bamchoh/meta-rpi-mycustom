ROOTFS_POSTPROCESS_COMMAND += "disable_tty1_getty;"

disable_tty1_getty () {
    ln -sf /dev/null ${IMAGE_ROOTFS}/etc/systemd/system/getty@tty1.service
}

ROOTFS_POSTPROCESS_COMMAND += "set_my_timezone; "

set_my_timezone() {
    echo "Asia/Tokyo" > ${IMAGE_ROOTFS}/etc/timezone
    ln -sf /usr/share/zoneinfo/Asia/Tokyo ${IMAGE_ROOTFS}/etc/localtime
}