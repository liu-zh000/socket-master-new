#!/bin/bash

#######################
##  
##  $1:the channel_num
##  1~11, or 'auto'
##
#######################

MIN_CHAN_BG=1
MAX_CHAN_BG=11
#WIFI_BAND=``  #band=bg

print_usage()
{
    echo -e "Usage:"
    echo -e "    sudo $0 [<channel>] \t\t\t-------- range of <channel> is [1..11], or auto."
    echo -e "    $0 -h or $0 --help\t-------- help for this usage."
}

if [ `whoami` != root ]; then
    echo "Please run this script with sudo."
    print_usage
    exit 0
fi

if [ $# -eq 0 ] || [ x$1 == x"-h" ] || [ x$1 == x"--help" ]; then
    print_usage
    exit 0
fi

#check if $1 valid.
ret=` expr match $1 "[0-9]*$" `
if [ x$1 != x"auto" ]; then
    if [ ${ret} -eq 0 ]; then
        echo "only number or 'auto' is permitted."
        exit 1
    else 
	if [ $1 -lt ${MIN_CHAN_BG} ] || [ $1 -gt ${MAX_CHAN_BG} ]; then
	    echo "only number [1..11] or 'auto' is permitted for 2.4GHz."
            exit 1
	fi
    fi
fi

filename=/etc/NetworkManager/system-connections/ap_m.nmconnection
if [ ! -e $filename ]; then
    echo "no wifi config file!"
    exit 1
fi

line_num=`grep -n "channel=" ${filename}| awk -F':' '{print $1}'`
channel_num=$1
new_line="channel=${channel_num}"

if [ x$line_num == x ]; then  #if no 'channel=', means 'auto'
    if [ x"auto" == x$1 ]; then
        # $1 is 'auto', do nothing
	echo ""
    else # insert the line of 'channel='
	line_num_band=`grep -n "band=" ${filename}| awk -F':' '{print $1}'`
        sed -i "$line_num_band a $new_line" ${filename}
    fi
else #or exist 'channel='
    if [ x"auto" == x$1 ]; then # need to delete the line of 'channel='
        sed -i '/channel=/d' ${filename}
    else # replace 'channel=' which new value
        sed -i "$[ line_num ]c $new_line" ${filename}
    fi
fi
#nmcli c up ap_m
echo "set_done!"
exit 0
