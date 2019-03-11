if [ $# == 10 ]
then
	sudo sysctl -w -q net.ipv4.ip_forward=1 -q
	sudo sysctl -w -q net.ipv4.tcp_no_metrics_save=1 -q
    ./run.sh $1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} > log
else
	echo "usage: $0 natcp nacubic cubic westwood sprout verus cubic+codel bbr port trace_index"
    echo -e "Trace Array (Index starts from 0 to 15): ("ATT-LTE-driving.down"  "TMobile-LTE-driving.down" "TMobile-UMTS-driving.down" "Verizon-EVDO-driving.down" "Verizon-LTE-driving.down" "ATT-LTE-driving-2016.down"  "TMobile-LTE-short.down" "Verizon-LTE-short.down" "ATT-LTE-driving-2016.up" "TMobile-LTE-short.up" "Verizon-LTE-short.up"  "ATT-LTE-driving.up"  "TMobile-LTE-driving.up" "TMobile-UMTS-driving.up" "Verizon-EVDO-driving.up" "Verizon-LTE-driving.up" )
"
fi

