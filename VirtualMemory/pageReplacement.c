#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// 페이지 프레임 구조체
typedef struct {
    int page;
    int age;
    int reference_bit;
} PageFrame;

// 페이지 교체 알고리즘 열거형
typedef enum {
    OPTIMAL,
    FIFO,
    LRU,
    SECOND_CHANCE,
} PageReplacementAlgorithm;

//OPTIMAL 알고리즘
void optimalPageReplacement(int virtualAddresses[], int addressCount, int pageFrameCount, int pageSize) {
    PageFrame *pageFrames = (PageFrame *)malloc(pageFrameCount * sizeof(PageFrame));

    // 페이지 프레임 초기화
    for (int i = 0; i < pageFrameCount; i++) {
        pageFrames[i].page = -1; // 초기에 모든 페이지 프레임을 빈 상태로 설정
    }

    // 시뮬레이션 결과를 출력 파일에 쓰기
    FILE *outputFile = fopen("output.opt", "w");
    if (outputFile == NULL) {
        perror("Failed to open output file");
        exit(1);
    }
    fprintf(outputFile, "V.A.	Page No.	Frame No.	P.A.	Page Fault\n");

    int pageFaults = 0;
    int nextReference[pageFrameCount]; // 다음 참조 위치를 저장하기 위한 배열

    for (int i = 0; i < addressCount; i++) {
        int virtualAddress = virtualAddresses[i];
        int page = virtualAddress / pageSize;
        int found = 0;
        int emptyFrameIndex = -1;

        // 가상주소가 이미 페이지 프레임에 있는지 확인
        for (int j = 0; j < pageFrameCount; j++) {
            if (pageFrames[j].page == page) {
                found = 1;
                break;
            } else if (pageFrames[j].page == -1 && emptyFrameIndex == -1) {
        	 emptyFrameIndex = j; // 첫 번째 빈 프레임 인덱스 저장
    	    }
        }

        if (!found) {
            // 페이지 부재 (Page Fault) 발생
            pageFaults++;
            // 페이지 프레임이 비어 있는 경우
            if (emptyFrameIndex != -1) {
                pageFrames[emptyFrameIndex].page = page;
                nextReference[emptyFrameIndex] = i;
                // 시뮬레이션 결과 출력
                int physicalAddress = emptyFrameIndex * pageSize + virtualAddress % pageSize;
                fprintf(outputFile, "%d	%d		%d		%d	F\n", virtualAddress, page, emptyFrameIndex, physicalAddress);
                continue;
            }
            // 페이지 교체 로직: Optimal 알고리즘
            int replaceIndex = 0;
            int maxDistance = -1;

            for (int j = 0; j < pageFrameCount; j++) {
                int nextPageUse = addressCount; // 기본값으로 아주 큰 값을 설정

                for (int k = i + 1; k < addressCount; k++) {
                    if (virtualAddresses[k] / pageSize == pageFrames[j].page) {
                        nextPageUse = k;
                        break;
                    }
                }

                if (nextPageUse > maxDistance) {
                    maxDistance = nextPageUse;
                    replaceIndex = j;
                } else if (nextPageUse == maxDistance) {
                    // 만약 다음 참조 위치가 동일한 경우, FIFO에 따라 선택
                    if (nextReference[j] < nextReference[replaceIndex]) {
                        replaceIndex = j;
                    }
                }
            }

            // 페이지 교체
            pageFrames[replaceIndex].page = page;
            nextReference[replaceIndex] = i;

            // 시뮬레이션 결과 출력 
            int physicalAddress = replaceIndex * pageSize + virtualAddress % pageSize;
            fprintf(outputFile, "%d	%d		%d		%d	F\n", virtualAddress, pageFrames[replaceIndex].page, replaceIndex, physicalAddress);
        } else {
            // 페이지가 이미 메모리에 있는 경우
            // 시뮬레이션 결과 출력 
            int frameIndex = -1;
            for (int j = 0; j < pageFrameCount; j++) {
                if (pageFrames[j].page == page) {
                    frameIndex = j;
                    break;
                }
            }
            int physicalAddress = frameIndex * pageSize + virtualAddress % pageSize;
            fprintf(outputFile, "%d	%d		%d		%d 	H\n", virtualAddress, pageFrames[frameIndex].page, frameIndex, physicalAddress);
        }
    }

    // Page Faults 수를 출력 파일에 쓰기
    fprintf(outputFile, "Page Faults: %d\n", pageFaults);

    fclose(outputFile);
    free(pageFrames);
}


//FIFO 알고리즘
void fifoPageReplacement(int virtualAddresses[], int addressCount, int pageFrameCount, int pageSize) {
    PageFrame *pageFrames = (PageFrame *)malloc(pageFrameCount * sizeof(PageFrame));

    // 페이지 프레임 초기화
    for (int i = 0; i < pageFrameCount; i++) {
        pageFrames[i].page = -1; // 초기에 모든 페이지 프레임을 빈 상태로 설정
    }

    // 시뮬레이션 결과를 출력 파일에 쓰기
    FILE *outputFile = fopen("output.fifo", "w");
    if (outputFile == NULL) {
        perror("Failed to open output file");
        exit(1);
    }

    fprintf(outputFile, "V.A.	Page No.	Frame No.	P.A.	Page Fault\n");
    int pageFaults = 0;
    int front = 0; // 가장 먼저 들어온 페이지를 가리키는 변수

    for (int i = 0; i < addressCount; i++) {
        int virtualAddress = virtualAddresses[i];
        int page = virtualAddress / pageSize;
        int found = 0;

        // 가상주소가 이미 페이지 프레임에 있는지 확인
        for (int j = 0; j < pageFrameCount; j++) {
            if (pageFrames[j].page == page) {
                found = 1;
                break;
            }
        }

        // 페이지 부재 (Page Fault) 발생
        if (!found) {
            pageFaults++;            
            pageFrames[front].page = page;
            int physicalAddress = front * pageSize + virtualAddress % pageSize;
            fprintf(outputFile, "%d	%d		%d		%d	F\n", virtualAddress, page, front, physicalAddress);
            front = (front + 1) % pageFrameCount;
        } else {
            // 페이지가 이미 메모리에 있는 경우
            int frameIndex = -1;
            for (int j = 0; j < pageFrameCount; j++) {
                if (pageFrames[j].page == page) {
                    frameIndex = j;
                    break;
                }
            }
            int physicalAddress = frameIndex * pageSize + virtualAddress % pageSize;
            fprintf(outputFile, "%d	%d		%d		%d	H\n", virtualAddress, page, frameIndex, physicalAddress);
        }
    }

    // Page Faults 수를 출력 파일에 쓰기
    fprintf(outputFile, "Page Faults: %d\n", pageFaults);

    fclose(outputFile);
    free(pageFrames);
}



//LRU 알고리즘
void lruPageReplacement(int virtualAddresses[], int addressCount, int pageFrameCount, int pageSize) {

    PageFrame *pageFrames = (PageFrame *)malloc(pageFrameCount * sizeof(PageFrame));

    // 페이지 프레임 초기화
    for (int i = 0; i < pageFrameCount; i++) {
        pageFrames[i].page = -1; // 초기에 모든 페이지 프레임을 빈 상태로 설정
        pageFrames[i].age = 0;    // 각 페이지의 나이 초기화  
    }

    // 시뮬레이션 결과를 출력 파일에 쓰기
    FILE *outputFile = fopen("output.lru", "w");
    if (outputFile == NULL) {
        perror("Failed to open output file");
        exit(1);
    }
    
    fprintf(outputFile, "V.A.	Page No.	Frame No.	P.A.	Page Fault\n");
    int pageFaults = 0;
    int nextReference[pageFrameCount]; // 다음 참조 위치를 저장하기 위한 배열

    for (int i = 0; i < addressCount; i++) {
        int virtualAddress = virtualAddresses[i];
        int page = virtualAddress / pageSize;
        int found = 0;
        int emptyFrameIndex = -1;
       
        // 가상주소가 이미 페이지 프레임에 있는지 확인
        for (int j = 0; j < pageFrameCount; j++) {
            if (pageFrames[j].page == page) {
                found = 1;
                // 페이지가 참조될 때마다 나이를 초기화
                pageFrames[j].age = 0;
                int physicalAddress = j * pageSize + virtualAddress % pageSize;
                fprintf(outputFile, "%d	%d		%d		%d	H\n", virtualAddress, pageFrames[j].page, j, physicalAddress);
                break;
            } else if (pageFrames[j].page == -1 && emptyFrameIndex == -1) {
        	 emptyFrameIndex = j; // 첫 번째 빈 프레임 인덱스 저장
        	 break;
    	    }
        }

        // 페이지 부재 (Page Fault) 발생
        if (!found) {
            pageFaults++;

            // 페이지 프레임이 비어 있는 경우
            if (emptyFrameIndex != -1) {
                pageFrames[emptyFrameIndex].page = page;
                pageFrames[emptyFrameIndex].age = 0;
                nextReference[emptyFrameIndex] = i;
                // 시뮬레이션 결과 출력
                int physicalAddress = emptyFrameIndex * pageSize + virtualAddress % pageSize;
                fprintf(outputFile, "%d	%d		%d		%d	F\n", virtualAddress, page, emptyFrameIndex, physicalAddress);
                continue;
            }

            // 페이지 교체가 필요한 경우
            
            // 가장 오래된 페이지를 찾아서 교체
            int replaceIndex = 0;
            int maxAge = -1;

            for (int j = 0; j < pageFrameCount; j++) {
                if (pageFrames[j].age > maxAge) {
                    maxAge = pageFrames[j].age;
                    replaceIndex = j;
                }
            }
                
            pageFrames[replaceIndex].page = page;
            pageFrames[replaceIndex].age = 0;
            nextReference[replaceIndex] = i;
            int physicalAddress = replaceIndex * pageSize + virtualAddress % pageSize;
            fprintf(outputFile, "%d	%d		%d		%d	F\n", virtualAddress, page, replaceIndex, physicalAddress);
            
        } 

        // 모든 페이지의 나이를 증가
        for (int j = 0; j < pageFrameCount; j++) {
            if(pageFrames[j].page != -1)
                pageFrames[j].age++;
        }
    }

    // Page Faults 수를 출력 파일에 쓰기
    fprintf(outputFile, "Page Faults: %d\n", pageFaults);

    fclose(outputFile);
    free(pageFrames);
}


//SECOND-CHANCE 알고리즘
void secondChancePageReplacement(int virtualAddresses[], int addressCount, int pageFrameCount, int pageSize) {
    PageFrame *pageFrames = (PageFrame *)malloc(pageFrameCount * sizeof(PageFrame));

    // 페이지 프레임 초기화
    for (int i = 0; i < pageFrameCount; i++) {
        pageFrames[i].page = -1; // 초기에 모든 페이지 프레임을 빈 상태로 설정
        pageFrames[i].reference_bit = 0; // 참조 비트 초기화
    }

    // 시뮬레이션 결과를 출력 파일에 쓰기
    FILE *outputFile = fopen("output.sc", "w");
    if (outputFile == NULL) {
        perror("Failed to open output file");
        exit(1);
    }
    
    fprintf(outputFile, "V.A.	Page No.	Frame No.	P.A.	Page Fault\n");
    int pageFaults = 0;
    int hand = 0; // Clock 알고리즘의 hand 포인터

    for (int i = 0; i < addressCount; i++) {
        int virtualAddress = virtualAddresses[i];
        int page = virtualAddress / pageSize;
        int found = 0;
        int emptyFrameIndex = -1;
        // 가상주소가 이미 페이지 프레임에 있는지 확인
        for (int j = 0; j < pageFrameCount; j++) {
            if (pageFrames[j].page == page) {
                found = 1;
                pageFrames[j].reference_bit = 1; // 참조 비트를 1로 설정
                // 시뮬레이션 결과 출력
                int physicalAddress = j * pageSize + virtualAddress % pageSize;
                fprintf(outputFile, "%d	%d		%d		%d	H\n", virtualAddress, pageFrames[j].page, j, physicalAddress);
                break;
            } else if (pageFrames[j].page == -1 && emptyFrameIndex == -1) {
        	 emptyFrameIndex = j; // 첫 번째 빈 프레임 인덱스 저장
        	 break;
    	    }
        }

        // 페이지 부재 (Page Fault) 발생
        if (!found) {
            pageFaults++;
            // 페이지 프레임이 비어 있는 경우
            if (emptyFrameIndex != -1) {
                pageFrames[emptyFrameIndex].page = page;
                pageFrames[emptyFrameIndex].reference_bit = 0;	// 참조 비트를 0으로 설정
                // 시뮬레이션 결과 출력
                int physicalAddress = emptyFrameIndex * pageSize + virtualAddress % pageSize;
                fprintf(outputFile, "%d	%d		%d		%d	F\n", virtualAddress, page, emptyFrameIndex, physicalAddress);
                continue;
            }
            // 모든 페이지 프레임이 차있는 경우
            
            // Second-Chance (Clock) 알고리즘
            while (1) {
                if (pageFrames[hand].reference_bit == 1) {
                    // 참조 비트가 1이면 0으로 변경하고 다음 페이지로 이동
                    pageFrames[hand].reference_bit = 0;
                    hand = (hand + 1) % pageFrameCount;
                } else {
                    // 참조 비트가 0이면 해당 페이지를 교체
                    pageFrames[hand].page = page;                  
                    int physicalAddress = hand * pageSize + virtualAddress % pageSize;
                    fprintf(outputFile, "%d	%d		%d		%d	F\n", virtualAddress, page, hand, physicalAddress);
                    hand = (hand + 1) % pageFrameCount;
                    break;
                }
            }
   
            
        }

    }

    // Page Faults 수를 출력 파일에 쓰기
    fprintf(outputFile, "Page Faults: %d\n", pageFaults);

    fclose(outputFile);
    free(pageFrames);
}



// 페이지 교체 알고리즘을 구현하는 함수 원형
void pageReplacementSimulation(int virtualAddresses[], int addressCount, int pageFrameCount, int pageSize, PageReplacementAlgorithm algorithm){
    switch(algorithm){
        case 1:
            optimalPageReplacement(virtualAddresses, addressCount, pageFrameCount, pageSize);
            break;
        case 2:
            fifoPageReplacement(virtualAddresses, addressCount, pageFrameCount, pageSize);
            break;
        case 3:
            lruPageReplacement(virtualAddresses, addressCount, pageFrameCount, pageSize);
            break;
        case 4:
            secondChancePageReplacement(virtualAddresses, addressCount, pageFrameCount, pageSize);
            break;
            
  }          
}

// 사용자로부터 선택한 옵션을 반환하는 함수
int getUserChoice(int min, int max, const char *prompt) {
    int choice;
    do {
        printf("%s", prompt);
        scanf("%d", &choice);
    } while (choice < min || choice > max);
    return choice;
}

int main(int argc, char *argv[]) {
    int addressLength, pageSize, physicalMemorySize;
    PageReplacementAlgorithm algorithm;
    char inputFilename[256];

    printf("A. Simulation에 사용할 가상주소 길이를 선택하시오 (1. 18bits    2. 19bits    3. 20bits): ");
    addressLength = getUserChoice(1, 3, "") + 17;

    printf("B. Simulation에 사용할 페이지(프레임)의 크기를 선택하시오 (1. 1KB    2. 2KB    3. 4KB): ");
    pageSize = 1 << (getUserChoice(1, 3, "") + 9);

    printf("C. Simulation에 사용할 물리메모리의 크기를 선택하시오 (1. 32KB    2. 64KB): ");
    physicalMemorySize = 1 << (getUserChoice(1, 2, "") + 14);

    printf("D. Simulation에 적용할 Page Replacement 알고리즘을 선택하시오 \n (1. Optimal    2. FIFO    3. LRU    4. Second-Chance): ");
    algorithm = getUserChoice(1, 4, "");

    printf("E. 가상주소 스트링 입력방식 선택하시오 (1. Input.in 자동 생성    2. 기존 파일 사용): ");
    int inputOption = getUserChoice(1, 2, "");

    if (inputOption == 1) {
    	// E 단계에서 1을 선택한 경우, 자동 생성
    	// 랜덤 가상주소를 생성하고 파일에 저장
    	FILE *file = fopen("input.in", "w"); // "input.in" 파일을 쓰기 모드로 열기

    	if (file == NULL) {
        	perror("Failed to open file");
        	exit(1);
    	}

    	// 랜덤 시드 초기화 (시드 값을 다르게 설정하면 매번 다른 랜덤 가상주소 생성)
    	srand(time(NULL));

    	// 가상주소 스트링 생성 및 파일에 쓰기
    	for (int i = 0; i < 5000; i++) {
        	int virtualAddress = rand() % (1 << addressLength); // 랜덤 가상주소 생성
        	fprintf(file, "%d\n", virtualAddress); // 파일에 가상주소 쓰기
    	}

    	fclose(file); // 파일 닫기

    	// 파일을 입력으로 사용하기 위해 파일 이름을 설정
    	strcpy(inputFilename, "input.in");

    	
    } 
    else if (inputOption == 2) {
        // E 단계에서 2를 선택한 경우, 파일 사용
    	printf("F. 입력 파일 이름을 입력하시오: ");
    	scanf("%s", inputFilename);
    	
    }
    // 입력 파일 열기
    FILE *inputFile = fopen(inputFilename, "r");
    if (inputFile == NULL) {
    	perror("Failed to open input file");
    	exit(1);
    }

    // 가상주소 배열을 생성하고 입력 파일에서 읽어오기
    int virtualAddresses[5000];
    int addressCount = 0;

    while (fscanf(inputFile, "%d", &virtualAddresses[addressCount]) != EOF) {
    	addressCount++;
    }

    fclose(inputFile); // 입력 파일 닫기

    // Optimal 알고리즘을 사용하여 페이지 교체 시뮬레이션 실행
    pageReplacementSimulation(virtualAddresses, addressCount, physicalMemorySize / pageSize, pageSize, algorithm);
	
    // 나머지 시뮬레이션과 결과 출력 부분은 그대로 유지

    return 0;
}

