ROOTFS_POSTPROCESS_COMMAND += "disable_tty1_getty;"

disable_tty1_getty () {
    ln -sf /dev/null ${IMAGE_ROOTFS}/etc/systemd/system/getty@tty1.service
}