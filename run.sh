#!/bin/bash

source setup.sh
sudo sudo killall verus_server sprout* iperf3 2>tmp

natcp=$1;
nacubic=$2;
cubic=$3;
westwood=$4;
sprout=$5;
verus=$6;
tcp_codel=$7
bbr=$8
port=${9}
i=${10}                  #trace_file_index
latency=5       #Intrinsic Unidirectional delay between Server and Client (ms)
lag=1           #Unidirectional delay between Server and NetAssist (ms)

if [ $cubic -eq 1 ]
then
  echo "TCP CUBIC: ${DOWNLINKS[$i]}" >&2
  sudo modprobe tcp_cubic
  sudo su <<EOF
echo "cubic" > /proc/sys/net/ipv4/tcp_congestion_control
echo "0" > /proc/sys/net/ipv4/tcp_c2tcp_enable
EOF
  ./run-iperf ${DOWNLINKS[$i]} ${UPLINKS[$i]} cubic-${DOWNLINKS[$i]} $((port+10+2*i)) $latency ${duration[$i]} &
  sleep ${duration[$i]}
  sudo killall iperf3 iperf mm-link 2>tmp
  logfile="cubic-${DOWNLINKS[$i]}"
  out="$logfile-$latency-${duration[$i]}"
  echo "$out" >>summary.tr
  ./mm-thr 500 down-$out 1>tmp 2>>summary.tr
  echo "-------------------------------------------------------------" >>summary.tr
  echo "Done" >&2
fi

if [ $bbr -eq 1 ]
then
   echo "BBR: ${DOWNLINKS[$i]}" >&2
   source set-bbr.sh
   ./run-iperf ${DOWNLINKS[$i]} ${UPLINKS[$i]} bbr-${DOWNLINKS[$i]} $((port+20+2*i)) $latency ${duration[$i]} 2>>log &
   sleep ${duration[$i]}
   sudo killall iperf3 iperf mm-link 2>tmp
   logfile="bbr-${DOWNLINKS[$i]}"
   out="$logfile-$latency-${duration[$i]}"
   echo "$out" >>summary.tr

   ./mm-thr 500 down-$out 1>tmp 2>>summary.tr
   echo "-------------------------------------------------------------" >>summary.tr
sudo su <<EOF
echo "cubic" > /proc/sys/net/ipv4/tcp_congestion_control
echo "0" > /proc/sys/net/ipv4/tcp_c2tcp_enable
EOF
  echo "Done" >&2
fi

if [ $verus -eq 1 ]
then
  echo "VERUS: ${DOWNLINKS[$i]}" >&2
  ./run-verus ${DOWNLINKS[$i]} ${UPLINKS[$i]} verus-${DOWNLINKS[$i]} $((port+30+2*i)) $latency ${duration[$i]} 2>>log &
  sleep ${duration[$i]}
  sudo killall iperf3 iperf mm-link verus* 2>tmp
  logfile="verus-${DOWNLINKS[$i]}"
  out="$logfile-$latency-${duration[$i]}"
  echo "$out" >>summary.tr
  ./mm-thr 500 down-$out 1>tmp 2>>summary.tr
  echo "-------------------------------------------------------------" >>summary.tr
  echo "Done" >&2
fi

if [ $sprout -eq 1 ]
then
  echo "SPROUT: ${DOWNLINKS[$i]}" >&2
  ./run-sprout ${DOWNLINKS[$i]} ${UPLINKS[$i]} sprout-${DOWNLINKS[$i]} $((port+40+2*i)) $latency 2>>log &
  sleep ${duration[$i]}
  sudo killall iperf3 iperf mm-link sprout* 2>tmp
    logfile="sprout-${DOWNLINKS[$i]}"
    out="$logfile-$latency"
   echo "$out" >>summary.tr
   ./mm-thr 500 up-$out 1>tmp 2>>summary.tr
   echo "-------------------------------------------------------------" >>summary.tr
  echo "Done" >&2
fi

if [ $tcp_codel -eq 1 ]
then
  echo "CUBIC+CODEL: ${DOWNLINKS[$i]}" >&2
  sudo modprobe tcp_cubic
  sudo su <<EOF
echo "cubic" > /proc/sys/net/ipv4/tcp_congestion_control
echo "0" > /proc/sys/net/ipv4/tcp_c2tcp_enable
EOF
 ./run-iperf-codel ${DOWNLINKS[$i]} ${UPLINKS[$i]} codel-${DOWNLINKS[$i]} $((port+50+2*i)) $latency ${duration[$i]} 2>>log &
  sleep ${duration[$i]}
  sudo killall iperf3 iperf mm-link 2>tmp
  logfile="codel-${DOWNLINKS[$i]}"
  out="$logfile-$latency-${duration[$i]}"
  echo "$out" >>summary.tr
  ./mm-thr 500 down-$out 1>tmp 2>>summary.tr
  echo "-------------------------------------------------------------" >>summary.tr
  echo "Done" >&2
fi

if [ $westwood -eq 1 ]
then
  echo "TCP Westwood: ${DOWNLINKS[$i]}" >&2
  sudo modprobe tcp_westwood
  sudo su <<EOF
 echo "westwood" > /proc/sys/net/ipv4/tcp_congestion_control
EOF
    ./run-iperf ${DOWNLINKS[$i]} ${UPLINKS[$i]} westwood-${DOWNLINKS[$i]} $((port+60+2*i)) $latency ${duration[$i]} 2>>log &
    sleep ${duration[$i]}
    sudo killall iperf3 iperf mm-link 2>tmp
    logfile="westwood-${DOWNLINKS[$i]}"
    out="$logfile-$latency-${duration[$i]}"
    echo "$out" >>summary.tr
    ./mm-thr 500 down-$out 1>tmp 2>>summary.tr
    echo "-------------------------------------------------------------" >>summary.tr
sudo su <<EOF
echo "cubic" > /proc/sys/net/ipv4/tcp_congestion_control
echo "0" > /proc/sys/net/ipv4/tcp_c2tcp_enable
EOF
  echo "Done" >&2
fi

if [ ${natcp} -eq 1 ]
then
  echo "NATCP ${DOWNLINKS[${i}]}" >&2
  sudo bash natcp.sh $latency $lag $i 0 1 test 100 $((port+5))
fi

if [ ${nacubic} -eq 1 ]
then
  echo "NACubic ${DOWNLINKS[${i}]}" >&2
  sudo bash natcp.sh $latency $lag $i 1 0 test 100 $((port+5))
fi

cat summary.tr >&2
