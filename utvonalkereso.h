#ifndef NHF_UTVONALKERESO_H
#define NHF_UTVONALKERESO_H

#include <stdbool.h>
#include "szintek.h"

typedef struct Csucs Csucs;

/**
 * Egy cs�cs egy �l�t reprezent�l� strukt�ra.
 */
typedef struct {
    int suly;       /**< Az �l s�lya, azaz a k�t cs�cs t�vols�ga (mez�kben) */
    Csucs *csucs;   /**< Pointer a cs�csra, ahova az �l mutat */
} El;

/**
 * Egy cs�csot reprezent�l� strukt�ra
 */
struct Csucs{
    int x;          /**< A cs�cs x koordin�t�ja */
    int y;          /**< A cs�cs y koordin�t�ja */
    int tavolsag;   /**< A cs�cs t�vols�ga a kezd� cs�cst�l. Amennyiben a cs�cs nem a c�l cs�cs,
                    *  akkor a Dijkstra-algoritmus fut�sa ut�n se biztos, hogy helyes �rt�k,
                    *  de ezeknek a cs�csoknak a t�vols�ga nem is �rdekel benn�nket. */
    bool vizsgalt;  /**< Vizsg�ltuk-e m�r a cs�csot a Dijkstra-algoritmusban? */
    int elozo;      /**< Az elek[elozo] adja meg azt az �lt, amivel eljuthatunk az el�z� cs�cshoz,
                    *  ha a legr�videbb �ton akarunk visszajutni a kezd� cs�csba.
                    *  Ha a cs�cs nem r�sze a legr�videbb �tnak a c�l �s a rajt k�z�tt, akkor az algoritmus fut�sa ut�n se biztos, hogy helyes az �rt�ke.
                    *  Am�g nem tudjuk, melyik az el�z� cs�cs, addig �rt�ke 5. (de �gy sosem haszn�ljuk) */
    El elek[4];     /**< A cs�csb�l kiindul� �lek t�mbje. A sorrend az Irany enummal megegyez� (fel, balra, le, jobbra) */
    bool cel;       /**< Ez a cs�cs a c�l-e? */
    bool rajt;      /**< Ez a cs�cs a rajt-e? */
};

/**
 * A n�gy lehets�ges ir�ny a j�t�kban. Minden f�ggv�ny, ami egy cs�cs �leit veszi sorba, ebben a sorrendben halad.
 */
typedef enum {
    fel = 0,
    jobb = 1,
    le = 2,
    bal = 3,
    egyikse = 4
} Irany;
/**
 * \cond Doxygen hagyja ki ezta r�szt, a c f�jlban vannak rendesen dokument�lva ezek a f�ggv�nyek.
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
