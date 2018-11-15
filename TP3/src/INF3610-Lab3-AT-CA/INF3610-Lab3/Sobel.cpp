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
	sensitive << clk.pos();
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

	// Attendre signal pour lire
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

	// Attendre signal pour ecrire
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
	unsigned int readAddress = 0;
	unsigned int data = 0x00000000;
	unsigned int position = 0;

	while (true)
	{
		readAddress = 0;

		// Lecture de l'image

		imgWidth = read(readAddress);
		readAddress += 4;
		imgHeight = read(readAddress);
		readAddress += 4;
		unsigned int imgSize = imgWidth * imgHeight;

		uint8_t * image = new uint8_t[imgSize]();

		// Lecture de tous les pixels de l'image
		for (unsigned int i = 0; i < imgHeight; i++) {
			for (unsigned int j = 0; j < imgWidth; j += 4) {
				position = j + i * imgWidth;
				data = read(position + readAddress);	// Les premiers octets ne correspondent pas a des pixels
				image[position] = data & 0x000000FF;	// Read renvoie 4 octets, donc 4 pixels a la fois	
				image[position + 1] = data & 0x0000FF00;
				image[position + 2] = data & 0x00FF0000;
				image[position + 3] = data & 0xFF000000;
			}
		}
		
		// Fin de la lecture, Debut de l'ecriture

		for (unsigned int i = 0; i < imgHeight; i++) {
			for (unsigned int j = 0; j < imgWidth; j += 4) {
				position = j + i * imgWidth;
				
				// A la premiere et a la derniere rangee, on met du blanc
				if (i == 0 || i == imgHeight - 1) {
					write(position + readAddress, 0);
				}
				else {
					// Premier pixel
					unsigned int pixel1 = 0;
					if (j != 0) {	// A la premiere colonne, on met du blanc
						pixel1 = sobel_operator(position, imgWidth, image);
					}

					// Deuxieme pixel
					unsigned int pixel2 = sobel_operator(position + 1, imgWidth, image) << 8;

					// Troisieme pixel
					unsigned int pixel3 = sobel_operator(position + 2, imgWidth, image) << 16;

					// Quatrieme pixel
					unsigned int pixel4 = 0;
					if (j < imgWidth - 4) { // A la derniere colonne, on met du blanc
						pixel4 = sobel_operator(position + 3, imgWidth, image) << 24;
					}

					data = pixel1 + pixel2 + pixel3 + pixel4;
					write(position + readAddress, data);
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


