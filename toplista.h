#ifndef NHF_TOPLISTA_H
#define NHF_TOPLISTA_H

#include "szintek.h"

/**
 * Egy eredmény struktúrája, ami a toplistára kerülhet.
 */
typedef struct {
    int hely;      /**< Helyezés, 0-tól indexelve */
    int pont;      /**< Pontszám */
    char nev[50];  /**< Név (maximum 50 karakter) */
} Eredmeny;

/**
 * A toplista dinamikus (egydimenziós) tömbbje.
 */
typedef struct {
    int meret;      /**< A tömb mérete, maximum 10 */
    Eredmeny *hs;   /**< A dinamikusan foglalt tömb */
} Toplista;

/**
 * \cond Doxygen hagyja ki ezta részt, a c fájlban vannak rendesen dokumentálva ezek a függvények.
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
