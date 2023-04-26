#!/bin/sh
# WiFi.pak

mkdir -p "$USERDATA_PATH/.wifi"

if [ -f /mnt/SDCARD/.system/paks/WiFi.pak/8188fu.ko ] && [ -f "$USERDATA_PATH/.wifi/wifi_on.txt" ] && [ -f /appconfigs/wpa_supplicant.conf ]; then
	if [ ! -f "$USERDATA_PATH/.wifi/telnet_on.txt" ]; then
		killall telnetd > /dev/null 2>&1 &
	fi
	if ! cat /proc/modules | grep -c 8188fu; then
		insmod /mnt/SDCARD/.system/paks/WiFi.pak/8188fu.ko
	fi
	ifconfig lo up
	/customer/app/axp_test wifion
	sleep 2
	ifconfig wlan0 up
	/customer/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script > /dev/null 2>&1 &

	# FTP
	if [ -f "$USERDATA_PATH/.wifi/ftp_on.txt" ]; then
		tcpsvd -E 0.0.0.0 21 ftpd -w /mnt/SDCARD > /dev/null 2>&1 &
	fi

	# SSH (TBD)
	# if [ ! -f "$USERDATA_PATH/.wifi/ssh_on.txt" ]; then
	# 	ssh_pass=$(cat "$USERDATA_PATH/.wifi/ssh_pass.txt")
	# 	if [ ! -z "$ssh_pass" ]; then
	# 		/mnt/SDCARD/.system/paks/WiFi.pak/dropbear -R -B -z -e -Y "$ssh_pass" > /dev/null 2>&1 &
	# 	fi
	# fi
else
	killall telnetd > /dev/null 2>&1 &
fi
