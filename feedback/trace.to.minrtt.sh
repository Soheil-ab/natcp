#1-Get tracefile using particular <Bin Value>
#2-Print Capacity line from the trace.
if [ $# != 2 ]
then
    echo "usage $0 output_file_prefix input_sample"
    exit
fi
feedback_period=50
./mm-throughput-minrtt ${feedback_period} $2 1>$1-${feedback_period}
cat $1-${feedback_period} | awk 'BEGIN{a=0}{if($2!=0) {printf("%f %f %9.0f\n",$1,$2,12000/$2);a=12000/$2}else printf("%f %f %9.0f\n",$1,0,a)}{}' > $1-${feedback_period}-trace
rm $1-${feedback_period}

