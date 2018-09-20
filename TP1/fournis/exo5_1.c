/*
*********************************************************************************************************
*                                                 uC/OS-II
*                                          The Real-Time Kernel
*                                               PORT Windows
*
*
*            		          	Arnaud Desaulty, Frederic Fortier, Eva Terriault
*                                  Ecole Polytechnique de Montreal, Qc, CANADA
*                                                  01/2018
*
* File : exo4.c
*
*********************************************************************************************************
*/

// Main include of �C-II
#include "includes.h"
/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE			16384            // Size of each task's stacks (# of WORDs)

#define ROBOT_A1_PRIO   		8				 // Defining Priority of each task
#define ROBOT_A2_PRIO   		9
#define ROBOT_B1_PRIO   		10
#define ROBOT_B2_PRIO   		11
#define CONTROLLER_PRIO			7
#define MUTEX_ITEM_COUNT_PRIO	6


/*
*********************************************************************************************************
*                                             VARIABLES
*********************************************************************************************************
*/

OS_STK           prepRobotA1Stk[TASK_STK_SIZE];	//Stack of each task
OS_STK           prepRobotA2Stk[TASK_STK_SIZE];	//Stack of each task
OS_STK           prepRobotB1Stk[TASK_STK_SIZE];
OS_STK           prepRobotB2Stk[TASK_STK_SIZE];
OS_STK           controllerStk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                           SHARED  VARIABLES
*********************************************************************************************************
*/
volatile int total_item_count = 0;
int producer_done = 0;

OS_EVENT* mutex_item_count;

OS_EVENT* Robot_A_Queue;
OS_EVENT* Robot_B1_Queue;
OS_EVENT* Robot_B2_Queue;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void    robotA(void *data);
void    robotB(void *data);
void    controller(void *data);
void    errMsg(INT8U err, char* errMSg);
int		readCurrentTotalItemCount();
void	writeCurrentTotalItemCount(int qty);

/*
*********************************************************************************************************
*                                             STRUCTURES
*********************************************************************************************************
*/

typedef struct work_data {
	int work_data_a;
	int work_data_b;
} work_data;

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main(void)
{
	UBYTE err;
	void *RobotAMsg[10];
	void *RobotB1Msg[10];
	void *RobotB2Msg[10];

	// A completer

	OSInit();

	Robot_A_Queue = OSQCreate(&RobotAMsg[0], 10);
	Robot_B1_Queue = OSQCreate(&RobotB1Msg[0], 10);
	Robot_B2_Queue = OSQCreate(&RobotB2Msg[0], 10);

	mutex_item_count = OSMutexCreate(MUTEX_ITEM_COUNT_PRIO, &err);
	errMsg(err, "Erreur creation mutex_item_count");

	errMsg(OSTaskCreate(controller, (void*)0, &controllerStk[TASK_STK_SIZE - 1], CONTROLLER_PRIO), "Erreur Controller");
	errMsg(OSTaskCreate(robotA, (void*) 1, &prepRobotA1Stk[TASK_STK_SIZE - 1], ROBOT_A1_PRIO), "Erreur Robot A - 1");
	errMsg(OSTaskCreate(robotA, (void*) 2, &prepRobotA2Stk[TASK_STK_SIZE - 1], ROBOT_A2_PRIO), "Erreur Robot A - 2");
	errMsg(OSTaskCreate(robotB, (void*) 1, &prepRobotB1Stk[TASK_STK_SIZE - 1], ROBOT_B1_PRIO), "Erreur Robot B - 1");
	errMsg(OSTaskCreate(robotB, (void*) 2, &prepRobotB2Stk[TASK_STK_SIZE - 1], ROBOT_B2_PRIO), "Erreur Robot B - 2");

	OSStart();

	return;
}

/*
*********************************************************************************************************
*                                            TASK FUNCTIONS
*********************************************************************************************************
*/

void robotA(void* data)
{
	INT8U err;
	int startTime = 0;
	int orderNumber = 1;

	printf("ROBOT A%d @ %d : DEBUT.\n", data, OSTimeGet() - startTime);
	int itemCountRobotA;
	work_data* wd;

	while (1)
	{
		// A completer
		wd = OSQAccept(Robot_A_Queue);
		errMsg(err, "Error while trying to access Robot_A_Queue");

		if (producer_done && wd == NULL) {
			printf("Robot A%d va se suspendre\n", data);
			OSTaskSuspend(OS_PRIO_SELF);
		}
		else {
			if ((int)data == 1) {
				err = OSQPost(Robot_B1_Queue, &(wd->work_data_b));
				errMsg(err, "Erreur Robot B1 Post Queue");
			}
			else if ((int)data == 2) {
				err = OSQPost(Robot_B2_Queue, &(wd->work_data_b));
				errMsg(err, "Erreur Robot B2 Post Queue");
			}
			else {
				printf("Error wrong Data");
				exit(1);
			}

			itemCountRobotA = wd->work_data_a;

			OSMutexPend(mutex_item_count, 0, &err);
			errMsg(err, "Error while trying to access mutex_item_count");
			writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCountRobotA);
			err = OSMutexPost(mutex_item_count);
			errMsg(err, "Error while trying to post mutex_item_count");


			int counter = 0;
			while (counter < itemCountRobotA * 1000) { counter++; }
			printf("ROBOT A%d COMMANDE #%d avec %d items @ %d.\n", data, orderNumber, itemCountRobotA, OSTimeGet() - startTime);

			orderNumber++;
		}
	}
}

void robotB(void* data)
{
	INT8U err;
	int startTime = 0;
	int orderNumber = 1;

	printf("ROBOT B%d @ %d : DEBUT. \n", data, OSTimeGet() - startTime);
	int* itemCountRobotB;

	while (1)
	{
		// A completer
		if ((int)data == 1) {
			itemCountRobotB = (int*)OSQAccept(Robot_B1_Queue, 0, &err);
			errMsg(err, "Error while trying to access Robot_B1_Queue");
		}
		else if ((int)data == 2) {
			itemCountRobotB = (int*)OSQAccept(Robot_B2_Queue, 0, &err);
			errMsg(err, "Error while trying to access Robot_B1_Queue");
		}
		else {
			printf("Error wrong Data");
			exit(1);
		}

		if (producer_done && itemCountRobotB == NULL) {
			printf("Rodot B%d B va se suspendre\n", data);
			OSTaskSuspend(OS_PRIO_SELF);
		}
		else {
			OSMutexPend(mutex_item_count, 0, &err);
			errMsg(err, "Error while trying to access mutex_item_count");
			writeCurrentTotalItemCount(readCurrentTotalItemCount() + *itemCountRobotB);
			err = OSMutexPost(mutex_item_count);
			errMsg(err, "Error while trying to post mutex_item_count");

			int counter = 0;
			while (counter < *itemCountRobotB * 1000) { counter++; }
			printf("ROBOT B%d COMMANDE #%d avec %d items @ %d.\n", data, orderNumber, *itemCountRobotB, OSTimeGet() - startTime);

			orderNumber++;
		}
	}
}

void controller(void* data)
{
	INT8U err;
	int startTime = 0;
	int randomTime = 0;
	work_data* workData;
	printf("TACHE CONTROLLER @ %d : DEBUT. \n", OSTimeGet() - startTime);

	for (int i = 1; i < 11; i++)
	{
		//Cr�ation d'une commande
		workData = malloc(sizeof(work_data));

		workData->work_data_a = (rand() % 8 + 3) * 10;
		workData->work_data_b = (rand() % 8 + 6) * 10;

		printf("TACHE CONTROLLER @ %d : COMMANDE #%d. \n prep time A = %d, prep time B = %d\n", OSTimeGet() - startTime, i, workData->work_data_a, workData->work_data_b);
		
		// A completer
		err = OSQPost(Robot_A_Queue, workData);
		errMsg(err, "Erreur Robot A Post Queue");

		// D�lai al�atoire avant nouvelle commande
		randomTime = (rand() % 9 + 5) * 4;
		OSTimeDly(randomTime);
	}

	producer_done = 1;
	printf("Fin de la Production\n");

	free(workData);
}

int	readCurrentTotalItemCount()
{
	OSTimeDly(2);
	return total_item_count;
}
void writeCurrentTotalItemCount(int newCount)
{
	OSTimeDly(2);
	total_item_count = newCount;
}

void errMsg(INT8U err, char* errMsg)
{
	if (err != OS_ERR_NONE)
	{
		printf(errMsg);
		exit(1);
	}
}