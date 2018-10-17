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
	safePrint("Handler timer 1s");

}

void fit_timer_3s_isr(void *not_valid) {
	/*TODO: definition handler pour timer 3s*/
	uint8_t err = OSSemPost(semVer);
	errMsg(err, "semVer");
	safePrint("Handler timer 3s");

}

void gpio_isr(void * not_valid) {
	/*TODO: definition handler pour switches*/
	uint8_t err = OSSemPost(semStat);
	errMsg(err, "semStat");
	XGpio_InterruptClear(&gpSwitch, 0xFFFFFFF);
	safePrint("Handler gpio");
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
	static OS_STK terminal0Stk[TASK_STK_SIZE];
	static OS_STK terminal1Stk[TASK_STK_SIZE];
	static OS_STK decollageStk[TASK_STK_SIZE];
	static OS_STK statistiquesStk[TASK_STK_SIZE];
	static OS_STK verificationStk[TASK_STK_SIZE];

	uint8_t err;

	/*TODO: Creation des taches*/
	err = OSTaskCreate(generation, NULL, &generationStk[TASK_STK_SIZE - 1], GENERATION_PRIO);
	errMsg(err, "Error with generation task creation");

	err = OSTaskCreate(atterrissage, NULL, &atterrissage0Stk[TASK_STK_SIZE - 1], ATTERRISSAGE_PRIO);
	errMsg(err, "Error with atterrissage task creation");

	err = OSTaskCreate(terminal, (void*) 1, &terminal0Stk[TASK_STK_SIZE - 1], TERMINAL0_PRIO);
	errMsg(err, "Error with terminal 0 task creation");

	err = OSTaskCreate(terminal, (void*) 2, &terminal1Stk[TASK_STK_SIZE - 1], TERMINAL1_PRIO);
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

	mutexPrint = OSMutexCreate(MUTEX_PRINT_PRIO, &err);
	errMsg(err, "Error while creation of print mutex");

	if (!(semGen = OSSemCreate(0))){
		safePrint("Error while creation event semGen");
	}
	if (!(semVer = OSSemCreate(0))){
		safePrint("Error while creation event semVer");
	}
	if (!(semStat = OSSemCreate(0))){
		safePrint("Error while creation event semStat");
	}

	Q_atterrissage_high = OSQCreate(&Q_atterrissage_high_data[0], 3);
	Q_atterrissage_medium = OSQCreate(&Q_atterrissage_medium_data[0], 4);
	Q_atterrissage_low = OSQCreate(&Q_atterrissage_low_data[0], 6);

	Q_decollage = OSQCreate(&Q_decollage_data[0], 10);

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
			Avion* avion = malloc(sizeof(Avion));
			avion->id = nbAvionsCrees;
			remplirAvion(avion);
			nbAvionsCrees++;

			/*TODO: Envoi des avions dans les files appropriees*/
			if (avion->retard <= BORNE_SUP_LOW){
				err = OSQPost(Q_atterrissage_low, avion);
				errMsg(err, "Erreur Atterrissage Low Post Queue");
			}
			else if (avion->retard >= BORNE_INF_MEDIUM && avion->retard <= BORNE_SUP_MEDIUM) {
				err = OSQPost(Q_atterrissage_medium, avion);
				errMsg(err, "Erreur Atterrissage Medium Post Queue");
			}
			else if (avion->retard >= BORNE_INF_HIGH && avion->retard <= BORNE_SUP_HIGH){
				err = OSQPost(Q_atterrissage_high, avion);
				errMsg(err, "Erreur Atterrissage High Post Queue");
			}
			else {
				errMsg(err, "Impossible retard time");
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

		safePrint("[ATTERRISSAGE] Debut atterrissage\n");
		OSTimeDly(150); //Temps pour que l'avion atterrisse

		safePrint("[ATTERRISSAGE] Attente terminal libre\n");
		/*TODO: Mise en attente d'un terminal libre (mecanisme a votre choix)*/
		//safePrint("[ATTERRISSAGE] Terminal libre num %d obtenu\n", ...);

		/*TODO: Envoi de l'avion au terminal choisi (mecanisme de votre choix)*/
	}
}

void terminal(void* data)
{
	uint8_t err;
	int numTerminal = 0; //TODO: A modifier
	Avion* avion = NULL;
	safePrint("[TERMINAL 0 ] Tache lancee\n"); //TODO: A modifier

	while (1) {

		/*TODO: Mise en attente d'un avion venant de la piste d'atterrissage*/
		safePrint("[TERMINAL 0] Obtention avion\n"); //TODO: A modifier

		OSTimeDly(160);//Attente pour le vidage, le nettoyage et le remplissage de l'avion

		remplirAvion(avion);

		/*TODO: Envoi de l'avion pour le piste de decollage*/
		safePrint("[TERMINAL 0] Liberation avion\n"); //TODO: A modifier

		/*TODO: Notifier que le terminal est libre (mecanisme de votre choix)*/
	}
	
}

void decollage(void* data)
{
	uint8_t err;
	Avion* avion = NULL;
	safePrint("[DECOLLAGE] Tache lancee\n");

	while (1) {
		/*TODO: Mise en attente d'un avion pret pour le decollage*/

		OSTimeDly(30); //Temps pour que l'avion decolle
		safePrint("[DECOLLAGE] Avion decolle\n");

		/*TODO: Destruction de l'avion*/
	}
}


void statistiques(void* data){
	uint8_t err;
	safePrint("[STATISTIQUES] Tache lancee\n");
	while(1){
		/*TODO: Synchronisation unilaterale switches*/
		safePrint("\n------------------ Affichage des statistiques ------------------\n");

		/*TODO: Obtenir statistiques pour les files d'atterrissage*/
		/*xil_printf("Nb d'avions en attente d'atterrissage de type High : %d\n", ...);
		xil_printf("Nb d'avions en attente d'atterrissage de type Medium : %d\n", ...);
		xil_printf("Nb d'avions en attente d'atterrissage de type Low : %d\n", ...);*/

		/*TODO: Obtenir statistiques pour la file de decollage*/
		//xil_printf("Nb d'avions en attente de decollage : %d\n", ...);

		/*TODO: Obtenir statut des terminaux*/
		safePrint("Terminal 0 ");
		int statutTerm0 = 0; /*A modifier (simplement un exemple d'affichage pour vous aider)*/
		(statutTerm0 == 0) ? safePrint("OCCUPE\n") : safePrint("LIBRE\n");

		safePrint("Terminal 1 ");
		int statutTerm1 = 0; /*A modifier (simplement un exemple d'affichage pour vous aider)*/
		(statutTerm1 == 0) ? safePrint("OCCUPE\n") : safePrint("LIBRE\n");
	}
}

void verification(void* data){
	uint8_t err;

	safePrint("[VERIFICATION] Tache lancee\n");
	/* while(1){
		/*TODO: Synchronisation unilaterale avec timer 3s
		if (stopSimDebordement){
			/*TODO: Suspension de toutes les taches de la simulation
		}
	}
*/

}
void remplirAvion(Avion* avion) {
	srand(OSTimeGet());
	avion->retard = rand() % BORNE_SUP_HIGH;
	avion->origine = rand() % NB_AEROPORTS;
	do { avion->destination = rand() % NB_AEROPORTS; } while (avion->origine == avion->destination);
	/*xil_printf("Avion retard = %d\n", avion->retard);
	xil_printf("Avion origine = %d\n", avion->origine);
	xil_printf("Avion destination = %d\n", avion->destination);*/
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
	errMsg(err, "Error while trying to access mutexPrint");

	xil_printf(printMsg);

	err = OSMutexPost(mutexPrint);
	errMsg(err, "Error while trying to post mutexPrint");
}
