#!/bin/sh
LANG=C
LANGUAGE=C
LC_ALL=C
c_date=$(date)
c_temp=$(mktemp)
>&2 printf "c_date: %s\n" "${c_date}"
>&2 printf "c_temp: %s\n" "${c_temp}"
printf "%s\n" "${c_date}" > "${c_temp}"
smbclient //192.168.1.238/Public 1234 \
    -U guestshare -c "put ${c_temp} date.txt"
