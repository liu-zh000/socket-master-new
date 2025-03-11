#!/bin/bash

#############################
##
## this script return the following value
##     auto(current n)/1/2/3/../11
##      
##
#############################

print_usage()
{
    echo -e "Usage:"
    echo -e "    sudo $0 \t\t\t\t-------- return string from one of following: auto(current n)/1/2/3/../11"
    echo -e "    $0 -h or $0 --help\t-------- help for this usage."
}


if [ `whoami` != root ]; then
    echo "Please run this script with sudo."
    print_usage	
    exit 0
fi

if [ $# -gt 0 ]; then
    print_usage
    exit 0
fi

filename=/etc/NetworkManager/system-connections/ap_m.nmconnection
if [ ! -e $filename ]; then
    echo "no wifi config file!"
    exit 1
fi

line_num=`grep -n "channel=" ${filename}`

if [ x$line_num == x ]; then  #if no 'channel=', means 'auto'
    echo "auto"
else # get the value of channel
    match_line=`grep "channel=" ${filename}`
    channel_num=`echo ${match_line:8}`
    echo ${channel_num}
fi
exit 0
