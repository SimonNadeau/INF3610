///////////////////////////////////////////////////////////////////////////////
//
//	Sobel.cpp
//
///////////////////////////////////////////////////////////////////////////////
#include "Sobel.h"


///////////////////////////////////////////////////////////////////////////////
//
//	Constructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobel::Sobel( sc_module_name name ) : sc_module(name) /* À compléter */
{
	/* À compléter */
	SC_THREAD(thread);
	//sensitive << clk.pos;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Destructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobel::~Sobel()
{
	/* À compléter */
}

// Exemple de synchronisation "Handshaking"
unsigned int Sobel::read(unsigned int readAddress) {
	unsigned int dataToRead = 0;
	address.write(readAddress);
	requestRead.write(true);

	// Wait for rising edge clk
	do {
		wait(clk->posedge_event());
	} while (!ack.read());

	dataToRead = data.read();
	requestRead.write(false);

	return dataToRead;

}

void Sobel::write(unsigned int writeAddress, unsigned int dataToWrite) {
	address.write(writeAddress);
	data.write(dataToWrite);
	requestWrite.write(true);

	// Wait for rising edge clk
	do {
		wait(clk->posedge_event());
	} while (!ack.read());

	requestWrite.write(false);
}


///////////////////////////////////////////////////////////////////////////////
//
//	thread
//
///////////////////////////////////////////////////////////////////////////////
void Sobel::thread(void)
{
	/* À compléter */
	unsigned int imgWidth = 0;
	unsigned int imgHeight = 0;
	unsigned int threadAddress = 0;
	unsigned int data = 0x00000000;
	unsigned int adress = 0;

	while (true)
	{
		// Lecture de l'image

		imgWidth = read(threadAddress);
		threadAddress += 4;
		imgHeight = read(threadAddress);
		unsigned int imgSize = imgWidth * imgHeight;

		uint8_t * image = new uint8_t[imgSize];

		for (unsigned int i = 0; i < imgHeight; i++) {
			for (unsigned int j = 0; j < imgWidth; j += 4) {
				address = j + i*imgWidth;
				data = read(address + 8);
				image[address] = data & 0x000000FF;
				image[address + 1] = data & 0x0000FF00;
				image[address + 2] = data & 0x00FF0000;
				image[address + 3] = data & 0xFF000000;
			}
		}
		
		// Fin de la lecture

		for (unsigned int i = 0; i < imgHeight; i++) {
			for (unsigned int j = 0; j < imgWidth; j += 4) {
				// Si c'est sur le bord;
				address = j + i * imgWidth;
				if (i == 0 || i == imgHeight - 1) {
					write(8 + address, 0);
				}
				else {
					wait(48); // Valeur non arbitraire. A justifier

					// bits 0-7
					unsigned int bits0to7 = 0;
					if (j != 0) {
						bits0to7 = sobel_operator(address, imgWidth, image);
					}

					// bits 8-15
					unsigned int bits8to15 = sobel_operator(address + 1, imgWidth, image) << 8;

					// bits 16-23
					unsigned int bits16to23 = sobel_operator(address + 2, imgWidth, image) << 16;

					// bits 24-31
					unsigned int bits24to31 = 0;
					if (j != imgWidth - 4) {
						bits24to31 = sobel_operator(address + 3, imgWidth, image) << 24;
					}

					data = bits0to7 + bits8to15 + bits16to23 + bits24to31;
					write(8 + address, data);
				}
			}
		}
		// Fin écriture

		delete[] image;
		sc_stop();

	}

}


///////////////////////////////////////////////////////////////////////////////
//
//	sobel_operator
//
///////////////////////////////////////////////////////////////////////////////
static inline uint8_t getVal(int index, int xDiff, int yDiff, int img_width, uint8_t * Y) 
{ 
	return Y[index + (yDiff * img_width) + xDiff]; 
};

uint8_t Sobel::sobel_operator(const int index, const int imgWidth, uint8_t * image)
{
	int x_weight = 0;
	int y_weight = 0;

	unsigned edge_weight;
	uint8_t edge_val;

	const char x_op[3][3] = {	{ -1,0,1 },
								{ -2,0,2 },
								{ -1,0,1 } };

	const char y_op[3][3] = {	{ 1,2,1 },
								{ 0,0,0 },
								{ -1,-2,-1 } };

	//Compute approximation of the gradients in the X-Y direction
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		// X direction gradient
		x_weight = x_weight + (getVal(index, i - 1, j - 1, imgWidth, image) * x_op[i][j]);

		// Y direction gradient
		y_weight = y_weight + (getVal(index, i - 1, j - 1, imgWidth, image) * y_op[i][j]);
		}
	}

	edge_weight = std::abs(x_weight) + std::abs(y_weight);

	edge_val = (255 - (uint8_t)(edge_weight));

	//Edge thresholding
	if (edge_val > 200)
		edge_val = 255;
	else if (edge_val < 100)
		edge_val = 0;

	return edge_val;
}


