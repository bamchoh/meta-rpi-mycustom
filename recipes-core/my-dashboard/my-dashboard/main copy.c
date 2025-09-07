#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <dirent.h>

#include "font8x8_basic.h"

// ピクセル描画
void put_pixel(char *fbp, int x, int y, int r, int g, int b,
               struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo)
{
    if (x < 0 || y < 0 || x >= vinfo.xres || y >= vinfo.yres) return;

    long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                    (y + vinfo.yoffset) * finfo.line_length;

    if (vinfo.bits_per_pixel == 32) {
        *(fbp + location)     = b; // Blue
        *(fbp + location + 1) = g; // Green
        *(fbp + location + 2) = r; // Red
        *(fbp + location + 3) = 0;
    } else {
        unsigned short pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        *((unsigned short*)(fbp + location)) = pixel;
    }
}

// 1文字描画
void draw_char(char *fbp, int x, int y, char c, int r, int g, int b,
               struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo)
{
    for (int row = 0; row < 8; row++) {
        unsigned char bits = font8x8_basic[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                put_pixel(fbp, x + col, y + row, r, g, b, vinfo, finfo);
            } else {
                put_pixel(fbp, x + col, y + row, 0, 0, 0, vinfo, finfo); // 背景を黒
            }
        }
    }
}

// 文字列描画
void draw_text(char *fbp, struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo,
               int x, int y, const char *text, int r, int g, int b)
{
    while (*text) {
        draw_char(fbp, x, y, *text, r, g, b, vinfo, finfo);
        x += 8;
        text++;
    }
}

int disp_ip_addr(char *fbp, struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo,
                 int offset_x, int offset_y) {
    int fd;
    struct ifreq ifr;
    char ip[INET_ADDRSTRLEN];

    const char *ifname = "eth0";

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    struct sockaddr_in *ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
    if (!inet_ntop(AF_INET, &ipaddr->sin_addr, ip, sizeof(ip))) {
        perror("inet_ntop");
        close(fd);
        return 1;
    }

    draw_text(fbp, vinfo, finfo, offset_x, offset_y, "| ", 255, 255, 255);
    draw_text(fbp, vinfo, finfo, offset_x+(2*8), offset_y, ip, 0, 255, 0);
    draw_text(fbp, vinfo, finfo, offset_x+(32*8), offset_y, " |", 255, 255, 255);

    close(fd);
    return 0;
}

// systemctl is-active の結果を取得
int get_service_status(const char *service, char *buf, size_t size)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "systemctl is-active %s 2>/dev/null", service);

    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;

    if (fgets(buf, size, fp) == NULL) {
        pclose(fp);
        return -1;
    }

    // 改行を削除
    buf[strcspn(buf, "\n")] = 0;
    pclose(fp);
    return 0;
}



int disp_service_status(char *fbp, struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo,
                 int offset_x, int offset_y, char *servicename) {
    // サービスの状態を取得
    char status[64];
    if (get_service_status(servicename, status, sizeof(status)) == 0) {
        // 状態によって色を変える
        int r=255, g=255, b=255;
        if (strcmp(status, "active") == 0) { r=0; g=255; b=0; }     // 緑
        else if (strcmp(status, "inactive") == 0) { r=255; g=255; b=0; } // 黄
        else if (strcmp(status, "failed") == 0) { r=255; g=0; b=0; } // 赤

        char status_disp[9] = "        "; // 8文字分のスペース
        strncpy(status_disp, status, 8); // 最大8文字までコピー

        draw_text(fbp, vinfo, finfo, offset_x, offset_y, "| ", 255, 255, 255);
        draw_text(fbp, vinfo, finfo, offset_x+(2*8), offset_y, servicename, 255, 255, 255);
        draw_text(fbp, vinfo, finfo, offset_x+(2*8)+(strlen(servicename)+1)*8, offset_y, status_disp, r, g, b);
        draw_text(fbp, vinfo, finfo, offset_x+(32*8), offset_y, " |", 255, 255, 255);
    } else {
        draw_text(fbp, vinfo, finfo, offset_x, offset_y, "Error getting status", 255, 0, 0);
    }
}

int is_iphone_connected(char *mntpoint) {
    struct stat st;
    if (stat(mntpoint, &st) == 0) {
        return 1; // 実際にアクセスできた
    }
    return 0; // 失敗（ぶち抜かれた or 未マウント）
}

int disp_mnt_status(char *fbp, struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo,
                 int offset_x, int offset_y, char *mntpoint, char* dispname) {
    // iPhone接続状態を取得
    int connected = is_iphone_connected(mntpoint);

    // 状態によって色を変える
    int r=255, g=255, b=255;
    char *status;
    if (connected) {
        status = "mounted  ";
        r=0; g=255; b=0; // 緑
    } else {
        status = "unmounted";
        r=255; g=0; b=0; // 赤
    }

    draw_text(fbp, vinfo, finfo, offset_x, offset_y, "| ", 255, 255, 255);
    draw_text(fbp, vinfo, finfo, offset_x+(2*8), offset_y, dispname, 255, 255, 255);
    draw_text(fbp, vinfo, finfo, offset_x+(2*8)+(strlen(dispname)+1)*8, offset_y, status, r, g, b);
    draw_text(fbp, vinfo, finfo, offset_x+(32*8), offset_y, " |", 255, 255, 255);

    return 0;
}

int disp_title(char *fbp, struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo,
                 int offset_x, int offset_y) {

    char *str1 = "*****************************";
    char *str2 = "* iPhone Data Gateway       *";
    draw_text(fbp, vinfo, finfo, offset_x, offset_x+0, str1, 255, 255, 255);
    draw_text(fbp, vinfo, finfo, offset_x, offset_x+8, str2, 255, 255, 255);
    draw_text(fbp, vinfo, finfo, offset_x, offset_x+16, str1, 255, 255, 255);

    return 0;
}

void disp_media_dir(char *fbp, struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo, int offset_x, int offset_y) {
    const char *path = "/run/media/system";
    DIR *dir = opendir(path);
    if (!dir) {
        draw_text(fbp, vinfo, finfo, offset_x, offset_y, "Cannot open dir", 255, 0, 0);
        return;
    }
    struct dirent *entry;
    int line = 0;

    for(int i = 0; i < 5; i++) {
        draw_text(fbp, vinfo, finfo, offset_x, offset_y + (i * 8), "                                ", 255, 255, 255);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            draw_text(fbp, vinfo, finfo, offset_x, offset_y + (line * 8), "* ", 255, 255, 255);
            draw_text(fbp, vinfo, finfo, offset_x+(2*8), offset_y + (line * 8), entry->d_name, 0, 255, 0);
            line++;
        }
    }
    closedir(dir);
}

int main()
{
    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) { perror("open"); return 1; }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) { perror("finfo"); return 2; }
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) { perror("vinfo"); return 3; }

    long screensize = vinfo.yres_virtual * finfo.line_length;
    char *fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((intptr_t)fbp == -1) { perror("mmap"); return 4; }

    // 背景を黒
    memset(fbp, 0, screensize);

    int offset_x = 20;
    int offset_y = 20;

    char loading_chars[9] = { '/', '-', '\\', '|', '/', '-', '\\', '|', 0 };
    int loading_index = 0;
    while(1) {
        disp_title(fbp, vinfo, finfo, offset_x, offset_y);

        char *str1 = "+--------------------------------+";
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(4*8), "[IP Address]", 0, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(5*8), str1, 255, 255, 255);
        disp_ip_addr       (fbp, vinfo, finfo, offset_x, offset_y+(6*8));
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(7*8), str1, 255, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(8*8), "", 255, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(9*8), "[Service Status]", 0, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(10*8), str1, 255, 255, 255);
        disp_service_status(fbp, vinfo, finfo, offset_x, offset_y+(11*8), "go-ftpserver");
        disp_service_status(fbp, vinfo, finfo, offset_x, offset_y+(12*8), "usbmuxd");
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(13*8), str1, 255, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(14*8), "", 255, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(15*8), "[iPhone Status]", 0, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(16*8), str1, 255, 255, 255);
        disp_mnt_status    (fbp, vinfo, finfo, offset_x, offset_y+(17*8), "/run/media/iphone/DCIM", "iPhone");
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(18*8), str1, 255, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(19*8), "", 255, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(20*8), "[USB storage Status]", 0, 255, 255);
        draw_text          (fbp, vinfo, finfo, offset_x, offset_y+(21*8), "", 255, 255, 255);
        disp_media_dir     (fbp, vinfo, finfo, offset_x+(2*8), offset_y+(22*8));

        char loading[2] = { loading_chars[loading_index], 0 };
        draw_text(fbp, vinfo, finfo, offset_x + 22 * 8, offset_y + 8, loading, 0, 255, 0);
        usleep(200000); // 200ms
        loading_index = (loading_index + 1) % 8;
    }

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}