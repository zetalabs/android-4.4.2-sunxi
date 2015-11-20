/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <dirent.h>
#include <fs_mgr.h>

#define LOG_TAG "Vold"

#include "cutils/klog.h"
#include "cutils/log.h"
#include "cutils/properties.h"

#include "VolumeManager.h"
#include "CommandListener.h"
#include "NetlinkManager.h"
#include "DirectVolume.h"
#include "cryptfs.h"

static int process_config(VolumeManager *vm);
static void coldboot(const char *path);

#define FSTAB_PREFIX "/fstab."
struct fstab *fstab;

int main() {

    VolumeManager *vm;
    CommandListener *cl;
    NetlinkManager *nm;

    SLOGI("Vold 2.1 (the revenge) firing up");

    mkdir("/dev/block/vold", 0755);

    /* For when cryptfs checks and mounts an encrypted filesystem */
    klog_set_level(6);

    /* Create our singleton managers */
    if (!(vm = VolumeManager::Instance())) {
        SLOGE("Unable to create VolumeManager");
        exit(1);
    };

    if (!(nm = NetlinkManager::Instance())) {
        SLOGE("Unable to create NetlinkManager");
        exit(1);
    };


    cl = new CommandListener();
    vm->setBroadcaster((SocketListener *) cl);
    nm->setBroadcaster((SocketListener *) cl);

    if (vm->start()) {
        SLOGE("Unable to start VolumeManager (%s)", strerror(errno));
        exit(1);
    }

    if (process_config(vm)) {
        SLOGE("Error reading configuration (%s)... continuing anyways", strerror(errno));
    }

    if (nm->start()) {
        SLOGE("Unable to start NetlinkManager (%s)", strerror(errno));
        exit(1);
    }

    coldboot("/sys/block");
//    coldboot("/sys/class/switch");

    /*
     * Now that we're up, we can respond to commands
     */
    if (cl->startListener()) {
        SLOGE("Unable to start CommandListener (%s)", strerror(errno));
        exit(1);
    }

    // Eventually we'll become the monitoring thread
    while(1) {
        sleep(1000);
    }

    SLOGI("Vold exiting");
    exit(0);
}

static void do_coldboot(DIR *d, int lvl)
{
    struct dirent *de;
    int dfd, fd;

    dfd = dirfd(d);

    fd = openat(dfd, "uevent", O_WRONLY);
    if(fd >= 0) {
        write(fd, "add\n", 4);
        close(fd);
    }

    while((de = readdir(d))) {
        DIR *d2;

        if (de->d_name[0] == '.')
            continue;

        if (de->d_type != DT_DIR && lvl > 0)
            continue;

        fd = openat(dfd, de->d_name, O_RDONLY | O_DIRECTORY);
        if(fd < 0)
            continue;

        d2 = fdopendir(fd);
        if(d2 == 0)
            close(fd);
        else {
            do_coldboot(d2, lvl + 1);
            closedir(d2);
        }
    }
}

static void coldboot(const char *path)
{
    DIR *d = opendir(path);
    if(d) {
        do_coldboot(d, 0);
        closedir(d);
    }
}

static int process_config(VolumeManager *vm)
{
    char fstab_filename[PROPERTY_VALUE_MAX + sizeof(FSTAB_PREFIX)];
    char propbuf[PROPERTY_VALUE_MAX];
    int i;
    int ret = -1;
    int flags;

    property_get("ro.hardware", propbuf, "");
    snprintf(fstab_filename, sizeof(fstab_filename), FSTAB_PREFIX"%s", propbuf);

    fstab = fs_mgr_read_fstab(fstab_filename);
    if (!fstab) {
        SLOGE("failed to open %s\n", fstab_filename);
        return -1;
    }

    char cmdline[1024], partition_dev[32], partition_name[32];
    char *ptr;
    int fd, len;
    char dev_path[64];
    char tmp_type[2];
    int boot_type = -1;

    fd = open("/proc/cmdline", O_RDONLY);
    if (fd >= 0) {
        int n = read(fd, cmdline, 1023);
        if (n < 0) n = 0;

            /* get rid of trailing newline, it happens */
            if (n > 0 && cmdline[n-1] == '\n') n--;

            cmdline[n] = 0;
            close(fd);
    } else {
        cmdline[0] = 0;
        SLOGE("Error open /proc/cmdline");
        return -1;
    }

    ptr = strstr(cmdline, "boot_type=");
    if(ptr) {
        ptr += strlen("boot_type=");
        memccpy(tmp_type, ptr, ' ', 2);
        boot_type = atoi(tmp_type);
    }
    SLOGI("> boot_type = %d\n", boot_type);

    if(boot_type == 0) {
        ptr = strstr(cmdline, "partitions=");
        ptr += strlen("partitions=");
        memset(dev_path, 0, sizeof(dev_path));
        memset(partition_dev, 0, sizeof(partition_dev));
        memset(partition_name, 0, sizeof(partition_name));

        while (ptr && *ptr) {
            char *x = strchr(ptr, '@');
            len = x - ptr;
            memccpy(partition_name, ptr, '@', len);
            ptr = x + 1;
            x = strchr(ptr, ':');
            if(x == NULL) {
                len = strlen(ptr);
            } else {
                len = x - ptr;
            }

            memccpy(partition_dev, ptr, ':', len);

            if(!strncmp(partition_name, "UDISK", 5)) {
                snprintf(dev_path, sizeof(dev_path), "/devices/virtual/block/%s", partition_dev);
                SLOGI("> dev_path = %s\n", dev_path);
                break;
            }

            memset(dev_path, 0, sizeof(dev_path));
            memset(partition_dev, 0, sizeof(partition_dev));
            memset(partition_name, 0, sizeof(partition_name));

            if(x == NULL)
                break;
            else
                ptr = x + 1;
            }
    } else {
        memset(dev_path, 0, sizeof(dev_path));
        snprintf(dev_path, sizeof(dev_path),
            "/devices/platform/sunxi-mmc.%d/mmc_host",
            (boot_type == 2 ? 2 : 0));
            SLOGI("> dev_path = %s\n", dev_path);
    }

    /* Loop through entries looking for ones that vold manages */
    for (i = 0; i < fstab->num_entries; i++) {
        if (fs_mgr_is_voldmanaged(&fstab->recs[i])) {
            DirectVolume *dv = NULL;
            flags = 0;

            /* Set any flags that might be set for this volume */
            if (fs_mgr_is_nonremovable(&fstab->recs[i])) {
                flags |= VOL_NONREMOVABLE;
            }
            if (fs_mgr_is_encryptable(&fstab->recs[i])) {
                flags |= VOL_ENCRYPTABLE;
            }
            /* Only set this flag if there is not an emulated sd card */
            if (fs_mgr_is_noemulatedsd(&fstab->recs[i]) &&
                !strcmp(fstab->recs[i].fs_type, "vfat")) {
                flags |= VOL_PROVIDES_ASEC;
            }

            if (!strcmp(fstab->recs[i].label, "sdcard")) {
                if (boot_type != 0)
                    fstab->recs[i].partnum = 1;
                fstab->recs[i].blk_device = dev_path;
            }

            dv = (DirectVolume*) vm->lookupVolume(fstab->recs[i].label);
            if (dv == NULL) {
                dv = new DirectVolume(vm, &(fstab->recs[i]), flags);
                vm->addVolume(dv);
             }   

            if (dv->addPath(fstab->recs[i].blk_device)) {
                SLOGE("Failed to add devpath %s to volume %s",
                      fstab->recs[i].blk_device, fstab->recs[i].label);
                goto out_fail;
            }

        }
    }

    ret = 0;

out_fail:
    return ret;
}
