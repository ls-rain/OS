#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

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

void printExecutionTime(pid_t pid, struct timeval start, struct timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds / 1e6;

    struct tm start_tm;
    struct tm end_tm;

    localtime_r(&start.tv_sec, &start_tm);
    localtime_r(&end.tv_sec, &end_tm);
    printf("PID: %d | Start Time: %02d:%02d:%02d.%06ld | End Time: %02d:%02d:%02d.%06ld, Elapsed Time: %.6f seconds\n", pid, start_tm.tm_hour, start_tm.tm_min, start_tm.tm_sec, start.tv_usec, end_tm.tm_hour, end_tm.tm_min, end_tm.tm_sec, end.tv_usec, elapsed);
}

int main(){
	for (int i = 0; i < NUM_PROCESSES; i++) {
        	pid_t child_pid = fork(); // 새로운 프로세스 생성

        	if (child_pid == 0) {
           	 // 자식 프로세스
           	 	struct timeval start_time, end_time;
            		gettimeofday(&start_time, NULL);
			initializeArrays();
			matrixMultiplication(); // 배열 곱셈 연산 함수 호출
			gettimeofday(&end_time, NULL);
			printExecutionTime(getpid(), start_time, end_time);
            	exit(0); // 자식 프로세스 종료
        	} else if (child_pid < 0) {
            		// 에러 처리
            		perror("Fork failed");
            		exit(1);
        	}
    	}

    	// 부모 프로세스
    	for (int i = 0; i < NUM_PROCESSES; i++) {
        	wait(NULL); // 모든 자식 프로세스의 종료를 기다림
    	}
    	
    	return 0;
}	
