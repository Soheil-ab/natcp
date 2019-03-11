#Generate Feedback file for a trace

To generate a proper feedback file for a new trace file, you need to first use the trace file and for example cubic (or any other TCP) to run the evaluation. Later use the output file of that evaluation (say down-cubic-blahblah) and run the following:

    ```
    ./trace.to.minrtt.sh blahblah down-cubic-blahblah
    ```

A feedback file named blahblah-50-trace will be generated. Now copy the file in feedback directory.

