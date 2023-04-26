#!/bin/sh

DIR=$(dirname "$0")
cd "$DIR"

show confirm.png
if [ -f "$USERDATA_PATH/.wifi/wifi_on.txt" ]; then
	say "WiFi on boot: Enabled"
else
	say "WiFi on boot: Disabled"
fi

while confirm; do
	show confirm.png
	if [ -f "$USERDATA_PATH/.wifi/wifi_on.txt" ]; then
		rm -f "$USERDATA_PATH/.wifi/wifi_on.txt"
		say "WiFi on boot: Disabled"
	else
		touch "$USERDATA_PATH/.wifi/wifi_on.txt"
		say "WiFi on boot: Enabled"
	fi
	sleep 1
done
