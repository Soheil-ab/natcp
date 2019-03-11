//============================================================================
// Author      : Soheil Abbasloo (ab.soheil@nyu.edu)
// Version     : V1.0
//============================================================================

/*
  MIT License
  Copyright (c) 2018 Soheil Abbasloo

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "flow.h"
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
pthread_mutex_t lockit;
#include <unistd.h>
#include <math.h>
#include <time.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
struct timeval tv_start,tv_start2;	//Start time (after three way handshake)
struct timeval tv_stamped,tv_stamped2;
uint64_t start_of_client;
struct timeval tv_end,tv_end2;		//End time
bool done=false;
bool check=false;
uint64_t setup_time;  //Flow Completion Time
char *downlink;
char *uplink;
char *log_file;
int it=0;
double lag_time=0;
int feedback=0;
bool got_message=0;
double cwnd_scale=2;
double bw_scale=1;
int codel=0;
int qsize=100;
//#define HYPE

#define DBGSERVER  0 
#define CWND_INIT 10//250
#define HASH_RANGE 5
#define GAIN 2 //3 
#define GAIN_BW 1 

#define TCP_BBR_EN_MAXDEL 33
#define TCP_BBR_EN_PRBRTT 34
#define TCP_BBR_TRGTDEL_MS 35
#define TCP_BBR_MINRTTWIN_SEC 36
#define TCP_BBR_PRBERTTMDE_MS 37
#define TCP_BBR_BWAUTO 38
#define TCP_BBR_BWVAL 39
#define TCP_BBR_CWNDRVGAIN 40
#define TCP_BBR_DEBUG 41
#define TCP_BBR_APPLY 41
#define TCP_CWND_CLAMP 42

struct sTrace
{
	double time;
	double bw;
	double minRtt;
};
struct sInfo
{
	sTrace *trace;
	int sock;
	int num_lines;
};
int delay_ms;
int client_port;
sTrace *trace;

/* Code for the Server-Client Application
*/
uint64_t raw_timestamp( void )
{
    struct timespec ts;
    clock_gettime( CLOCK_REALTIME, &ts );
    uint64_t us = ts.tv_nsec / 1000;
    us += (uint64_t)ts.tv_sec * 1000000;
    return us;
}
uint64_t timestamp_begin(bool set)
{
        static uint64_t start;
        if(set)
            start = raw_timestamp();
        return start;
}
uint64_t timestamp_end( void )
{
        return raw_timestamp() - timestamp_begin(0);
}

uint64_t initial_timestamp( void )
{
        static uint64_t initial_value = raw_timestamp();
        return initial_value;
}

uint64_t timestamp( void )
{
        return raw_timestamp() - initial_timestamp();
}

enum { MAXLINES = 2000000 };
int scan_file(sTrace *tr, const char *line)
{
//	DBGPRINT(DBGSERVER,5,"Scan Func!!\n");
	// Change for the number of data in the file per line
	if (sscanf(line, "%lf %lf %lf", &tr->time, &tr->bw,&tr->minRtt) != 3)
		return -1;
	return 0;
}
void print_trace(FILE *fp, const char *tag, const sTrace *tr)
{
	fprintf(fp, "%s : %lf %lf %lf\n", tag, tr->time, tr->bw, tr->minRtt);
}

/* End of Code Segment */
//Start TCP server
void start_server(int flow_num, int client_port,char*);

int THRESHOLD;
#define UPDATE_SEGMENT 1000000 //10000000 //never! #1000KB
//thread functions
void* DataThread(void*);
void* CntThread(void*);
//Print usage information
void usage();

int set_qplus(int sock,int state,int isbig, int cwnd,int path)
{
return 0;
}
void get_info(int sk, struct tcp_info *info)
{
	int tcp_info_length = sizeof(*info);

	if ( getsockopt( sk, SOL_TCP, TCP_INFO, (void *)info, (socklen_t *)&tcp_info_length ) == 0 )
	{
		DBGMARK(DBGSERVER,4,"%u %u %u %u %u %u %u %u %u %u %u %u\n",
        info->tcpi_last_data_sent,
        info->tcpi_last_data_recv,
        info->tcpi_snd_cwnd,
        info->tcpi_snd_ssthresh,
        info->tcpi_rcv_ssthresh,
        info->tcpi_rtt,
        info->tcpi_rttvar,
        info->tcpi_unacked,
        info->tcpi_sacked,
        info->tcpi_lost,
        info->tcpi_retrans,
        info->tcpi_fackets);
	}
};

void handler(int sig) {
	void *array[10];
	size_t size;
	DBGMARK(DBGSERVER,2,"=============================================================== Start\n");
	// get void*'s for all entries on the stack
	size = backtrace(array, 20);
/*	fprintf(stderr, "We got signal %d:\n", sig);
    if(check)
        DBGERROR("============> Till %f of trace has been read. <==============\n",trace[it-1].time);
    else
        DBGERROR("============> Till %f of trace has been read. <==============\n",trace[it].time);
    DBGPRINT(0,0,"time_rcv:%"  PRIu64  "\n",start_of_client);   
	DBGMARK(DBGSERVER,2,"=============================================================== End\n");
*/
	fprintf(stderr, "Done.\n");
    exit(1);

}

int main(int argc, char **argv)
{
/*	struct sched_param param;
    param.__sched_priority=sched_get_priority_max(SCHED_RR);
    int policy=SCHED_RR;
    int s = pthread_setschedparam(pthread_self(), policy, &param);
    if (s!=0)
    {
        DBGERROR("Cannot set priority (%d) for the Main: %s\n",param.__sched_priority,strerror(errno));
    }

    s = pthread_getschedparam(pthread_self(),&policy,&param);
    if (s!=0)
    {
        DBGERROR("Cannot get priority for the Data thread: %s\n",strerror(errno));
    }
*/
    DBGPRINT(DBGSERVER,4,"Main\n");

    if(argc!=13)
	{
		usage();
		return 0;
	}

	signal(SIGSEGV, handler);   // install our handler
	signal(SIGTERM, handler);   // install our handler
	signal(SIGABRT, handler);   // install our handler
	signal(SIGFPE, handler);   // install our handler
    signal(SIGKILL,handler);   // install our handler
    int flow_num;
	const char* cnt_ip;
    cnt_ip="10.10.10.10";
	int cnt_port;

	bool qplus_enable;
	flow_num=1;
	delay_ms=atoi(argv[1]);
	client_port=atoi(argv[3]);
	qplus_enable=0;
	THRESHOLD=10000000;
	char* file_name=argv[2];
    downlink=argv[4];
    uplink=argv[5];
    log_file=argv[6];
    lag_time=1000.0*atoi(argv[7]);//us lag_time of feedback
    feedback=atoi(argv[8]);
    bw_scale=atoi(argv[9]);
    cwnd_scale=atoi(argv[10]);
    codel=atoi(argv[11]);
    qsize=atoi(argv[12]);
    DBGPRINT(DBGSERVER,1,"filename:%s\n",file_name);

	start_server(flow_num, client_port,file_name);
	DBGMARK(DBGSERVER,5,"DONE!\n");
	return 0;
}

void usage()
{
	DBGMARK(0,0,"./server.o [Delay(ms)] [Trace file name] [port] [DL-trace] [UP-trace] [log] [lag time of feedback(ms) ] [NACubic/NATCP: 1==> NATCP, 3==>NATCP ] [bw_scale] [cwnd_scale] [AQM:1 => codel, 2 => fifo] [qsize in pkts]\n");
}

void start_server(int flow_num, int client_port, char* file_name)
{
	cFlow *flows;
    int num_lines=0;
	bool qplus_enable=0;
	FILE* filep;
	sInfo *info;
	info = new sInfo;

	char line[4096];
	trace = new sTrace[MAXLINES];
	int msec = 0, trigger = 10; /* 10ms */
	clock_t before = clock();

	if ((filep = fopen(file_name, "r")) == NULL)
	{ /* Open source file. */
		DBGERROR("fopen source-file error");
	return ;
	}

	while (fgets(line, sizeof(line), filep)!=0)
	{
		if (scan_file(&trace[num_lines], line) == 0)
		{
		    num_lines++;
        }
	}

	fclose(filep);
	flows = new cFlow[flow_num];
	int cnt_port=PORT_CTR;
	if(flows==NULL)
	{
		DBGMARK(0,0,"flow generation failed\n");
		return;
	}
	//Socket for server
	int server_sockfd;

	//threads
	pthread_t data_thread;
	pthread_t cnt_thread;

	//Server address
	struct sockaddr_in server_addr;
	//Client address
	struct sockaddr_in client_addr;
	//Controller address
	struct sockaddr_in ctr_addr;

	memset(&server_addr,0,sizeof(server_addr));
	//IP protocol
    server_addr.sin_family=AF_INET;
	//Listen on "0.0.0.0" (Any IP address of this host)
    server_addr.sin_addr.s_addr=INADDR_ANY;
	//Specify port number
    server_addr.sin_port=htons(client_port);

	//Init socket
	if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		DBGMARK(0,0,"sockopt: %s\n",strerror(errno));
		return;
	}
	int reuse = 1;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	//Bind socket on IP:Port
	if(bind(server_sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr))<0)
	{
		DBGMARK(0,0,"bind error srv_ctr_ip: 000000: %s\n",strerror(errno));
		close(server_sockfd);
		return;
	}

    char cmd[1000];
    if(codel==1)
    {
        sprintf(cmd, "sudo -u `logname` mm-delay %d mm-link /usr/local/share/mahimahi/traces/%s /usr/local/share/mahimahi/traces/%s --once --uplink-log=up-%s --downlink-log=down-%s --uplink-queue=codel --uplink-queue-args=\"interval=100, target=5, packets=%d\" --downlink-queue=codel --downlink-queue-args=\"interval=100, target=5, packets=%d\" -- sh -c \'sudo ./client $MAHIMAHI_BASE 1 2700 100.64.0.4 %d\' &",delay_ms,uplink,downlink,log_file,log_file,qsize,qsize,client_port);
    }
    else if (codel==2) {
        sprintf(cmd, "sudo -u `logname` mm-delay %d mm-link /usr/local/share/mahimahi/traces/%s /usr/local/share/mahimahi/traces/%s --once --uplink-log=up-%s --downlink-log=down-%s --uplink-queue=droptail --uplink-queue-args=\"packets=%d\" --downlink-queue=droptail --downlink-queue-args=\"packets=%d\" -- sh -c \'sudo ./client $MAHIMAHI_BASE 1 2700 100.64.0.4 %d\' &",delay_ms,uplink,downlink,log_file,log_file,qsize,qsize,client_port);
    }
    else
    {
        sprintf(cmd, "sudo -u `logname` mm-delay %d mm-link /usr/local/share/mahimahi/traces/%s /usr/local/share/mahimahi/traces/%s --once --uplink-log=up-%s --downlink-log=down-%s -- sh -c \'sudo ./client $MAHIMAHI_BASE 1 2700 100.64.0.4 %d\' &",delay_ms,uplink,downlink,log_file,log_file,client_port);
    }
    DBGPRINT(DBGSERVER,0,"%s\n",cmd);
    info->trace=trace;
    info->num_lines=num_lines;
    initial_timestamp();
    system(cmd);         
	//Start to listen
	//The maximum number of concurrent connections is 40000
	listen(server_sockfd,40000);
	int sin_size=sizeof(struct sockaddr_in);
	int flow_index=0;
	while(flow_index<flow_num)
	{
		int value=accept(server_sockfd,(struct sockaddr *)&client_addr,(socklen_t*)&sin_size);
		if(value<0)
		{
			perror("accept error\n");
			DBGMARK(0,0,"sockopt: %s\n",strerror(errno));
			close(server_sockfd);
			return;
		}
		info->sock=value;
        DBGMARK(DBGSERVER,4,"socket:%d\n",value);
        if(feedback)
        {
            if(pthread_create(&cnt_thread, NULL , CntThread, (void*)info) < 0)
            {
                perror("could not create control thread\n");
                close(server_sockfd);
                return;
            }
        }
		flows[flow_index].flowinfo.sock=value;
		flows[flow_index].dst_addr=client_addr;

		if(pthread_create(&data_thread, NULL , DataThread, (void*)&flows[flow_index]) < 0)
		{
			perror("could not create thread\n");
			close(server_sockfd);
			return;
		}
		flow_index++;
	}
    pthread_join(data_thread, NULL);
}
void* CntThread(void* information)
{
	struct sched_param param;
    param.__sched_priority=sched_get_priority_max(SCHED_RR);
    int policy=SCHED_RR;
    int s = pthread_setschedparam(pthread_self(), policy, &param);
    if (s!=0)
    {
        DBGERROR("Cannot set priority (%d) for the Main: %s\n",param.__sched_priority,strerror(errno));
    }

    s = pthread_getschedparam(pthread_self(),&policy,&param);
    if (s!=0)
    {
        DBGERROR("Cannot get priority for the Data thread: %s\n",strerror(errno));
    }
    DBGPRINT(DBGSERVER,2,"Before While: %" PRIu64 " us\n",timestamp());
    while(!got_message)
    {
        usleep(1000);
    }
    DBGPRINT(DBGSERVER,2,"After While: %" PRIu64 " us\n",timestamp());
    uint64_t fct_=start_of_client-initial_timestamp();
    sInfo* info = (sInfo*)information;
	int val1,pre_val1=0,val3=1,val4=0,val5=0,val6=0;
	int val2,pre_val2=0;
    int64_t tmp; 
	int ret1;
	int ret2;
    bool strated=0;
	socklen_t optlen;
	optlen = sizeof val1;
	double preTime=0;
	double delta=0;
    int64_t offset=0;
    double bias_time=0;
	int reuse = 1;
    if (setsockopt(info->sock, IPPROTO_TCP, TCP_NODELAY, &reuse, sizeof(reuse)) < 0)
    {
        DBGMARK(0,0,"ERROR: set TCP_NODELAY option %s\n",strerror(errno));
    	return((void *)0);
    }

    //Calculate time interval (unit: microsecond)
    double base_timestamp=0.0;
    uint64_t setup_time=timestamp()-fct_;
    DBGPRINT(DBGSERVER,2,"Setup Time: %" PRIu64 " us, init:%" PRIu64 " us timestamp: %" PRIu64 " us\n",fct_,initial_timestamp(),setup_time+fct_);
    bias_time=(double)setup_time/1000000.0+base_timestamp/1000000.0;
    int bias_time_scaled=(int)(bias_time*1000);
    DBGPRINT(DBGSERVER,2,"it:%d,bias_time:%lf scaled:%d\n",it,bias_time,bias_time_scaled);
    usleep(lag_time);
    uint64_t fct=0;
    while(it<info->num_lines/*& !done*/)
	{
       check=0;
       timestamp_begin(1);
       int time_scaled=(int)(1000*info->trace[it].time);
       if (bias_time_scaled>=time_scaled)
       {
           preTime=bias_time;//info->trace[it].time;
           DBGPRINT(DBGSERVER,2,"it:%d,info->trace[it].time:%lf,bias_time:%lf\n",it,info->trace[it].time,bias_time);
           it++;
           continue;
       }
       
       delta=info->trace[it].time-preTime;
        offset=(delta*1000000-fct);
        if(offset<0)
        {
	    	preTime=info->trace[it].time-(double)offset/1000000.0;
            fct=0;
            it++;
            continue;
        }
  		usleep(offset);
        val1=(int)(info->trace[it].bw*1000*bw_scale);
        val3=val1*125;
        val2=(int)((info->trace[it].minRtt+2*1000*delay_ms));
        val6=cwnd_scale;
        tmp=(val2*val1)/8000;
        val4=(int)tmp*val6;
        //BW:kbps
        ret1 = setsockopt(info->sock, IPPROTO_TCP,TCP_BBR_BWVAL, &val1, optlen);
        //TRG_DEL:us
		ret2 = setsockopt(info->sock, IPPROTO_TCP,TCP_BBR_TRGTDEL_MS, &val2, optlen);
		ret2 = setsockopt(info->sock, IPPROTO_TCP,TCP_BBR_CWNDRVGAIN, &val6, optlen);
        if (feedback==1 /*|| feedback==3*/)        
            ret2 += setsockopt(info->sock, IPPROTO_TCP,TCP_BBR_APPLY, &val5, optlen);

        if (feedback==3)
        {
            //CWND_CLAMP: Bps*sec=Bytes
            ret2 += setsockopt(info->sock, IPPROTO_TCP,TCP_CWND_CLAMP, &val4, optlen);
        }
        //MAX_PACING_RATE: Bps
        if (setsockopt(info->sock, SOL_SOCKET, SO_MAX_PACING_RATE, &val3, sizeof(val3)) < 0)
            perror("setsockopt(SO_MAX_PACING_RATE) failed");
        if(ret1<0 || ret2<0)
		{
			DBGMARK(0,0,"setsockopt: TCP_BBR ... %s (ret1:%d,ret2:%d)\n",strerror(errno),ret1,ret2);
			return((void *)0);
		}
        pre_val1=val1;
		preTime=info->trace[it].time;
        it++;
        check=1;
        fct=timestamp_end();
        fct-=offset;
    }
	return((void *)0);
}
void* DataThread(void* info)
{
	struct sched_param param;
    param.__sched_priority=sched_get_priority_max(SCHED_RR);
    int policy=SCHED_RR;
    int s = pthread_setschedparam(pthread_self(), policy, &param);
    if (s!=0)
    {
        DBGERROR("Cannot set priority (%d) for the Main: %s\n",param.__sched_priority,strerror(errno));
    }

    s = pthread_getschedparam(pthread_self(),&policy,&param);
    if (s!=0)
    {
        DBGERROR("Cannot get priority for the Data thread: %s\n",strerror(errno));
    }
    pthread_t send_msg_thread;

	int i;
	cFlow* flow = (cFlow*)info;
	int sock = flow->flowinfo.sock;
	char* src_ip;
	char write_message[BUFSIZ+1];
	char read_message[1024]={0};
	int len;
	char *savePtr;
	char* dst_addr;
	int loop;
	int remaining_size;
	tcp_info tcp_info;

	memset(write_message,1,BUFSIZ);
	write_message[BUFSIZ]='\0';

	/**
	 * Get the RQ from client : {src_add} {flowid} {size} {dst_add}
	 */
	len=recv(sock,read_message,1024,0);
	if(len<=0)
	{
		DBGMARK(DBGSERVER,1,"recv failed! \n");
		close(sock);
		return 0;
	}
	/**
	 * WE have a problem with client address:
	 * it will be replaced by OS to the local ip when it is 127.2.2.1 for instance!
	 * For Now: we send the src IP in the RQ to! :D
	 */
	src_ip=strtok_r(read_message," ",&savePtr);
	if(src_ip==NULL)
	{
		//discard message:
		DBGMARK(DBGSERVER,1,"id: %d discarding this message:%s \n",flow->flowinfo.flowid,savePtr);
		close(sock);
		return 0;
	}
	char * isstr = strtok_r(NULL," ",&savePtr);
	if(isstr==NULL)
	{
		//discard message:
		DBGMARK(DBGSERVER,1,"id: %d discarding this message:%s \n",flow->flowinfo.flowid,savePtr);
		close(sock);
		return 0;
	}
	flow->flowinfo.flowid=atoi(isstr);
	char* size_=strtok_r(NULL," ",&savePtr);
	flow->flowinfo.size=1024*atoi(size_);
    DBGPRINT(DBGSERVER,4,"%s\n",size_);
	dst_addr=strtok_r(NULL," ",&savePtr);
	if(dst_addr==NULL)
	{
		//discard message:
		DBGMARK(DBGSERVER,1,"id: %d discarding this message:%s \n",flow->flowinfo.flowid,savePtr);
		close(sock);
		return 0;
	}
	char* time_s_=strtok_r(NULL," ",&savePtr);
    char *endptr;
    start_of_client=strtoull(time_s_,&endptr,10);
	got_message=1;
    DBGPRINT(DBGSERVER,2,"Got message: %" PRIu64 " us\n",timestamp());
    flow->flowinfo.rem_size=flow->flowinfo.size;
    DBGPRINT(DBGSERVER,2,"time_rcv:%" PRIu64 " get:%s\n",start_of_client,time_s_);

	//Get detailed address
	strtok_r(src_ip,".",&savePtr);
	if(dst_addr==NULL)
	{
		//discard message:
		DBGMARK(DBGSERVER,1,"id: %d discarding this message:%s \n",flow->flowinfo.flowid,savePtr);
		close(sock);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////
	char query[150];	
	char strTmp[150];
	char strTmp2[150];

	int sockfd;	//Socket

	//////////////////////////////////////////////////////////////////////////////

	//Calculate loops. In each loop, we can send BUFSIZ (8192) bytes of data
	loop=flow->flowinfo.size*1024/BUFSIZ;
	//Calculate remaining size to be sent
	remaining_size=flow->flowinfo.size*1024-loop*BUFSIZ;
	//Send data with 8192 bytes each loop
    DBGPRINT(DBGSERVER,5,"size:%d\trem_size:%d,loop:%d\n",flow->flowinfo.size*1024,remaining_size,loop);
	for(i=0;i<loop;i++)
	{
		len=strlen(write_message);
		while(len>0)
		{
			len-=send(sock,write_message,strlen(write_message),0);
		}
	}
	//Send remaining data
	memset(write_message,1,BUFSIZ);
	write_message[remaining_size]='\0';
	len=strlen(write_message);
	DBGMARK(DBGSERVER,3,"\n");
	while(len>0)
	{
		len-=send(sock,write_message,strlen(write_message),0);
		DBGMARK(DBGSERVER,3,"\n");
	}
	DBGMARK(DBGSERVER,3,"\n");
	flow->flowinfo.rem_size=0;
    done=true;
    DBGPRINT(DBGSERVER,1,"done=true\n");
    close(sock);
    DBGPRINT(DBGSERVER,1,"done\n");
	return((void *)0);
}
