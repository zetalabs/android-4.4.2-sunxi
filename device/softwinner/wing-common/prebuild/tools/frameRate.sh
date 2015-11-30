#!bin/sh
while true
do
	nowCount=`getprop sys.service.framerate`
	echo frameRate:
	echo $(($nowCount-preCount))
	preCount=$nowCount
	sleep 1;
done