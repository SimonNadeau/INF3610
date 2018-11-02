///////////////////////////////////////////////////////////////////////////////
//
//	main.cpp
//
///////////////////////////////////////////////////////////////////////////////
#include <systemc.h>
#include "Sobel.h"
#include "Reader.h"
#include "Writer.h"
#include "DataRAM.h"

#define RAMSIZE 0x200000

// Global variables
bool m_bError = false;

///////////////////////////////////////////////////////////////////////////////
//
//	Main
//
///////////////////////////////////////////////////////////////////////////////
int sc_main(int arg_count, char **arg_value)
{
	// Variables
	int sim_units = 2; //SC_NS 
	
	// Components
	// TODO: Instanciation des modules
	Sobel Sobel("Sobel");
	Reader Reader("Reader");
	Writer Writer("Writer");
	DataRAM DataRAM("DataRAM", "image.mem", RAMSIZE, false);

	// Connexions
	//TODO: Connexions des ports des modules
	Reader.dataPortRAM(DataRAM);
	Sobel.readPort(Reader);
	Sobel.writePort(Writer);
	Writer.dataPortRAM(DataRAM);

	// Démarrage de l'application
	if (!m_bError)
	{
		cout << "Démarrage de la simulation." << endl;	
		sc_start( -1, sc_core::sc_time_unit(sim_units) );
		cout << endl << "Simulation s'est terminée à " << sc_time_stamp();
	}
	// Fin du programme
	return 0;
}
