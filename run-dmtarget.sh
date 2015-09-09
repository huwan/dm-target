#! /bin/bash
# See http://narendrapal2020.blogspot.com/2014/03/device-mapper.html and
# http://techgmm.blogspot.com/p/writing-your-own-device-mapper-target.html.

usage ()
{
	echo "usage: `basename $0` [-sdfrh]"
	echo "      -s            setup device mapper target"
	echo "      -d            test using dd"
	echo "      -f            test using filesystem"
	echo "      -r            remove device mapper target"
	echo "      -h            display help"
}

if ( ! getopts ":sdfrh" option )
then
       usage
       exit 1
fi

while getopts ':sdfrh' option;
do
	case "$option" in
	s)
		dd if=/dev/zero of=/tmp/mydisk bs=1M count=128 # 128MB file
		losetup /dev/loop0 /tmp/mydisk # losetup -f
		make -s
		insmod ./mapper.ko
		# echo <starting logical sector number> <logical size of device in terms of sector> <target name> <device path> <unsed paramter> | dmsetup create <mapper  name>
		echo 0 262144 hello_target /dev/loop0 0 | dmsetup create my_device_mapper
		;;
	d)
		dd if=/dev/urandom of=/dev/mapper/my_device_mapper  bs=1K count=16
		;;
	f)
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
	r)
		if [ -d /mnt/mapper ]
		then
			umount /mnt/mapper
			rm -rf /mnt/mapper
		fi
		dmsetup remove my_device_mapper
		losetup -d /dev/loop0
		rmmod mapper
		make clean -s
		rm /tmp/mydisk
		;;
	h)
		usage
		exit
		;;
	\?)
		printf "illegal option: -%s\n" "$OPTARG" >&2
		usage
		exit 1
		;;
	esac
done
shift $((OPTIND - 1))
