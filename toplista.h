#ifndef NHF_TOPLISTA_H
#define NHF_TOPLISTA_H

#include "szintek.h"

/**
 * Egy eredm�ny strukt�r�ja, ami a toplist�ra ker�lhet.
 */
typedef struct {
    int hely;      /**< Helyez�s, 0-t�l indexelve */
    int pont;      /**< Pontsz�m */
    char nev[50];  /**< N�v (maximum 50 karakter) */
} Eredmeny;

/**
 * A toplista dinamikus (egydimenzi�s) t�mbbje.
 */
typedef struct {
    int meret;      /**< A t�mb m�rete, maximum 10 */
    Eredmeny *hs;   /**< A dinamikusan foglalt t�mb */
} Toplista;

/**
 * \cond Doxygen hagyja ki ezta r�szt, a c f�jlban vannak rendesen dokument�lva ezek a f�ggv�nyek.
 */
void toplista_betolt(Toplista *tl, Szintek meretek);
void uj_eredmeny(Toplista *tl, int pont, Szintek meretek);
void toplista_nyomtat(Toplista tl);

static void eredmeny_felvesz(Toplista *tl, Eredmeny new, Szintek meretek);
static void toplista_fajlba(Toplista tl);

/**
 * \endcond
 */
#endif
