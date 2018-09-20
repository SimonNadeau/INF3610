/*
*********************************************************************************************************
*                                                 uC/OS-II
*                                          The Real-Time Kernel
*                                               PORT Windows
*
*
*                                     Arnaud Desault, Fr�d�ric Fortier
*                                  �cole Polytechnique de Montreal, QC, CANADA
*                                                  01/2018
*
* File : exo1.c
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

#define TASK_STK_SIZE       16384            // Size of each task's stacks (# of WORDs)

#define TASK1_PRIO   		6				 // Defining Priority of each task
#define TASK2_PRIO   		5
#define TASK3_PRIO   		7
#define TASK4_PRIO   		8
#define TASK5_PRIO   		9

/*
*********************************************************************************************************
*                                          SHARED  VARIABLES
*********************************************************************************************************
*/

OS_STK           Task1Stk[TASK_STK_SIZE];	//Stack of each task
OS_STK           Task2Stk[TASK_STK_SIZE];
OS_STK           Task3Stk[TASK_STK_SIZE];
OS_STK           Task4Stk[TASK_STK_SIZE];
OS_STK           Task5Stk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void    Task1(void *data);
void    Task2(void *data);
void    Task3(void *data);
void    Task4(void *data);
void    Task5(void *data);
void	errMsg(INT8U err, char* errMsg);

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main(void)
{
	// � compl�ter
	INT8U err;

	OSInit();

	err = OSTaskCreate(Task1, (void*)0, &Task1Stk[TASK_STK_SIZE - 1], TASK1_PRIO);
	errMsg(err, "Erreur dans la tache 1\n");	
	err = OSTaskCreate(Task2, (void*)0, &Task2Stk[TASK_STK_SIZE - 1], TASK2_PRIO);
	errMsg(err, "Erreur dans la tache 2\n");	
	err = OSTaskCreate(Task3, (void*)0, &Task3Stk[TASK_STK_SIZE - 1], TASK3_PRIO);
	errMsg(err, "Erreur dans la tache 3\n");	
	err = OSTaskCreate(Task4, (void*)0, &Task4Stk[TASK_STK_SIZE - 1], TASK4_PRIO);
	errMsg(err, "Erreur dans la tache 4\n");
	err = OSTaskCreate(Task5, (void*)0, &Task5Stk[TASK_STK_SIZE - 1], TASK5_PRIO);
	errMsg(err, "Erreur dans la tache 5\n");
	
	OSStart();

	return;
}

/*
*********************************************************************************************************
*                                            TASK FUNCTIONS
*********************************************************************************************************
*/

void Task1(void* data)
{
	while (1)
	{
		printf("are an \n");
		OSTimeDly(10000);
	}
}

void Task2(void* data)
{
	while (1)
	{
		printf("Task priorities \n");
		OSTimeDly(10000);
	}
}

void Task3(void* data)
{
	while (1)
	{
		printf("important \n");
		OSTimeDly(10000);
	}
}

void Task4(void* data)
{
	while (1)
	{
		printf("feature \n");
		OSTimeDly(10000);
	}
}

void Task5(void* data)
{
	while (1)
	{
		printf("of MicroC-II ! \n");
		OSTimeDly(10000);
	}
}

/*
*********************************************************************************************************
*                                                 ERROR
*********************************************************************************************************
*/

void errMsg(INT8U err, char* errMsg)
{
	if (err != OS_ERR_NONE)
	{
		printf(errMsg);
		exit(1);
	}
}