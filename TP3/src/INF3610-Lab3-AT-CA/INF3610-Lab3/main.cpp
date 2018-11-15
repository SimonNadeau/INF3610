///////////////////////////////////////////////////////////////////////////////
//
//	main.cpp
//
///////////////////////////////////////////////////////////////////////////////
#include <systemc.h>
#include "Sobel.h"
#include "Sobelv2.h"
#include "Reader.h"
#include "Writer.h"
#include "DataRAM.h"
#include "CacheMem.h"

#define RAMSIZE 0x200000

///////////////////////////////////////////////////////////////////////////////
//
//	Main
//
///////////////////////////////////////////////////////////////////////////////
int sc_main(int arg_count, char **arg_value)
{
	// Variables
	int sim_units = 2; //SC_NS

	// Clock
	const sc_core::sc_time_unit simTimeUnit = SC_NS;
	const int clk_freq = 4000;
	sc_clock clk("SysClock", clk_freq, simTimeUnit, 0.5);

	// Components
	Reader reader("Reader");
	DataRAM dataRAM("DataRAM", "image.mem", RAMSIZE, false);
	//TODO : Déclaration du module de l'écrivain
	Writer writer("Writer");

	// Signals
	sc_signal<unsigned int, SC_MANY_WRITERS> data;
	sc_signal<unsigned int, SC_MANY_WRITERS> address;
	sc_signal<unsigned int*> addressData;
	sc_signal<unsigned int> length;
	sc_signal<bool, SC_MANY_WRITERS> reqRead;
	sc_signal<bool, SC_MANY_WRITERS> reqWrite;
	sc_signal<bool, SC_MANY_WRITERS> ackReaderWriter;
	sc_signal<bool, SC_MANY_WRITERS> ackCache;
	sc_signal<bool, SC_MANY_WRITERS> reqCache;


	/* à compléter*/

	// Connexions
	reader.clk(clk);
	reader.data(data);
	reader.address(address);
	reader.request(reqRead);
	reader.ack(ackReaderWriter);
	reader.dataPortRAM(dataRAM);

	/* à compléter */
	writer.clk(clk);
	writer.data(data);
	writer.address(address);
	writer.request(reqWrite);
	writer.ack(ackReaderWriter);
	writer.dataPortRAM(dataRAM);

	const bool utiliseCacheMem = false;

	if (!utiliseCacheMem) {
		Sobel sobel("Sobel");

		/* à compléter */
		sobel.clk(clk);
		sobel.data(data);
		sobel.address(address);
		sobel.ack(ackReaderWriter);
		sobel.requestRead(reqRead);
		sobel.requestWrite(reqWrite);

		// Démarrage de l'application
		cout << "Démarrage de la simulation." << endl;
		sc_start(-1, sc_core::sc_time_unit(sim_units));
		cout << endl << "Simulation s'est terminée à " << sc_time_stamp();
	} else {
		Sobelv2 sobel2("Sobelv2");
		CacheMem cacheMem("CacheMem");

		/* à compléter*/
		sobel2.ackCache(ackCache);
		sobel2.ackReaderWriter(ackReaderWriter);
		sobel2.address(address);
		sobel2.addressRes(addressData);
		sobel2.clk(clk);
		sobel2.dataRW(data);
		sobel2.length(length);
		sobel2.requestCache(reqCache);
		sobel2.requestRead(reqRead);
		sobel2.requestWrite(reqWrite);

		cacheMem.ackFromReader(ackReaderWriter);
		cacheMem.ackToCPU(ackCache);
		cacheMem.address(address);
		cacheMem.addressData(addressData);
		cacheMem.clk(clk);
		cacheMem.dataReader(data);
		cacheMem.length(length);
		cacheMem.requestFromCPU(reqCache);
		cacheMem.requestToReader(reqRead);

		// Démarrage de l'application
		cout << "Démarrage de la simulation." << endl;
		sc_start(-1, sc_core::sc_time_unit(sim_units));
		cout << endl << "Simulation s'est terminée à " << sc_time_stamp();

	}

	return 0;
}
