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

// Main include of µC-II
#include "includes.h"
/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE			16384            // Size of each task's stacks (# of WORDs)

#define ROBOT_A_PRIO   			8				 // Defining Priority of each task
#define ROBOT_B_PRIO   			9
#define CONTROLLER_PRIO			7
#define MUTEX_ITEM_COUNT_PRIO	6


/*
*********************************************************************************************************
*                                             VARIABLES
*********************************************************************************************************
*/

OS_STK           prepRobotAStk[TASK_STK_SIZE];	//Stack of each task
OS_STK           prepRobotBStk[TASK_STK_SIZE];
OS_STK           controllerStk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                           SHARED  VARIABLES
*********************************************************************************************************
*/
volatile int total_item_count = 0;

OS_EVENT* mutex_item_count;
OS_EVENT* Robot_Queue;
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
	void *RobotMsg[10];

	// A completer

	OSInit();

	Robot_Queue = OSQCreate(&RobotMsg[0], 10);

	mutex_item_count = OSMutexCreate(MUTEX_ITEM_COUNT_PRIO, &err);

	errMsg(OSTaskCreate(controller, (void*)0, &controllerStk[TASK_STK_SIZE - 1], CONTROLLER_PRIO), "Erreur Controller");
	errMsg(OSTaskCreate(robotA, (void*)0, &prepRobotAStk[TASK_STK_SIZE - 1], ROBOT_A_PRIO), "Erreur Robot A");
	errMsg(OSTaskCreate(robotB, (void*)0, &prepRobotBStk[TASK_STK_SIZE - 1], ROBOT_B_PRIO), "Erreur Robot B");

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

	printf("ROBOT A @ %d : DEBUT.\n", OSTimeGet() - startTime);
	int itemCountRobotA;
	work_data* wd;
	while (1)
	{
		// A completer
		wd = OSQPend(Robot_Queue, 0, &err);
		errMsg(err, "Error while trying to access RobotA_Queue");

		err = OSQPost(Robot_Queue, &(wd->work_data_b));
		errMsg(err, "Erreur Robot B Post Queue");

		itemCountRobotA = wd->work_data_a;

		OSMutexPend(mutex_item_count, 0, &err);
		errMsg(err, "Error while trying to access mutex_item_count");
		writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCountRobotA);
		err = OSMutexPost(mutex_item_count);
		errMsg(err, "Error while trying to post mutex_item_count");


		int counter = 0;
		while (counter < itemCountRobotA * 1000) { counter++; }
		printf("ROBOT A COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCountRobotA, OSTimeGet() - startTime);

		orderNumber++;
	}
}

void robotB(void* data)
{
	INT8U err;
	int startTime = 0;
	int orderNumber = 1;
	printf("ROBOT B @ %d : DEBUT. \n", OSTimeGet() - startTime);
	int itemCountRobotB;
	while (1)
	{
		// A completer
		itemCountRobotB = *(int*)OSQPend(Robot_Queue, 0, &err);
		errMsg(err, "Error while trying to access RobotB_Queue");

		OSMutexPend(mutex_item_count, 0, &err);
		errMsg(err, "Error while trying to access mutex_item_count");
		writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCountRobotB);
		err = OSMutexPost(mutex_item_count);
		errMsg(err, "Error while trying to post mutex_item_count");

		int counter = 0;
		while (counter < itemCountRobotB * 1000) { counter++; }
		printf("ROBOT B COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCountRobotB, OSTimeGet() - startTime);

		orderNumber++;
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
		//Création d'une commande
		workData = malloc(sizeof(work_data));

		workData->work_data_a = (rand() % 8 + 3) * 10;
		workData->work_data_b = (rand() % 8 + 6) * 10;

		printf("TACHE CONTROLLER @ %d : COMMANDE #%d. \n prep time A = %d, prep time B = %d\n", OSTimeGet() - startTime, i, workData->work_data_a, workData->work_data_b);
		
		// A completer
		err = OSQPost(Robot_Queue, workData);
		errMsg(err, "Erreur Robot A Post Queue");

		// Délai aléatoire avant nouvelle commande
		randomTime = (rand() % 9 + 5) * 4;
		OSTimeDly(randomTime);
	}

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