#!/bin/sh
DEVNAME="$1"
ACTION="$2"

log() {
    echo "[usb-automount] $*" >> /var/log/usb-automount.log
}

MOUNT_BASE="/run/media/system"

case "$ACTION" in
    remove)
        # 以前マウントしたマウントポイントをログから探す
        MOUNTPOINT=$(grep "/dev/$DEVNAME" /var/log/usb-automount.log | tail -n1 | awk '{print $NF}')
        if [ -n "$MOUNTPOINT" ] && [ -d "$MOUNTPOINT" ]; then
            systemd-mount --umount "/dev/$DEVNAME"
            log "Unmounted /dev/$DEVNAME from $MOUNTPOINT"
            rmdir "$MOUNTPOINT" 2>/dev/null && log "Removed mount point $MOUNTPOINT"
        else
            # 念のため /run/media/system 以下に残っているディレクトリを削除
            for dir in "$MOUNT_BASE"/*; do
                if [ -d "$dir" ]; then
                    rmdir "$dir" 2>/dev/null && log "Removed leftover mount point $dir"
                fi
            done
        fi
        ;;
    *)
        sleep 1
        # systemd-mount は自動で /run/media/system/<LABEL> にマウント
        if systemd-mount --no-block "/dev/$DEVNAME"; then
            MOUNTPOINT=$(udevadm info -q all -n /dev/sda1 | awk -F= '/ID_FS_LABEL=/{print "/run/media/system/" $2}')
            log "Mounted /dev/$DEVNAME via systemd-mount at $MOUNTPOINT"
        else
            log "Failed to mount /dev/$DEVNAME via systemd-mount"
        fi
        ;;
esac
