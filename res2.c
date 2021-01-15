
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "p3150026-p3150117-p3150148-res2.h"



int totalIncome, transactionCounter, totalWaitingTime, totalServiceTime, **seats, seatCounter, tilefonites, tamies, rows;
int transactions[4] = {0,0,0,0};
unsigned int seedGl;

//mutexes
pthread_mutex_t lockThl, lockCash, totInc, trCount, totWaitT, totServT, plan, pr, tran;
pthread_cond_t condThl = PTHREAD_COND_INITIALIZER;
pthread_cond_t condCash = PTHREAD_COND_INITIALIZER;



void Outputs(int customers){

	int i,j;	
	char zone;
	
	zone = 'A';

	for (i = 0; i < rows; i++) {

		if (i == N_zoneA) zone = 'B';
		if (i == N_zoneA + N_zoneB) zone = 'C';
		
		for(j = 0; j < N_seat; j++) {
			if(seats[i][j]==-1){
				printf("Zone %c / Row %d / Seat %d / Empty\n", zone, i+1, j+1);	
			}
			else{ 
				printf("Zone %c / Row %d / Seat %d / Customer %d\n", zone, i+1, j+1, seats[i][j] );
			}
		}
	}

	int total = transactions[0] + transactions[1] + transactions[2] + transactions[3];

	printf("\nTotal Income: %d euros.\n", totalIncome);

	printf("\nTransactions Percentage for Zone A: %.2f\n", (double)transactions[0]/(double)total*100);

	printf("Transactions Percentage for Zone B: %.2f\n", (double)transactions[1]/(double)total*100);

	printf("Transactions Percentage for Zone C: %.2f\n", (double)transactions[2]/(double)total*100);

	printf("Uncompleted Transactions Percentage: %.2f\n", (double)transactions[3]/(double)total*100);

	printf("\nAverage waiting time: %f.\n", (double)totalWaitingTime/(double)customers);

	printf("\nAverage service time: %f.\n", (double)totalServiceTime/(double)customers);

	printf("\n");


}





void *Reservation(void *threadId) {

	struct timespec start,mid1,mid2,mid3,end;
	int start_time, mid1_time, mid2_time, mid3_time, end_time;
	clock_gettime(CLOCK_REALTIME,&start); 
	start_time = start.tv_sec;
	int *tid;
	tid = (int *)threadId;
	int rc;
	int i;
	//printf("I am the customer with number %d!\n", *tid);

	int randomSeats = rand_r(&seedGl) % (N_seatHigh - N_seatLow + 1) + N_seatLow;
	int randomThlTime = rand_r(&seedGl) % (t_seatHigh - t_seatLow + 1) + t_seatLow;
	int randomCashTime = rand_r(&seedGl) % (t_cashHigh - t_cashLow + 1) + t_cashLow;
	int randomSuc = rand_r(&seedGl) % 100;
	int randomZone = rand_r(&seedGl) % 100;

	rc = pthread_mutex_lock(&lockThl);
	
	while(tilefonites==0){

		//printf("Customer %d, no available tilefonitis. Blocked...\n",*tid);
		rc = pthread_cond_wait(&condThl,&lockThl);
	
	}

	clock_gettime(CLOCK_REALTIME,&mid1);
	mid1_time = mid1.tv_sec;

		
	//printf("Customer %d getting service\n",*tid);
	tilefonites--;
	rc = pthread_mutex_unlock(&lockThl);

	rc = pthread_mutex_lock(&totWaitT);
	totalWaitingTime += (mid1_time-start_time);
	rc = pthread_mutex_unlock(&totWaitT);
	
	int zone, start_row, end_row, zone_cost;
	

	if (randomZone<P_zoneA){ 
		zone = 1;
		start_row = 0;
		end_row = N_zoneA;
		zone_cost = C_zoneA;
	}
	else if (randomZone<P_zoneA+P_zoneB){
		zone = 2;
		start_row = N_zoneA;
		end_row = N_zoneA + N_zoneB;
		zone_cost = C_zoneB;
	}	
	else{
		zone = 3;
		start_row = N_zoneA + N_zoneB;
		end_row = N_zoneA + N_zoneB + N_zoneC;
		zone_cost = C_zoneC;
	}
	
	
	sleep(randomThlTime);

	rc = pthread_mutex_lock(&plan);

	bool tmp = false;

	int space, row = -1, column;

	if (seatCounter == N_seat*rows){
		printf("Πελάτης %d:\nΗ κράτηση ματαιώθηκε γιατί το θέατρο είναι γεμάτο.\n", *tid);
		tmp = true;
	}

	else{		
		for (i=start_row;i<end_row;i++){
			space=0;		
			for (int j=0;j<N_seat;j++){
				if (seats[i][j]==-1){
					space++;
				}
				else space=0;
				if (space==randomSeats){
					row=i;
					column=j;
					i=end_row;j=N_seat;break;			
				}		
			}					
		}	
	
		if (row==-1){
			printf("Πελάτης %d:\nΗ κράτηση ματαιώθηκε γιατί δεν υπάρχουν διαθέσιμες συνεχόμενες θέσεις στη συγκεκριμένη ζώνη.\n", *tid);
			tmp = true;	
		}
		else{
			for (i=column-randomSeats+1;i<=column;i++){
				seats[row][i] = *tid;	
			}
			seatCounter += randomSeats;
		}
	}

	

	rc = pthread_mutex_unlock(&plan);				
	rc = pthread_mutex_lock(&lockThl);
	//printf("Customer %d got service\n",*tid);
	tilefonites++;
	rc = pthread_cond_signal(&condThl);
	rc = pthread_mutex_unlock(&lockThl);

	if (tmp){
		rc = pthread_mutex_lock(&tran);
		transactions[3]++;
		rc = pthread_mutex_unlock(&tran);
		
		clock_gettime(CLOCK_REALTIME,&end);
		end_time = end.tv_sec;

		rc = pthread_mutex_lock(&totServT);
		totalServiceTime += (end_time-start_time);
		rc = pthread_mutex_unlock(&totServT);		

		pthread_exit(tid);
	}

	
	clock_gettime(CLOCK_REALTIME,&mid2);
	mid2_time = mid2.tv_sec;


	int cost = randomSeats*zone_cost;
	
	rc = pthread_mutex_lock(&lockCash);
	
	while(tamies==0){

		//printf("Customer %d, no available tamias. Blocked...\n",*tid);
		rc = pthread_cond_wait(&condCash,&lockCash);
	
	}
		
	//printf("Customer %d getting service\n",*tid);
	tamies--;
	rc = pthread_mutex_unlock(&lockCash);

	clock_gettime(CLOCK_REALTIME,&mid3);
	mid3_time = mid3.tv_sec;

	rc = pthread_mutex_lock(&totWaitT);
	totalWaitingTime += (mid3_time-mid2_time);
	rc = pthread_mutex_unlock(&totWaitT);
	
	
	sleep(randomCashTime);

	if (randomSuc > P_cardsuccess-1){
		printf("Πελάτης %d:\nΗ κράτηση ματαιώθηκε γιατί η συναλλαγή με την πιστωτική κάρτα δεν έγινε αποδεκτή.\n", *tid);
		rc = pthread_mutex_lock(&plan);
		for (i=column-randomSeats+1;i<=column;i++){
				seats[row][i] = -1;	
		}
		seatCounter -= randomSeats;
		rc = pthread_mutex_unlock(&plan);		
		rc = pthread_mutex_lock(&lockCash);
		//printf("Customer %d got service\n",*tid);
		tamies++;
		rc = pthread_cond_signal(&condCash);
		rc = pthread_mutex_unlock(&lockCash);

		rc = pthread_mutex_lock(&tran);
		transactions[3]++;
		rc = pthread_mutex_unlock(&tran);

		clock_gettime(CLOCK_REALTIME,&end);
		end_time = end.tv_sec;

		rc = pthread_mutex_lock(&totServT);
		totalServiceTime += (end_time-start_time);
		rc = pthread_mutex_unlock(&totServT);	

		pthread_exit(tid);
	}
	
	
	rc = pthread_mutex_lock(&totInc);
	totalIncome += cost;
	rc = pthread_mutex_unlock(&totInc);

	rc = pthread_mutex_lock(&trCount);
	int count = transactionCounter;
	transactionCounter++;
	rc = pthread_mutex_unlock(&trCount);
	
	rc = pthread_mutex_lock(&pr);
	printf("Πελάτης %d:\nΗ κράτηση ολοκληρώθηκε επιτυχώς.\nΟ αριθμός συναλλαγής είναι <%d>", *tid, count);
	printf(", οι θέσεις σας στη Ζώνη %d είναι οι <Σ%d-Θ%d", zone, row+1, column-randomSeats+2);
	for(i=column-randomSeats+3;i<=column+1;i++){
		printf(", Σ%d-Θ%d", row+1, i);
	}				
	printf("> και το κόστος της συναλλαγής είναι <%d> ευρώ.\n",cost);
	rc = pthread_mutex_unlock(&pr);


	rc = pthread_mutex_lock(&lockCash);
	//printf("Customer %d got service\n",*tid);
	tamies++;
	rc = pthread_cond_signal(&condCash);
	rc = pthread_mutex_unlock(&lockCash);
	
	rc = pthread_mutex_lock(&tran);
	transactions[zone-1]++;
	rc = pthread_mutex_unlock(&tran);

	clock_gettime(CLOCK_REALTIME,&end);
	end_time = end.tv_sec;

	rc = pthread_mutex_lock(&totServT);
	totalServiceTime += (end_time-start_time);
	rc = pthread_mutex_unlock(&totServT);

	pthread_exit(tid);
}





int main(int argc, char **argv) { 
	
	int N_cust, seed;
	totalIncome = 0, transactionCounter = 0, totalWaitingTime = 0, totalServiceTime = 0, seatCounter = 0;
	tilefonites = N_tel;
	tamies = N_cash;	
	rows = N_zoneA + N_zoneB + N_zoneC;


	if (argc != 3) {
		printf("ERROR: Provide two arguments.\n");
		return -1;
	}


	N_cust = atoi(argv[1]);
	seed = atoi(argv[2]);

	
	if (N_cust < 0) {
		printf("ERROR: the number of threads to run should be a positive number. Current number given %d.\n", N_cust);
		exit(-1);
	}
	

	printf("Customers: %d, Seed: %d.\n", N_cust, seed);

	seedGl = seed;	


	seats = (int **) malloc(sizeof(int*) * rows);
	for (int k=0;k<rows;k++){
		seats[k]=(int*) malloc(N_seat*sizeof(int));	
	}
	

	if (seats == NULL) {
		printf("ERROR: Malloc failed not enough memory!\n");
		return -1;
	}

	
	int i, j;
	//arxikopoihsh twn timwn tou pinaka me -1
	for (i = 0; i < rows; i++) {
		for(j = 0; j < N_seat; j++) {
			seats[i][j] = -1;
		}
	}




	pthread_t *threads;

	threads = malloc(N_cust * sizeof(pthread_t));
	if (threads == NULL) {
		printf("NOT ENOUGH MEMORY!\n");
		return -1;
	}

	pthread_mutex_init(&lockThl,NULL);
	pthread_mutex_init(&lockCash,NULL);
	pthread_mutex_init(&totInc,NULL);
	pthread_mutex_init(&trCount,NULL);
	pthread_mutex_init(&totWaitT,NULL);
	pthread_mutex_init(&totServT,NULL);
	pthread_mutex_init(&plan,NULL);
	pthread_mutex_init(&pr,NULL);


	int rc;
   	int threadCount;
	int countArray[N_cust];
   	for(threadCount = 0; threadCount < N_cust; threadCount++) {
    		//printf("Main: creating thread %d\n", threadCount+1);
		countArray[threadCount] = threadCount + 1;
	    	rc = pthread_create(&threads[threadCount], NULL, Reservation, &countArray[threadCount]);


	    	if (rc != 0) {
	    		printf("ERROR: return code from pthread_create() is %d\n", rc);
	       		exit(-1);
	       	}
   	}


	void *status;

	for (threadCount = 0; threadCount < N_cust; threadCount++) {
		rc = pthread_join(threads[threadCount], &status);
		
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}

		//printf("Main: Thread %d returned %d as status code.\n", countArray[threadCount], (*(int *)status));
	}
	
	

	pthread_mutex_destroy(&lockThl);
	pthread_mutex_destroy(&lockCash);
	pthread_mutex_destroy(&totInc);
	pthread_mutex_destroy(&trCount);
	pthread_mutex_destroy(&totWaitT);
	pthread_mutex_destroy(&totServT);
	pthread_mutex_destroy(&plan);
	pthread_mutex_destroy(&pr);

	
	pthread_cond_destroy(&condThl);
	pthread_cond_destroy(&condCash);


	Outputs(N_cust);


	for (int k=0;k<rows;k++){
		free(seats[k]);	
	}
    	free(seats);
	free(threads);

	return 0;
	
}



