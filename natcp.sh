#!/bin/bash

if [ $# != 8 ]
then
   echo -e "usage:$0 latency lag_time trace_file_id naCubic natcp it qsize port"
   exit
fi

source setup.sh
sudo killall server tcpdump client 2>tmp
rm basetime

latency=$1
lag_time=$2
i=$3
nacubic=$4
natcp=$5
it=$6
qsize=$7
bw_scale=1
cwnd_scale=2
port=$8
fb_period=50    #Feedback Period
feedback_file=${DOWNLINKS[$i]}-${fb_period}-trace
echo $feedback_file >&2

if [ $nacubic -eq 1 ]
then
sudo su <<EOF
 echo "cubic" > /proc/sys/net/ipv4/tcp_congestion_control
 echo "0" > /proc/sys/net/ipv4/tcp_c2tcp_enable
EOF
     log="nacubic-${feedback_file}-$latency-${lag_time}-${it}"
    ./server $latency ./feedback/${feedback_file} $port ${DOWNLINKS[$i]} ${UPLINKS[$i]} $log $lag_time 3 ${bw_scale} ${cwnd_scale} 2 ${qsize} &
    sleep 4
    sleep ${duration[$i]}
    killall server mm-link client 2>tmp
    echo "" >> summary.tr
    echo "$log" >> summary.tr
    ./mm-thr 500 down-${log} 1>tmp 2>>summary.tr
    echo "++++++++++++++++++++++++++++++++++++" >> summary.tr
sudo su <<EOF
echo "cubic" > /proc/sys/net/ipv4/tcp_congestion_control
EOF
fi

port=$((port+50))
if [ $natcp -eq 1 ]
then
 sudo su <<EOF
 echo "bbr" > /proc/sys/net/ipv4/tcp_congestion_control
 echo "pfifo_fast" > /proc/sys/net/core/default_qdisc
 echo "1" > /proc/sys/net/ipv4/tcp_bbr_enable_maxdelay
 echo "10000" > /proc/sys/net/ipv4/tcp_bbr_minrttwinsec
 echo "1" > /proc/sys/net/ipv4/tcp_bbr_enable_probertt
 echo "0" > /proc/sys/net/ipv4/tcp_bbr_bw_auto
 echo "1" > /proc/sys/net/ipv4/tcp_bbr_init_cwnd
 echo "0" > /proc/sys/net/ipv4/tcp_bbr_enable_lt_bw
 echo "0" > /proc/sys/net/ipv4/tcp_bbr_enable_app_limited
EOF
 log="natcp-${feedback_file}-$latency-${lag_time}-${it}"
 ./server $latency ./feedback/${feedback_file} $port ${DOWNLINKS[$i]} ${UPLINKS[$i]} $log $lag_time 1 ${bw_scale} ${cwnd_scale} 2 ${qsize} &
 sleep 4
 sleep ${duration[$i]}
 sudo killall server mm-link client 2>tmp
 sudo su <<EOF
 echo "cubic" > /proc/sys/net/ipv4/tcp_congestion_control
 echo "0" > /proc/sys/net/ipv4/tcp_c2tcp_enable
EOF
 echo "" >> summary.tr
 echo "natcp -${feedback_file}-$latency-${lag_time}" >> summary.tr
 ./mm-thr 500 down-$log 1>tmp 2>>summary.tr
 echo "++++++++++++++++++++++++++++++++++++" >> summary.tr
fi

rm tmp

