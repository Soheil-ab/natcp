sudo su <<EOF
echo "bbr" > /proc/sys/net/ipv4/tcp_congestion_control
echo "pfifo_fast" > /proc/sys/net/core/default_qdisc
echo "0" > /proc/sys/net/ipv4/tcp_bbr_enable_maxdelay
echo "10" > /proc/sys/net/ipv4/tcp_bbr_minrttwinsec
echo "1" > /proc/sys/net/ipv4/tcp_bbr_bw_auto
echo "1" > /proc/sys/net/ipv4/tcp_bbr_enable_lt_bw
echo "1" > /proc/sys/net/ipv4/tcp_bbr_enable_app_limited
echo "1" > /proc/sys/net/ipv4/tcp_bbr_enable_probertt
EOF

