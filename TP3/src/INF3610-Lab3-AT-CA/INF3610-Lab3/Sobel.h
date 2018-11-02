///////////////////////////////////////////////////////////////////////////////
//
//	Sobel.h
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <systemc.h>
#include <stdio.h>


///////////////////////////////////////////////////////////////////////////////
//
//	Class Sobel
//
///////////////////////////////////////////////////////////////////////////////
class Sobel : sc_module /* À compléter */
{
	public: 
		// Ports
		sc_in_clk				clk;
		sc_inout<unsigned int>  data;
		sc_out<unsigned int>  	address;
		sc_out<bool>			requestRead;
		sc_out<bool>			requestWrite;
		sc_in<bool>				ack;

	
		// Constructor
		Sobel( sc_module_name name );
		
		// Destructor
		~Sobel();
		
	private:
		// Process SystemC
		/* À compléter */
		SC_HAS_PROCESS(Sobel);
		
		void thread(void);
		uint8_t Sobel::sobel_operator(const int index, const int imgWidth, uint8_t * image);
		//unsigned int Sobel::Read(unsigned int addresse);
		//void Sobel::Write(unsigned int addresse, unsigned int dataToWrite);
};
