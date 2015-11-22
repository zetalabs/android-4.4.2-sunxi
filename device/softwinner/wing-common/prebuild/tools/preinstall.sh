#!/system/bin/busybox sh

BUSYBOX="/system/bin/busybox"

/system/bin/chown system:system /dev/block/by-name/misc
/system/bin/chmod 0660 /dev/block/by-name/misc

mkdir /bootloader
mount -t vfat /dev/block/by-name/bootloader  /bootloader

if [ ! -e /data/system.notfirstrun ]; then	
    echo "do preinstall job"	
  
	/system/bin/sh /system/bin/pm preinstall /system/preinstall
	/system/bin/sh /system/bin/pm preinstall /sdcard/preinstall
	/system/bin/sh /system/bin/data_copy.sh


	$BUSYBOX touch /data/system.notfirstrun	
	
	 mkdir /databk
   mount -t ext4 /dev/block/by-name/databk /databk
   rm /databk/data_backup.tar
   umount /databk
   rmdir /databk
   echo "preinstall ok"

elif [ -e /bootloader/data.need.backup ];then
   echo "data backup:tar /databk/data_backup.tar /data"
   mkdir /databk
   mount -t ext4 /dev/block/by-name/databk /databk
   
   rm /databk/data_backup.tar

   $BUSYBOX tar -cf /databk/data_backup.tar /data
   rm /bootloader/data.need.backup
  
   umount /databk
   rmdir /databk

else 
   umount /databk
   rmdir /databk
   echo "do nothing"
fi
umount /bootloader
rmdir /bootloader




