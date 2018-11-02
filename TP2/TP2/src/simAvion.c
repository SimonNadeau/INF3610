/*
*********************************************************************************************************
*                                                 uC/OS-II
*                                          The Real-Time Kernel
*                                               PORT Windows
*
*
*            		          	Arnaud Desaulty, Frederic Fortier, Eva Terriault
*                                  Ecole Polytechnique de Montreal, Qc, CANADA
*                                                  08/2017
*
* File : simulateur.c
*
*********************************************************************************************************
*/

// Main include of ï¿½C-II
#include "simAvion.h"

#include "bsp_init.h"
#include "platform.h"
#include <stdlib.h>
#include <stdbool.h>
#include <xil_printf.h>
#include <xgpio.h>
///////////////////////////////////////////////////////////////////////////////////////
//								Routines d'interruptions
///////////////////////////////////////////////////////////////////////////////////////


void timer_isr(void* not_valid) {
	if (private_timer_irq_triggered()) {
		private_timer_clear_irq();
		OSTimeTick();
	}
}

void fit_timer_1s_isr(void *not_valid) {
	/*TODO: definition handler pour timer 1s*/
	uint8_t err = OSSemPost(semGen);
	errMsg(err, "semGen");
	//safePrint("Handler timer 1s\n");

}

void fit_timer_3s_isr(void *not_valid) {
	/*TODO: definition handler pour timer 3s*/
	uint8_t err = OSSemPost(semVer);
	errMsg(err, "semVer");
	//safePrint("Handler timer 3s\n");

}

void gpio_isr(void * not_valid) {
	/*TODO: definition handler pour switches*/
	uint8_t err = OSSemPost(semStat);
	errMsg(err, "semStat");
	XGpio_InterruptClear(&gpSwitch, XGPIO_IR_MASK);
	//safePrint("Handler gpio\n");
}

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

int main(void)
{
	initialize_bsp();

	OSInit(); 

	create_application();

	prepare_and_enable_irq();

	safePrint("*** Starting uC/OS-II scheduler ***\n");

	stopSimDebordement = false;

	OSStart();

	cleanup();

	cleanup_platform();

	return 0;
}

void create_application() {
	int error;

	error = create_tasks();
	if (error != 0)
		xil_printf("Error %d while creating tasks\n", error);

	error = create_events();
	if (error != 0)
		xil_printf("Error %d while creating events\n", error);
}

int create_tasks() {
	// Stacks
	static OS_STK generationStk[TASK_STK_SIZE]; //Stack of each task
	static OS_STK atterrissage0Stk[TASK_STK_SIZE];
	static OS_STK terminal1Stk[TASK_STK_SIZE];
	static OS_STK terminal2Stk[TASK_STK_SIZE];
	static OS_STK decollageStk[TASK_STK_SIZE];
	static OS_STK statistiquesStk[TASK_STK_SIZE];
	static OS_STK verificationStk[TASK_STK_SIZE];

	uint8_t err;

	/*TODO: Creation des taches*/
	err = OSTaskCreate(generation, NULL, &generationStk[TASK_STK_SIZE - 1], GENERATION_PRIO);
	errMsg(err, "Error with generation task creation");

	err = OSTaskCreate(atterrissage, NULL, &atterrissage0Stk[TASK_STK_SIZE - 1], ATTERRISSAGE_PRIO);
	errMsg(err, "Error with atterrissage task creation");

	err = OSTaskCreate(terminal, (void*) 1, &terminal1Stk[TASK_STK_SIZE - 1], TERMINAL1_PRIO);
	errMsg(err, "Error with terminal 0 task creation");

	err = OSTaskCreate(terminal, (void*) 2, &terminal2Stk[TASK_STK_SIZE - 1], TERMINAL2_PRIO);
	errMsg(err, "Error with terminal 1 task creation");

	err = OSTaskCreate(decollage, NULL, &decollageStk[TASK_STK_SIZE - 1], DECOLLAGE_PRIO);
	errMsg(err, "Error with decollage task creation");

	err = OSTaskCreate(statistiques, NULL, &statistiquesStk[TASK_STK_SIZE - 1], STATISTIQUES_PRIO);
	errMsg(err, "Error with statistiques task creation");

	err = OSTaskCreate(verification, NULL, &verificationStk[TASK_STK_SIZE - 1], VERIFICATION_PRIO);
	errMsg(err, "Error with verification task creation");

	return 0;
}

int create_events() {
	uint8_t err;

	/* TODO: Creation des semaphores, flags, files, maiblox, mutex, ... */

	// Mutex
	mutexVerification = OSMutexCreate(MUTEX_VERIFICATION_PRIO, &err);
	errMsg(err, "Error while creation of verification mutex");

	mutexPrint = OSMutexCreate(MUTEX_PRINT_PRIO, &err);
	errMsg(err, "Error while creation of print mutex");

	mutexDecollage = OSMutexCreate(MUTEX_DECOLLAGE_PRIO, &err);
	errMsg(err, "Error while creation of decollage mutex");

	mutexAtterrissage = OSMutexCreate(MUTEX_ATTERRISSAGE_PRIO, &err);
	errMsg(err, "Error while creation of Atterrissage mutex");

	mutexTerminal1 = OSMutexCreate(MUTEX_TERMINAL1_PRIO, &err);
	errMsg(err, "Error while creation of mutexTerminal1");

	mutexTerminal2 = OSMutexCreate(MUTEX_TERMINAL2_PRIO, &err);
	errMsg(err, "Error while creation of mutexTerminal1");

	// Sémaphores
	if (!(semGen = OSSemCreate(0))){
		safePrint("Error while creation event semGen");
	}
	if (!(semVer = OSSemCreate(0))){
		safePrint("Error while creation event semVer");
	}
	if (!(semStat = OSSemCreate(0))){
		safePrint("Error while creation event semStat");
	}

	// Queues
	Q_atterrissage_high = OSQCreate(&Q_atterrissage_high_data[0], 3);
	Q_atterrissage_medium = OSQCreate(&Q_atterrissage_medium_data[0], 4);
	Q_atterrissage_low = OSQCreate(&Q_atterrissage_low_data[0], 6);

	Q_decollage = OSQCreate(&Q_decollage_data[0], 10);

	Q_terminal1 = OSQCreate(&Q_terminal1_data[0], 1);
	Q_terminal2 = OSQCreate(&Q_terminal2_data[0], 1);

	return 0;
}

/*
*********************************************************************************************************
*                                            TASK FUNCTIONS
*********************************************************************************************************
*/
void generation(void* data) {
	uint8_t err;
	int nbAvionsCrees = 0;
	safePrint("[GENERATION] Tache lancee\n");
	int skipGen = 0;
	int seed = 42;

	while (1) {
		/*TODO: Synchronisation unilaterale timer 1s*/
		OSSemPend(semGen, 0, &err);
		errMsg(err, "Error while trying to access semGen");

		srand(seed);
		skipGen = rand() % 5; //On saute la generation 1 fois sur 5
		if (skipGen != 0){
			safePrint("[GENERATION] Generation\n");
			Avion* avion = malloc(sizeof(Avion));
			avion->id = nbAvionsCrees;
			remplirAvion(avion);
			nbAvionsCrees++;

			/*TODO: Envoi des avions dans les files appropriees*/
			if (avion->retard <= BORNE_SUP_LOW){
				err = OSQPost(Q_atterrissage_low, avion);
				signalOverflow(err);
				errMsg(err, "Erreur Atterrissage Low Post Queue");
			}
			else if (avion->retard >= BORNE_INF_MEDIUM && avion->retard <= BORNE_SUP_MEDIUM) {
				err = OSQPost(Q_atterrissage_medium, avion);
				signalOverflow(err);
				errMsg(err, "Erreur Atterrissage Medium Post Queue");
			}
			else if (avion->retard >= BORNE_INF_HIGH && avion->retard <= BORNE_SUP_HIGH){
				err = OSQPost(Q_atterrissage_high, avion);
				signalOverflow(err);
				errMsg(err, "Erreur Atterrissage High Post Queue");
			}
			else {
				errMsg(err, "Wrong delay time");
			}
		}
		else{
			safePrint("[GENERATION] Pas de generation\n");
		}
		seed++;
	}

}

void atterrissage(void* data)
{
	uint8_t err;
	Avion* avion = NULL;
	safePrint("[ATTERRISSAGE] Tache lancee\n");
	while (1) {

		/*TODO: Mise en attente des 3 files en fonction de leur priorité*/
		if ((avion = OSQAccept(Q_atterrissage_high, &err)) != NULL) {
			processLanding();
		}
		else if ((avion = OSQAccept(Q_atterrissage_medium, &err)) != NULL) {
			processLanding();
		}
		else if ((avion = OSQAccept(Q_atterrissage_low, &err))!= NULL) {
			processLanding();
		}

		/*TODO: Mise en attente d'un terminal libre (mecanisme a votre choix)*/
		/*TODO: Envoi de l'avion au terminal choisi (mecanisme de votre choix)*/
		if (avion != NULL && nbrAvionQueue(Q_terminal1) == 0) {
			err = OSQPost(Q_terminal1, avion);
			errMsg(err, "Erreur Post Q_terminal1");
			avion = NULL;
			safePrint("[ATTERRISSAGE] Terminal libre num 1 obtenu\n");
		}

		if (avion != NULL && nbrAvionQueue(Q_terminal2) == 0) {
			err = OSQPost(Q_terminal2, avion);
			errMsg(err, "Erreur Post Q_terminal2");
			avion = NULL;
			safePrint("[ATTERRISSAGE] Terminal libre num 2 obtenu\n");
		}

	}
}

void processLanding(){
	uint8_t err;

	OSMutexPend(mutexAtterrissage, 0, &err);
	errMsg(err, "Error while trying to access mutexAtterrissage");

	safePrint("[ATTERRISSAGE] Debut atterrissage\n");
	OSTimeDly(150); //Temps pour que l'avion atterrisse
	safePrint("[ATTERRISSAGE] Attente terminal libre\n");

	err = OSMutexPost(mutexAtterrissage);
	errMsg(err, "Error while trying to post mutexAtterrissage");

}

void terminal(void* data)
{
	uint8_t err;
	int numTerminal = (int)data;
	Avion* avion = NULL;
	numTerminal == 1 ? safePrint("[TERMINAL 1] Tache lancee\n") : safePrint("[TERMINAL 2] Tache lancee\n");

	while (1) {

		/*TODO: Mise en attente d'un avion venant de la piste d'atterrissage*/
		if (numTerminal == 1) {
			avion = OSQPend(Q_terminal1, 0, &err);
			errMsg(err, "Error while trying to access Q_terminal1");
			safePrint("[TERMINAL 1] Obtention avion\n");

			OSMutexPend(mutexTerminal1, 0, &err);
			errMsg(err, "Error while trying to access mutexTerminal1");

			OSTimeDly(160);//Attente pour le vidage, le nettoyage et le remplissage de l'avion
			remplirAvion(avion);

			err = OSMutexPost(mutexTerminal1);
			errMsg(err, "Error while trying to post mutexTerminal1");

		} else if (numTerminal == 2) {
			avion = OSQPend(Q_terminal2, 0, &err);
			errMsg(err, "Error while trying to access Q_terminal2");
			safePrint("[TERMINAL 2] Obtention avion\n");

			OSMutexPend(mutexTerminal2, 0, &err);
			errMsg(err, "Error while trying to access mutexTerminal2");

			OSTimeDly(160);//Attente pour le vidage, le nettoyage et le remplissage de l'avion
			remplirAvion(avion);

			err = OSMutexPost(mutexTerminal2);
			errMsg(err, "Error while trying to post mutexTerminal2");

		} else {
			safePrint("Wrong terminal number");
		}


		/*TODO: Envoi de l'avion pour le piste de decollage*/
		/*TODO: Notifier que le terminal est libre (mecanisme de votre choix)*/
		err = OSQPost(Q_decollage, avion);
		errMsg(err, "Erreur Avion Post Queue");

		if ((int)data == 1) {safePrint("[TERMINAL 1] Liberation avion\n");}
		else if ((int)data == 2) {safePrint("[TERMINAL 2] Liberation avion\n");}
	}
	
}

void decollage(void* data)
{
	uint8_t err;
	Avion* avion = NULL;
	safePrint("[DECOLLAGE] Tache lancee\n");

	while (1) {
		/*TODO: Mise en attente d'un avion pret pour le decollage*/
		avion = OSQPend(Q_decollage, 0, &err);
		errMsg(err, "Error while trying to access RobotA_Queue");

		OSMutexPend(mutexDecollage, 0, &err);
		errMsg(err, "Error while trying to access mutexDecollage");

		OSTimeDly(30); //Temps pour que l'avion decolle
		safePrint("[DECOLLAGE] Avion decolle\n");

		err = OSMutexPost(mutexDecollage);
		errMsg(err, "Error while trying to post mutexDecollage");

		/*TODO: Destruction de l'avion*/
		free(avion);
	}
}


void statistiques(void* data){
	uint8_t err;
	safePrint("[STATISTIQUES] Tache lancee\n");
	while(1){
		/*TODO: Synchronisation unilaterale switches*/
		OSSemPend(semStat, 0, &err);
		errMsg(err, "Error while trying to access semVer");

		safePrint("\n------------------ Affichage des statistiques ------------------\n");

		/*TODO: Obtenir statistiques pour les files d'atterrissage*/
		/*TODO: Obtenir statistiques pour la file de decollage*/
		OSMutexPend(mutexPrint, 0, &err);
		xil_printf("Nb d'avions en attente d'atterrissage de type High :  %d\n", nbrAvionQueue(Q_atterrissage_high));
		xil_printf("Nb d'avions en attente d'atterrissage de type Medium :  %d\n", nbrAvionQueue(Q_atterrissage_medium));
		xil_printf("Nb d'avions en attente d'atterrissage de type Low :  = %d\n", nbrAvionQueue(Q_atterrissage_low));
		xil_printf("Nb d'avions en attente de decollage:  = %d\n", nbrAvionQueue(Q_decollage));
		OSMutexPost(mutexPrint);


		/*TODO: Obtenir statut des terminaux*/
		safePrint("Terminal 1 ");
		nbrAvionQueue(Q_terminal1) == 0 ? safePrint("LIBRE\n") : safePrint("OCCUPE\n");

		safePrint("Terminal 2 ");
		nbrAvionQueue(Q_terminal2) == 0 ? safePrint("LIBRE\n") : safePrint("OCCUPE\n");
	}
}

int nbrAvionQueue(OS_EVENT* queue){
	int nbrAvion = 0;
	uint8_t err;
	OS_Q_DATA qdata;

	err = OSQQuery(queue, &qdata);
	if (err == OS_ERR_NONE){
		nbrAvion = qdata.OSNMsgs;
	} else {
		errMsg(err, "Error while trying to query queue");
	}
	return nbrAvion;
}

void verification(void* data){
	uint8_t err;

	safePrint("[VERIFICATION] Tache lancee\n");
	while(1){
		/*TODO: Synchronisation unilaterale avec timer 3s*/
		OSSemPend(semVer, 0, &err);
		errMsg(err, "Error while trying to access semVer");

		if (stopSimDebordement){
			/*TODO: Suspension de toutes les taches de la simulation*/

			OSTaskSuspend(STATISTIQUES_PRIO);
			errMsg(err, "Error with suspending task statistique");

			OSTaskSuspend(GENERATION_PRIO);
			errMsg(err, "Error with suspending task generation");

			OSTaskSuspend(ATTERRISSAGE_PRIO);
			errMsg(err, "Error with suspending task atterrissage");

			OSTaskSuspend(TERMINAL1_PRIO);
			errMsg(err, "Error with suspending task terminal 1");

			OSTaskSuspend(TERMINAL2_PRIO);
			errMsg(err, "Error with suspending task terminal 2");

			OSTaskSuspend(DECOLLAGE_PRIO);
			errMsg(err, "Error with suspending task decollage");

			OSTaskSuspend(VERIFICATION_PRIO);
			errMsg(err, "Error with suspending task verification");

			exit(1);
		}
	}


}
void remplirAvion(Avion* avion) {

	uint8_t err;

	srand(OSTimeGet());
	avion->retard = rand() % BORNE_SUP_HIGH;
	avion->origine = rand() % NB_AEROPORTS;
	do { avion->destination = rand() % NB_AEROPORTS; } while (avion->origine == avion->destination);

	OSMutexPend(mutexPrint, 0, &err);
	xil_printf("Avion retard = %d\n", avion->retard);
	xil_printf("Avion origine = %d\n", avion->origine);
	xil_printf("Avion destination = %d\n", avion->destination);
	OSMutexPost(mutexPrint);
}

void signalOverflow(uint8_t err)
{
	if (err != OS_ERR_NONE)
	{
		xil_printf("Overflow");
		OSMutexPend(mutexVerification, 0, &err);
		errMsg(err, "Error while trying to access mutexVerification");
		stopSimDebordement = true;
		err = OSMutexPost(mutexVerification);
		errMsg(err, "Error while trying to post mutexVerification");
	}
}

void errMsg(uint8_t err, char* errMsg)
{
	if (err != OS_ERR_NONE)
	{
		OSMutexPend(mutexPrint, 0, &err);
		xil_printf(errMsg);
		OSMutexPost(mutexPrint);

		exit(1);
	}
}

void safePrint(char* printMsg)
{
	uint8_t err;

	OSMutexPend(mutexPrint, 0, &err);
	xil_printf(printMsg);
	OSMutexPost(mutexPrint);
}
