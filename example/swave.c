/* compile using  "gcc -o swave swave.c -lrt -Wall"  */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/io.h>

#define PORT 0x378
#define NSEC_PER_SEC    1000000000

/* using clock_nanosleep of librt */
extern int clock_nanosleep(clockid_t __clock_id, int __flags,
      __const struct timespec *__req,
      struct timespec *__rem);

/* the struct timespec consists of nanoseconds
 * and seconds. if the nanoseconds are getting
 * bigger than 1000000000 (= 1 second) the
 * variable containing seconds has to be
 * incremented and the nanoseconds decremented
 * by 1000000000.
 */
static inline void tsnorm(struct timespec *ts)
{
   while (ts->tv_nsec >= NSEC_PER_SEC) {
      ts->tv_nsec -= NSEC_PER_SEC;
      ts->tv_sec++;
   }
}

/* increment counter and write to parallelport */
void out()
{
   static unsigned char state=0;
   outb(state++,PORT);
   //add by xyj
   printf("%d\n", state-1);
}

int main(int argc,char** argv)
{
   struct timespec t;
   struct sched_param param;
   /* default interval = 50000ns = 50us
    * cycle duration = 100us
    */
   int interval=50000;

   /* set permissions of parallelport */
   ioperm(PORT,1,1);

   if(argc>=2 && atoi(argv[1])>0)
   {
      printf("using realtime, priority: %d\n",atoi(argv[1]));
      param.sched_priority = atoi(argv[1]);
      /* enable realtime fifo scheduling */
      if(sched_setscheduler(0, SCHED_FIFO, &param)==-1){
         perror("sched_setscheduler failed");
         exit(-1);
      }
   }
   if(argc>=3)
      interval=atoi(argv[2]);

   /* get current time */
   clock_gettime(0,&t);
   /* start after one second */
   t.tv_sec++;
   while(1){
      /* wait untill next shot */
      clock_nanosleep(0, TIMER_ABSTIME, &t, NULL);
      /* do the stuff */
      out();
      /* calculate next shot */
      t.tv_nsec+=interval;
      tsnorm(&t);
   }
   return 0;
}
