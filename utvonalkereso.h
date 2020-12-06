#ifndef NHF_UTVONALKERESO_H
#define NHF_UTVONALKERESO_H

#include <stdbool.h>
#include "szintek.h"

typedef struct Csucs Csucs;

/**
 * Egy csúcs egy élét reprezentáló struktúra.
 */
typedef struct {
    int suly;       /**< Az él súlya, azaz a két csúcs távolsága (mezõkben) */
    Csucs *csucs;   /**< Pointer a csúcsra, ahova az él mutat */
} El;

/**
 * Egy csúcsot reprezentáló struktúra
 */
struct Csucs{
    int x;          /**< A csúcs x koordinátája */
    int y;          /**< A csúcs y koordinátája */
    int tavolsag;   /**< A csúcs távolsága a kezdõ csúcstól. Amennyiben a csúcs nem a cél csúcs,
                    *  akkor a Dijkstra-algoritmus futása után se biztos, hogy helyes érték,
                    *  de ezeknek a csúcsoknak a távolsága nem is érdekel bennünket. */
    bool vizsgalt;  /**< Vizsgáltuk-e már a csúcsot a Dijkstra-algoritmusban? */
    int elozo;      /**< Az elek[elozo] adja meg azt az élt, amivel eljuthatunk az elõzõ csúcshoz,
                    *  ha a legrövidebb úton akarunk visszajutni a kezdõ csúcsba.
                    *  Ha a csúcs nem része a legrövidebb útnak a cél és a rajt között, akkor az algoritmus futása után se biztos, hogy helyes az értéke.
                    *  Amíg nem tudjuk, melyik az elõzõ csúcs, addig értéke 5. (de így sosem használjuk) */
    El elek[4];     /**< A csúcsból kiinduló élek tömbje. A sorrend az Irany enummal megegyezõ (fel, balra, le, jobbra) */
    bool cel;       /**< Ez a csúcs a cél-e? */
    bool rajt;      /**< Ez a csúcs a rajt-e? */
};

/**
 * A négy lehetséges irány a játékban. Minden függvény, ami egy csúcs éleit veszi sorba, ebben a sorrendben halad.
 */
typedef enum {
    fel = 0,
    jobb = 1,
    le = 2,
    bal = 3,
    egyikse = 4
} Irany;
/**
 * \cond Doxygen hagyja ki ezta részt, a c fájlban vannak rendesen dokumentálva ezek a függvények.
 */

bool **legrovidebb(Szintek meretek, int *idealis);

static void graf_epit(Csucs* uj, Csucs ***vizsgalt, Szintek meretek, int *meret);
static void graf_kilapit(Csucs *cs, Csucs ***lista, int *n, bool **vizsgalt, int hossz);
static void dijkstra(Csucs **lista, int meret);
static void laposgraf_szabadit(Csucs ***lista, int meret);

static bool utca_teszt(Pozicio p, Szintek meretek);
static int utak_szama(Pozicio p, Szintek meretek);
static void kovi_utca(Pozicio *regi, Pozicio *uj, Szintek meretek);
static Irany irany_hataroz(Pozicio p);
static void csucs_init(Csucs *cs, Pozicio p, Szintek meretek);

static bool van_nem_latogatott(Csucs **lista, int meret);
static int legkozelebbi(Csucs **lista, int meret);
static bool **matrix_letrehoz(Csucs **lista, int meret, Szintek meretek, int *idealis);

/**
 * \endcond
 */
#endif
