#!/system/bin/busybox sh

/system/bin/log -t pagecache "flush page cache"
echo 1 > /proc/sys/vm/drop_caches
