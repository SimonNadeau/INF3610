///////////////////////////////////////////////////////////////////////////////
//
//	Sobelv2.cpp
//
///////////////////////////////////////////////////////////////////////////////
#include "Sobelv2.h"


///////////////////////////////////////////////////////////////////////////////
//
//	Constructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobelv2::Sobelv2( sc_module_name name ) : sc_module(name)
/* À compléter */
{
	SC_THREAD(thread);
	sensitive << clk.pos();
}


///////////////////////////////////////////////////////////////////////////////
//
//	Destructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobelv2::~Sobelv2()
{
	// À compléter
}

// Exemple de synchronisation "Handshaking"
unsigned int Sobelv2::read(unsigned int readAddress) {
	unsigned int dataToRead = 0;
	address.write(readAddress);
	requestRead.write(true);

	// Attendre signal pour lire
	do {
		wait(clk->posedge_event());
	} while (!ackReaderWriter.read());

	dataToRead = dataRW.read();
	requestRead.write(false);

	return dataToRead;

}

void Sobelv2::readCache(unsigned int readAddress, unsigned int* writeCacheAddress, unsigned int readLength) {
	address.write(readAddress);
	addressRes.write(writeCacheAddress);
	length.write(readLength);
	requestCache.write(true);

	// Attendre signal pour lire
	do {
		wait(clk->posedge_event());
	} while (!ackCache.read());

	requestCache.write(false);
}

void Sobelv2::write(unsigned int writeAddress, unsigned int dataToWrite) {
	address.write(writeAddress);
	dataRW.write(dataToWrite);
	requestWrite.write(true);

	// Attendre signal pour ecrire
	do {
		wait(clk->posedge_event());
	} while (!ackReaderWriter.read());

	requestWrite.write(false);
}

///////////////////////////////////////////////////////////////////////////////
//
//	thread
//
///////////////////////////////////////////////////////////////////////////////
void Sobelv2::thread(void)
{
	/* À compléter */

	unsigned int imgWidth = 0;
	unsigned int imgHeight = 0;
	unsigned int data = 0x00000000;
	uint8_t* image = NULL;
	uint8_t* cache = NULL;
	unsigned int position = 0;

	while (true) {
		unsigned int readAddress = 0;

		imgWidth = read(readAddress);
		readAddress += 4;
		imgHeight = read(readAddress);
		readAddress += 4;
		unsigned int prevBytes = readAddress;

		unsigned int cacheSize = 4 * imgWidth;
		cache = new uint8_t[cacheSize]();	// Tampon d'une taille de 4 lignes de l'image
		image = new uint8_t[imgHeight * imgWidth]();	// On garde l'image en entier en memoire

		// Premiere lecture pour remplir la cache
		readCache(readAddress, (unsigned int*)cache, 3 * imgWidth);
		readAddress += 3 * imgWidth;

		// Alternance lecture - calcul de l'image

		for (unsigned int i = 0; i < imgHeight; i++) {
			// Lecture de la prochaine ligne de l'image
			if (i != 0 && i != imgHeight - 1) {
				unsigned int* writeCacheAddress = (unsigned int*)cache + (readAddress - prevBytes) % cacheSize / 4;
				readCache(readAddress, writeCacheAddress, imgWidth);
				readAddress += imgWidth;
			}

			// Calcul
			unsigned int cacheIndex = 0;
			for (unsigned int j = 0; j < imgWidth; j++) {
				position = i * imgWidth + j;

				// Sur la bordure, on met du blanc
				if (i == 0 || j == 0 || i == imgHeight - 1 || j == imgWidth - 1) {
					image[position] = 0;
				}
				else {
					cacheIndex = (readAddress - prevBytes - 3 * imgWidth) % cacheSize + j;
					image[position] = Sobelv2_operator(cacheIndex, imgWidth, cache);
				}
			}
		}

		// Ecriture de l'image

		for (unsigned int i = 0; i < imgHeight; i++) {
			for (unsigned int j = 0; j < imgWidth; j += 4) {
				position = i * imgWidth + j;
				data = image[position] + (image[position + 1] << 8) + (image[position + 2] << 16) +( image[position + 3] << 24);
				write(prevBytes + position, data);
			}
		}

		delete[] cache;
		delete[] image;
		sc_stop();
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//	Sobelv2_operator
//
///////////////////////////////////////////////////////////////////////////////
static inline uint8_t getVal(int index, int xDiff, int yDiff, int img_width, uint8_t * Y)
{
	int fullIndex = (index + (yDiff * img_width)) + xDiff;
	if (fullIndex < 0)
	{
		//Cas ou on doit chercher la derniere ligne
		fullIndex += img_width * 4;
	}
	else if (fullIndex >= img_width * 4)
	{
		//Cas ou on doit aller chercher la premiere ligne
		fullIndex -= img_width * 4;
	}

	return Y[fullIndex];
};

uint8_t Sobelv2::Sobelv2_operator(const int index, const int imgWidth, uint8_t * image)
{
	int x_weight = 0;
	int y_weight = 0;

	unsigned edge_weight;
	uint8_t edge_val;

	const char x_op[3][3] = { { -1,0,1 },
	{ -2,0,2 },
	{ -1,0,1 } };

	const char y_op[3][3] = { { 1,2,1 },
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