while true; do
    df | grep hda; sleep 5;
done | client /system/diskspace &

while true; do
    cat /proc/loadavg; sleep 5;
done | client /system/load &

