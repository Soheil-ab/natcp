THIS_SCRIPT=$(cd "$(dirname "${BASH_SOURCE}")" ; pwd -P)
export SPROUT_MODEL_IN="$THIS_SCRIPT/alfalfa/src/examples/sprout.model"
export SPROUT_BT2="$THIS_SCRIPT/alfalfa/src/examples/sproutbt2"
export VERUS_SERVER="$THIS_SCRIPT/verus/src/verus_server"
export VERUS_CLIENT="$THIS_SCRIPT/verus/src/verus_client"

sudo sysctl net.ipv4.tcp_wmem="4096 32768 4194304" -q #Doubling the default value from 16384 to 32768
sudo sysctl -w net.ipv4.tcp_low_latency=1 -q
sudo sysctl -w net.ipv4.tcp_bbr_init_cwnd=1 -q
sudo sysctl -w net.ipv4.tcp_autocorking=0 -q
sudo sysctl -w net.ipv4.tcp_no_metrics_save=1 -q
sudo sysctl -w net.ipv4.ip_forward=1 -q
./build.sh
sudo cp wired48 /usr/local/share/mahimahi/traces/

DOWNLINKS=("ATT-LTE-driving.down"  "TMobile-LTE-driving.down" "TMobile-UMTS-driving.down" "Verizon-EVDO-driving.down" "Verizon-LTE-driving.down" "ATT-LTE-driving-2016.down"  "TMobile-LTE-short.down" "Verizon-LTE-short.down" "ATT-LTE-driving-2016.up" "TMobile-LTE-short.up" "Verizon-LTE-short.up"  "ATT-LTE-driving.up"  "TMobile-LTE-driving.up" "TMobile-UMTS-driving.up" "Verizon-EVDO-driving.up" "Verizon-LTE-driving.up" )
UPLINKS=("wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48" "wired48")
duration=("1012" "480" "930" "1065" "1365" "120" "140" "140" "120" "140" "140" "790" "480" "930" "1065" "1365");


