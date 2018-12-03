#include "Sobel.h"
#include <string.h>
#include <malloc.h>


#define ABS(x)          ((x>0)? x : -x)

typedef union {
	uint8_t pix[4];
	unsigned full;
} OneToFourPixels;

static inline uint8_t getVal(int index, int xDiff, int yDiff, int img_width, uint8_t * Y)
{
	return Y[index + (yDiff * img_width) + xDiff];
};

uint8_t sobel_operator(const int fullIndex, uint8_t image[IMG_SIZE])
{

#pragma HLS inline			// Inliner la fonction lui permet d'�tre "copi�e-coll�e" l� o� elle est appell�e
							// et ainsi faciliter le pipelinage de la boucle principale
	/* � compl�ter en important votre code du lab 3.
	 * � noter que la fonction peut avoir 3 signatures diff�rentes, selon vos diff�rentes modifications:
	 * uint8_t sobel_operator(const int fullIndex, uint8_t * image)
	 * uint8_t sobel_operator(const int fullIndex, uint8_t image[IMG_HEIGHT * IMG_WIDTH])
	 * uint8_t sobel_operator(const int col, const int row, uint8_t image[IMG_HEIGHT][IMG_WIDTH])
	 *
	 * Les deux premi�res sont assez �quivalentes, mais la derni�re permet d'acc�der � l'image comme un
	 * tableau 2D. Par contre, un tableau 2D doit alors lui �tre pass�, ce qui n'est pas �vident consid�rant
	 * que les entr�es de la fonction sobel_filtrer() sont 1D. Cependant, si pour une raison ou une autre
	 * un buffer-cache interm�diaire �tait utilis�, celui-ci pourrait �tre 2D...
	 */
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
		x_weight = x_weight + (getVal(fullIndex, i - 1, j - 1, IMG_WIDTH, image) * x_op[i][j]);

		// Y direction gradient
		y_weight = y_weight + (getVal(fullIndex, i - 1, j - 1, IMG_WIDTH, image) * y_op[i][j]);
		}
	}

	edge_weight = ABS(x_weight) + ABS(y_weight);

	edge_val = (255 - (uint8_t)(edge_weight));

	//Edge thresholding
	if (edge_val > 200)
		edge_val = 255;
	else if (edge_val < 100)
		edge_val = 0;

	return edge_val;
}


void sobel_filter(uint8_t inter_pix[IMG_WIDTH * IMG_HEIGHT], unsigned out_pix[IMG_WIDTH * IMG_HEIGHT])
{
	/* On demande � HLS de nous synth�tiser des ma�tres AXI que l'on connectera � la m�moire principale.
	 * Ainsi, le CPU n'a pas besoin de transf�rer l'image au filtre: c'est le filtre qui va chercher l'image
	 * dans la m�moire principale (DDR de la carte) et �crit le r�sultat dans cette m�me m�moire.
	 * Un esclave AXI-Lite est aussi cr��, accessible par le CPU, pour informer le filtre des adresses
	 * auxquelles il doit aller chercher et �crire l'image, lui dire de d�marrer ou d'arr�ter, etc.
	 */
	// ***** LES 3 LIGNES SUIVANTES DOIVENT �TRE D�COMMENT�ES UNE FOIS LES QUESTIONS INITIALES COMPL�T�ES!! ******
#pragma HLS INTERFACE m_axi port=inter_pix offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=out_pix offset=slave bundle=gmem1
#pragma HLS INTERFACE s_axilite port=return

	unsigned int address = 0;

	//Create array
	//uint8_t image[IMG_SIZE];
	//uint8_t result[IMG_SIZE];

	uint8_t * image = (uint8_t*)malloc(IMG_HEIGHT * IMG_WIDTH * sizeof(unsigned char));
	uint8_t * result = (uint8_t*)malloc(IMG_HEIGHT * IMG_WIDTH * sizeof(unsigned char));

	for (unsigned int i = 0; i < IMG_SIZE; i++) {
		//Request element
		image[i] = inter_pix[i];
		//address += 4;
		// imageAsInt[i] = readPort->Read(address);
	}

	//For simplicity, assume that the borders don't contain edges
	for (unsigned int i = 0; i < IMG_WIDTH; ++i)
		result[i] = 0;
	for (unsigned int i = IMG_SIZE - IMG_WIDTH; i < IMG_SIZE; ++i)
		result[i] = 0;
	for (unsigned int i = 0; i < IMG_SIZE; i += IMG_WIDTH)
		result[i] = 0;
	for (unsigned int i = IMG_WIDTH - 1; i < IMG_SIZE; i += IMG_WIDTH)
		result[i] = 0;

	//Calling the operator for each pixel
	for (unsigned int i = 1; i < IMG_HEIGHT - 1; ++i) {
		for (unsigned int j = 1; j < IMG_WIDTH - 1; ++j) {
			int fullIndex = i * IMG_WIDTH + j;
			result[fullIndex] = sobel_operator(fullIndex, image);
		}
	}

	//Write back nb. elements at the end
	address = 0;
	for (unsigned int i = 0; i < IMG_SIZE; i++) {
		//Write each element
		uint8_t val = result[i];
		OneToFourPixels fourWide;

		for (int j = 0; j < 4; ++j){
			fourWide.pix[j] = val;
		}
		out_pix[i] = fourWide.full;
	}

/*
	// � remplacer par votre fonction *apr�s* avoir r�pondu aux questions initiales
IMG: for (int i = 0; i < IMG_WIDTH * IMG_HEIGHT; ++i) {
		uint8_t val = inter_pix[i];
		OneToFourPixels fourWide;
OneTo4:	for (int j = 0; j < 4; ++j)
			fourWide.pix[j] = val;
		out_pix[i] = fourWide.full;
	}
*/
}
