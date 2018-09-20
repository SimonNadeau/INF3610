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
* File : exo5.c
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

#define ROBOT_A1_PRIO   		8				 // Defining Priority of each task
#define ROBOT_A2_PRIO   		9
#define ROBOT_B1_PRIO   		10
#define ROBOT_B2_PRIO   		11
#define CONTROLLER_PRIO			7
#define MUTEX_ITEM_COUNT_PRIO	6
#define MUTEX_DONE_PRIO			22

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

OS_EVENT* mutex_item_count;
OS_EVENT* mutex_done;
OS_EVENT* RobotQueueTeam1;
OS_EVENT* RobotQueueTeam2;

int Producer_done = 0;

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
	void *RobotTeam1Msg[10];
	void *RobotTeam2Msg[10];

	// A completer

	OSInit();

	RobotQueueTeam1 = OSQCreate(&RobotTeam1Msg[0], 10);
	RobotQueueTeam2 = OSQCreate(&RobotTeam2Msg[0], 10);

	mutex_item_count = OSMutexCreate(MUTEX_ITEM_COUNT_PRIO, &err);
	mutex_done = OSMutexCreate(MUTEX_DONE_PRIO, &err);


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
	INT16U value;
	int startTime = 0;
	int orderNumber = 1;

	printf("ROBOT A @ %d : DEBUT.\n", OSTimeGet() - startTime);
	int itemCountRobotA;
	work_data* wd;
	while (1)
	{
		OSMutexPend(mutex_done, 0, &err);

		if (Producer_done) {
			OSMutexPost(mutex_done);
			value = OSQAccept(RobotQueueTeam1, &err);
			if (value == 0) {
				printf("Cons A va se détruire\n");
				OSTaskDel(OS_PRIO_SELF);
			}
		}
		else {
			OSMutexPost(mutex_done);

			if ((int)data == 1) {
				wd = OSQPend(RobotQueueTeam1, 0, &err);
				errMsg(err, "Error while trying to access RobotQueueTeam1");

				err = OSQPost(RobotQueueTeam1, &(wd->work_data_b));
				errMsg(err, "Erreur Robot B1 Post Queue");
			}
			else if ((int)data == 2) {
				wd = OSQPend(RobotQueueTeam2, 0, &err);
				errMsg(err, "Error while trying to access RobotQueueTeam2");

				err = OSQPost(RobotQueueTeam2, &(wd->work_data_b));
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
			printf("ROBOT A COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCountRobotA, OSTimeGet() - startTime);

			orderNumber++;
		}
	}
}

void robotB(void* data)
{
	INT8U err;
	INT16U value;
	int startTime = 0;
	int orderNumber = 1;
	printf("ROBOT B @ %d : DEBUT. \n", OSTimeGet() - startTime);
	int itemCountRobotB;

	while (1)
	{
		OSMutexPend(mutex_done, 0, &err);

		if (Producer_done) {
			OSMutexPost(mutex_done);
			value = OSQAccept(RobotQueueTeam2, &err);
			if (value == 0) {
				printf("Cons B va se détruire\n");
				OSTaskDel(OS_PRIO_SELF);
			}
		}
		else {
			OSMutexPost(mutex_done);

			// A completer
			if ((int)data == 1) {
				itemCountRobotB = *(int*)OSQPend(RobotQueueTeam1, 0, &err);
				errMsg(err, "Error while trying to access RobotB_Queue");
			}
			else if ((int)data == 2) {
				itemCountRobotB = *(int*)OSQPend(RobotQueueTeam2, 0, &err);
				errMsg(err, "Error while trying to access RobotB_Queue");
			}
			else {
				printf("Error wrong Data");
				exit(1);
			}

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
		err = OSQPost(RobotQueueTeam1, workData);
		errMsg(err, "Erreur Robot Team 1 Post Queue");

		err = OSQPost(RobotQueueTeam2, workData);
		errMsg(err, "Erreur Robot Team 2 Post Queue");

		// Délai aléatoire avant nouvelle commande
		randomTime = (rand() % 9 + 5) * 4;
		OSTimeDly(randomTime);
	}
	OSMutexPend(mutex_done, 0, &err);
	Producer_done = 1;
	printf("Fin de la Production\n");
	OSMutexPost(mutex_done);
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