#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

#define NUM_PROCESSES 21
#define ARRAY_SIZE 100

int A[ARRAY_SIZE][ARRAY_SIZE];
int B[ARRAY_SIZE][ARRAY_SIZE];
int result[ARRAY_SIZE][ARRAY_SIZE];

void initializeArrays() {
    int A[ARRAY_SIZE][ARRAY_SIZE],B[ARRAY_SIZE][ARRAY_SIZE],result[ARRAY_SIZE][ARRAY_SIZE] = {0};
    for(int i = 0; i<ARRAY_SIZE; i++){
    	for(int j = 0; j<ARRAY_SIZE; j++){
    		A[i][j] = i+j;
    		B[i][j] = i*j;
    	}
    }
}

void matrixMultiplication() {
    int count = 0;
    while (count < 100) {
        for (int k = 0; k < ARRAY_SIZE; k++) {
            for (int i = 0; i < ARRAY_SIZE; i++) {
                for (int j = 0; j < ARRAY_SIZE; j++) {
                    result[k][j] += A[k][i] * B[i][j];
                }
            }
        }
        count++;
    }
}

void printExecutionTime(pid_t pid, int nice_val, struct timeval start, struct timeval end, int policy) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds / 1e6;

    struct tm start_tm;
    struct tm end_tm;

    localtime_r(&start.tv_sec, &start_tm);
    localtime_r(&end.tv_sec, &end_tm);

    if (nice_val != -1) {
        printf("PID: %d | NICE: %d | Start Time: %02d:%02d:%02d.%06ld | End Time: %02d:%02d:%02d.%06ld | Elapsed Time: %.6f seconds\n",
               pid, nice_val,
               start_tm.tm_hour, start_tm.tm_min, start_tm.tm_sec, start.tv_usec,
               end_tm.tm_hour, end_tm.tm_min, end_tm.tm_sec, end.tv_usec, elapsed);
    } else {
        printf("PID: %d | Start Time: %02d:%02d:%02d.%06ld | End Time: %02d:%02d:%02d.%06ld, Elapsed Time: %.6f seconds\n",
               pid,
               start_tm.tm_hour, start_tm.tm_min, start_tm.tm_sec, start.tv_usec,
               end_tm.tm_hour, end_tm.tm_min, end_tm.tm_sec, end.tv_usec, elapsed);
    }

}

int main() {
    int policy;
    int pipe_fd[2];
    struct sched_param param;
    cpu_set_t cpuset;
    double total_elapsed_time = 0.0;

    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset); // Restrict to CPU core 0

    printf("Input the scheduling policy to apply:\n");
    printf("1. CFS_DEFAULT\n");
    printf("2. CFS_NICE\n");
    printf("3. RT_FIFO\n");
    printf("4. RT_RR\n");

    scanf("%d", &policy);

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    pid_t child_pids[NUM_PROCESSES];
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            // 자식 프로세스
            close(pipe_fd[0]);
            struct timeval start_time, end_time;
            gettimeofday(&start_time, NULL);
            if (policy == 1) {
                // Set CFS_DEFAULT
                if (setpriority(PRIO_PROCESS, 0, 0) == -1) {
                    perror("setpriority");
                    exit(1);
                }
            } else if (policy == 2) {
                // Set CFS_NICE 
                if (i < 7) {
                    if (setpriority(PRIO_PROCESS, 0, -20) == -1) {
                        perror("setpriority");
                        exit(1);
                    }
                } else if (i < 14) {
                    if (setpriority(PRIO_PROCESS, 0, 0) == -1) {
                        perror("setpriority");
                        exit(1);
                    }
                } else {
                    if (setpriority(PRIO_PROCESS, 0, 19) == -1) {
                        perror("setpriority");
                        exit(1);
                    }
                }
            } else if (policy == 3) {
    		// Set RT_FIFO 
    		param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    		if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
       	    perror("sched_setscheduler");
        	    exit(1);
    		}
	   }	

	       else if (policy == 4) {
                // Set RT_RR
                param.sched_priority = sched_get_priority_max(SCHED_RR); 
                if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
                    perror("sched_setscheduler");
                    exit(1);
                }
            }

            // CPU코어 0번 하나만 쓰도록 제한
            if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) { 
                perror("sched_setaffinity");
                exit(1);
            }
	    initializeArrays();
            
            matrixMultiplication();
            gettimeofday(&end_time, NULL);
                     
            if (policy == 3 || policy == 4) {
                printExecutionTime(getpid(), -1, start_time, end_time, policy);
            } else {
                int nice_val = getpriority(PRIO_PROCESS, 0);
                printExecutionTime(getpid(), nice_val, start_time, end_time, policy);
            }

            double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                        (end_time.tv_usec - start_time.tv_usec) / 1e6;
             
            write(pipe_fd[1], &elapsed, sizeof(elapsed));
            close(pipe_fd[1]);

            exit(0);
        }
    }
    
    if (policy == 1) {
        printf("Scheduling Policy: CFS_DEFAULT | ");
    } else if (policy == 2) {
        printf("Scheduling Policy: CFS_NICE | ");
    } else if (policy == 3) {
        printf("Scheduling Policy: RT_FIFO | ");
    } else if (policy == 4) {
        printf("Scheduling Policy: RT_RR | ");
    }
	if (policy == 4) {
        	struct timespec quantum;
        	if (sched_rr_get_interval(0, &quantum) == 0) {
            	printf("Time Quantum: %ld ms | ", quantum.tv_sec * 1000+quantum.tv_nsec / 1000000);
        }
    }
    close(pipe_fd[1]); 
    double child_elapsed_time;
    while (read(pipe_fd[0], &child_elapsed_time, sizeof(child_elapsed_time)) > 0) {
        total_elapsed_time += child_elapsed_time;
    }
    close(pipe_fd[0]); 

    printf("Average Elapsed Time: %.6f seconds\n", total_elapsed_time / NUM_PROCESSES);

    return 0;
}


