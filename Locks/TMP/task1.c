#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}



void lockReader (char msg, int lck)
{
        //int     ret;

        //kprintf ("  %s: to acquire lock\n", msg);
        

        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %c: acquired lock\n", msg);
        sleep1000(1);
        //sleep(1);
        int i=0;
        for ( i = 0; i < 25; ++i)
        {
            /* code */
            //int j;
            //for (j = 0; j < 100000; ++j);
            kprintf("%c ",msg);
        }
        //kprintf ("\n  %s: to release lock\n", msg);
        releaseall (1, lck);
        kprintf ("  %c: released lock\n", msg);
}

void reader (char msg, int lck)
{
        //int     ret;

        //kprintf ("  %s: to acquire lock\n", msg);
        //lock (lck, READ, DEFAULT_LOCK_PRIO);
       // kprintf ("  %s: acquired lock\n", msg);
        //sleep(1);
        sleep1000(1);

        int i=0;
        for ( i = 0; i < 25; ++i)
        {
            /* code */
            //int j;
            //for (j = 0; j < 100000; ++j);
            kprintf("%c ",msg);
        }
        //kprintf ("\n  %s: to release lock\n", msg);
        //releaseall (1, lck);
        //kprintf ("  %s: released lock\n", msg);
}


void lockWriter (char msg, int lck)
{
        //kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %c: acquired lock, \n", msg);
        sleep1000(1);

        //sleep (1);
        //kprintf ("  %s: to release lock\n", msg);
        int i=0;
        for ( i = 0; i < 25; ++i)
        {
            /* code */
            //int j;
            //for (j = 0; j < 100000; ++j);

            kprintf("%c ",msg);
        }
        releaseall (1, lck);
        kprintf ("  %c: released lock\n", msg);
}

void prioInversionWithLocks ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Lock failed");

        rd1 = create(lockWriter, 2000, 20, "writer3", 2, 'A', lck);
        rd2 = create(reader, 2000, 30, "reader3", 2, 'B', lck);
        wr1 = create(lockWriter, 2000, 40, "writer3", 2,'C', lck);

        
        //sleep (1);

        kprintf("- Writer A. A(prio 20) granted the lock\n");
        resume(rd1);
        kprintf("Writer A Priority before Writer C: %d\n",getprio(rd1));
        kprintf("-start writer C.(prio 40) blocked on the lock\n");
        resume(wr1);
        kprintf(" A Priority after Writer C in wait: %d\n",getprio(rd1));
        kprintf("-start  B, then sleep 5s. B(prio 30) normally running\n");
        resume (rd2);
        sleep (5);
}

void semReader (char msg, int sem)
{
        //int ret;
        

        kprintf ("  %c: to acquire semaphore\n", msg);
        wait(sem);

        kprintf ("  %c: acquired semaphore\n", msg);
        sleep1000(1);
        //sleep(1);
        int i=0;
        for ( i = 0; i < 25; ++i)
        {
            /* code */
            //int j;
            //for (j = 0; j < 100000; ++j);

            kprintf("%c ",msg);
        }
        //kprintf ("\n  %s: to release semaphore\n", msg);
        signal(sem);
        kprintf ("  %c: released semaphore\n", msg);
}

void semWriter (char msg, int sem)
{


        kprintf ("  %c: to acquire semaphore\n", msg);
        wait(sem);
        sleep1000(1);
        kprintf ("  %c: acquired semaphore, \n", msg);
        //sleep (1);
        //kprintf ("  %s: to release semaphore\n", msg);
        int i=0;
        for ( i = 0; i < 25; ++i)
        {
            /* code */
            //int j;
            //for (j = 0; j < 100000; ++j);

            kprintf("%c ",msg);
        }
        signal(sem);
        kprintf ("  %c: released semaphore\n", msg);
}

void prioInversionWithSem ()
{
        int     sem;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        sem  = screate (1);
        assert (sem != SYSERR, "Sem failed");

        rd1 = create(semWriter, 2000, 20, "writer3", 2, 'A', sem);
        rd2 = create(reader, 2000, 30, "reader3", 2,'B', sem);
        wr1 = create(semWriter, 2000, 40, "writer3", 2, 'C', sem);

        
        //sleep (1);

        kprintf("-start  A.  A(prio 20) granted the semaphore\n");
        resume(rd1);
        kprintf(" A Priority before Writer C: %d\n",getprio(rd1));
        kprintf("-start writer(prio 40) C blocked on the semaphore\n");
        resume(wr1);
        kprintf(" A Priority after Writer C in wait: %d\n",getprio(rd1));
        kprintf("-start  B.  B(prio 30) normally running\n");
        resume (rd2);
        sleep (5);
}

int main(){
    kprintf("Priority Inversion with Locks\n");
    prioInversionWithLocks();
    kprintf("Completed test with locks.\n\n");
    kprintf("Priority Inversion with Semaphores\n");
    prioInversionWithSem();
    kprintf("Completed test with Semaphores\n");
    shutdown();
}