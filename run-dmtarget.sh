#! /bin/bash
# http://narendrapal2020.blogspot.com/2014/03/device-mapper.html
if [ $# -lt 1 ]
then
	echo "Usage: $0 setup|remove|test|..."
	echo "see $0 for detail."
	exit
fi


case $1 in
	setup|on|1)
		dd if=/dev/zero of=/tmp/mydisk bs=1M count=1024 # 1GB file
		losetup /dev/loop0 /tmp/mydisk # losetup -f
		make
		insmod ./mapper.ko
		# echo <starting logical sector number> <logical size of device in terms of sector> <target name> <device path> <unsed paramter> | dmsetup create <mapper  name>
		echo 0 2097152 hello_target /dev/loop0 0 | dmsetup create my_device_mapper
		;;
	test|2)
		dd if=/dev/urandom of=/dev/mapper/my_device_mapper  bs=1K count=16
		;;
	test2|3)
		if [ ! -d /mnt/mapper ]
		then
			mkdir -p /mnt/mapper
		fi
		modprobe ext4
		mkfs.ext4 -q /dev/mapper/my_device_mapper
		mount /dev/mapper/my_device_mapper /mnt/mapper
		cd /mnt/mapper
		touch test.txt
		cp test.txt copy.txt
		ls
		;;
	remove|off|0)
		umount /mnt/mapper
		dmsetup remove my_device_mapper
		losetup -d /dev/loop0
		rmmod mapper
		make clean
		rm /tmp/mydisk
		if [ -d /mnt/mapper ]
		then
			rm -rf /mnt/mapper
		fi
		;;
	*)
		echo "Bad argument!"
		echo "Usage: $0 setup|remove|test|..."
		echo "see $0 for detail."
		;;
esac
