#!/bin/sh

mntiphone() {
    MOUNT_POINT="/run/media/iphone"

    is_mounted() {
        mount | grep -q "$MOUNT_POINT"
    }

    if [ ! -d "$MOUNT_POINT" ]; then
        if ! is_mounted; then
            echo "create $MOUNT_POINT"
            mkdir -p "$MOUNT_POINT"
        fi
    fi

    if idevicepair validate >/dev/null 2>&1; then
        echo "iphone is pairing"
        if is_mounted; then
            if ! ls "$MOUNT_POINT" >/dev/null 2>&1; then
                umount "$MOUNT_POINT"
            else
                echo "$MOUNT_POINT has already been mounted"
                return 0
            fi
        fi
        ifuse "$MOUNT_POINT"
    else
        idevicepair pair
    fi
}

while true; do
    mntiphone
    sleep 1
done