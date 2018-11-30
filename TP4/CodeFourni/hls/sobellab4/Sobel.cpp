#include "Sobel.h"
#include <string.h>

#define ABS(x)          ((x>0)? x : -x)

typedef union {
	uint8_t pix[4];
	unsigned full;
} OneToFourPixels;


uint8_t sobel_operator(const int fullIndex, uint8_t * image)
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
	return 0;
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
//#pragma HLS INTERFACE m_axi port=inter_pix offset=slave bundle=gmem0
//#pragma HLS INTERFACE m_axi port=out_pix offset=slave bundle=gmem1
//#pragma HLS INTERFACE s_axilite port=return

	// � remplacer par votre fonction *apr�s* avoir r�pondu aux questions initiales
IMG: for (int i = 0; i < IMG_WIDTH * IMG_HEIGHT; ++i) {
		uint8_t val = inter_pix[i];
		OneToFourPixels fourWide;
OneTo4:	for (int j = 0; j < 4; ++j)
			fourWide.pix[j] = val;
		out_pix[i] = fourWide.full;
	}
}
