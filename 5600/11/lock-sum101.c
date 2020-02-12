#include <stdio.h>
#include <assert.h>

#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <semaphore.h>

void*
malloc_shared(size_t size)
{
    return mmap(0, size, PROT_READ|PROT_WRITE,
                MAP_SHARED|MAP_ANONYMOUS, -1, 0);
}

void
free_shared(void* ptr, size_t size)
{
    munmap(ptr, size);
}

int
main(int _ac, char* _av[])
{
    // One billion
    const long TOP = 1000000000;
    const long NPP = TOP / 10;

    long* sum = malloc_shared(sizeof(long));
    sem_t* lock = malloc_shared(sizeof(sem_t));
    sem_init(lock, 1, 1);

    pid_t kids[10];

    for (long pp = 0; pp < 10; ++pp) {
        if ((kids[pp] = fork())) {
            // parent, do nothing
        }
        else {
            long i0 = NPP*pp;
            long i1 = i0 + NPP;

            for (long ii = i0; ii < i1; ++ii) {
                if (ii % 101 == 0) {
                    sem_wait(lock);
                    *sum += ii;
                    sem_post(lock);
                }
            }

            free_shared(sum, sizeof(long));
            exit(0);
        }
    }

    for (long pp = 0; pp < 10; ++pp) {
        waitpid(kids[pp], 0, 0);
    }

    printf("Sum = %ld\n", *sum);

    free_shared(sum, sizeof(long));
    return 0;
}