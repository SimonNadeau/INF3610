/*
 * tourControleAvions.h
 *
 *  Created on: 14 sept. 2018
 *      Author: evter
 */

#ifndef SRC_SIMAVION_H_
#define SRC_SIMAVION_H_

#include <ucos_ii.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE       	16384            // Size of each task's stacks (# of WORDs)

#define	VERIFICATION_PRIO		3				// Defining Priority of each task
#define	GENERATION_PRIO			4
#define	STATISTIQUES_PRIO		5
#define	ATTERRISSAGE_PRIO		6
#define	TERMINAL1_PRIO			7
#define	TERMINAL2_PRIO			8
#define	DECOLLAGE_PRIO			9
#define	MUTEX_VERIFICATION_PRIO	10
#define MUTEX_DECOLLAGE_PRIO	11
#define MUTEX_ATTERRISSAGE_PRIO	12
#define MUTEX_TERMINAL1_PRIO	13
#define MUTEX_TERMINAL2_PRIO	14
#define MUTEX_PRINT_PRIO		15


//Intervalles criteres de retard (en minutes)
#define BORNE_INF_LOW      	0
#define BORNE_SUP_LOW      	19
#define BORNE_INF_MEDIUM   	20
#define BORNE_SUP_MEDIUM  	39
#define BORNE_INF_HIGH     	40
#define BORNE_SUP_HIGH     	60

/*
*********************************************************************************************************
*                                               QUEUES
*********************************************************************************************************
*/
OS_EVENT *Q_atterrissage_high;
void *Q_atterrissage_high_data[3];
OS_EVENT *Q_atterrissage_medium;
void *Q_atterrissage_medium_data[4];
OS_EVENT *Q_atterrissage_low;
void *Q_atterrissage_low_data[6];

OS_EVENT *Q_decollage;
void *Q_decollage_data[10];

OS_EVENT *Q_terminal1;
void *Q_terminal1_data[1];
OS_EVENT *Q_terminal2;
void *Q_terminal2_data[1];

/*
*********************************************************************************************************
*                                              MAILBOX
*********************************************************************************************************
*/

/*TODO: Declaration des mailbox (si necessaire)*/

/*
*********************************************************************************************************
*                                              FLAGS
*********************************************************************************************************
*/

/*TODO: Declaration des flags (si necessaire)*/

/*
*********************************************************************************************************
*                                              SEMAPHORES
*********************************************************************************************************
*/

/*TODO: Declaration des semaphores (si necessaire)*/

OS_EVENT* semGen;
OS_EVENT* semVer;
OS_EVENT* semStat;
OS_EVENT* mutexVerification;
OS_EVENT* mutexPrint;
OS_EVENT* mutexDecollage;
OS_EVENT* mutexAtterrissage;
OS_EVENT* mutexTerminal1;
OS_EVENT* mutexTerminal2;


/*
*********************************************************************************************************
*                                             ENUMERATIONS
*********************************************************************************************************
*/

enum Aeroport { YUL, YYZ, YVR, PEK, DBX, LGA, HND, LAX, CDG, AMS, NB_AEROPORTS };

/*
*********************************************************************************************************
*                                             STRUCTURES
*********************************************************************************************************
*/

typedef struct Avion {
	int id;
	int retard;
	enum Aeroport origine;
	enum Aeroport destination;
} Avion;

/*
*********************************************************************************************************
*                                             SHARED VAIRABLES
*********************************************************************************************************
*/
bool stopSimDebordement;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void create_application();
int create_tasks();
int create_events();

void	generation(void *data);
void	atterrissage(void *data);
void	processLanding();
void    terminal(void *data);
void    decollage(void *data);
void	remplirAvion(Avion* avion);
void	statistiques(void *data);
void 	verification(void* data);

void	signalOverflow(INT8U err);
void    errMsg(INT8U err, char* errMSg);
void 	safePrint(char* printMsg);


#endif /* SRC_SIMAVION_H_ */
