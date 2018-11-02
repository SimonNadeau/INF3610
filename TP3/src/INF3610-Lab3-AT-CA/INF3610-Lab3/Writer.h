///////////////////////////////////////////////////////////////////////////////
//
//	Writer.h
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <systemc.h>
#include "LMBIF.h"

///////////////////////////////////////////////////////////////////////////////
//
//	Class Writer
//
///////////////////////////////////////////////////////////////////////////////
class Writer : sc_channel
{
public:
	// Ports
	sc_in_clk				clk;
	sc_in<unsigned int>  	address;
	sc_out<unsigned int>  	data;
	sc_in<bool>				request;
	sc_out<bool>			ack;
	sc_port<LMBIF>			dataPortRAM;

	// Constructor
	Writer(sc_module_name name);

	// Destructor
	~Writer();

private:
	// Process SystemC
	SC_HAS_PROCESS(Writer);

	void thread(void);
};


