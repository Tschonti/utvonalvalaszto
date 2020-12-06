#ifndef szintek_h
#define szintek_h

#include <stdio.h>
#include <stdbool.h>

/**
 * Egy cella a pályán. A képernyõn 3 karakter hosszú és 1 karakter magas.
 */
typedef enum {epulet, /**< Épület */
              utca,   /**< Utca, amin nem járt még a játékos */
              u_jart  /**< Utca, amin járt már a játékos */
} Cella;

/**
 * Egy jelenlegi pozíció a cellák mátrixában. Lehet játékos vagy "képzeletbeli" pont.
 */
typedef struct {
    int x,  /**< X koordináta (0-tól indexelve) */
    y;      /**< Y koordináta (0-tól indexelve) */
} Pozicio;

/**
 * A szintek tömbjét és méreteit tartalmazó struktúra. Háromdimenziós dinamikus tömb.
 * Ez a dinamikus tömb nem lesz átméretezve a program futása során, maximum felszabadítva majd újra
 * feltöltve.
 *
 * Utolsó elem: terkep[szintszam][mag][hossz]
 *
 * Gyakori használat: terkep[aktiv_szint][y][x]
 */
typedef struct {
    Cella ***terkep;    /**< A háromdimenziós, dinamikusan foglalt tömb pointere. */
    int szintszam;      /**< A szintek száma */
    int mag;            /**< A szintek magassága */
    int hossz;          /**< A szintek hossza */
    int aktiv_szint;    /**< A jelenleg aktív szint sorszáma, 0-tól indexelve */
    int iranykonstansok[4][2];  /**< Azok a konstansok, amit egy pont koordinátáihoz adva azt a megadott irányba mozdítjuk el eggyel.
                                 * Az irányok sorrendje megegyezik az Irany enum-mal.
                                 * Pl. ha balra akarjuk mozdítani a pontot, akkor:
                                 * p.x += iranykonstansok[3][0]; (-1)
                                 * p.y += iranykonstansok[3][1]; (0)
                                 * Az útvonalkeresõ függvények közül sokan használják, ezért célszerû a szintek adataival egy struktúrába tenni,
                                 * így minden függvényben elérhetõ, ahol szükség van rá. */
} Szintek;

/**
 * \cond Doxygen hagyja ki ezt a részt, a c fájlban vannak rendesen dokumentálva ezek a függvények.
 */
Szintek szintek_betolt(void);
void jatek_indul(int szint, int pont);
static int kovi_szint(Szintek meretek, int *idealis);
static void palya_nyomtat(Pozicio p, Szintek meretek, int time);
static void palya_vegso_nyomtat(Pozicio p, Szintek meretek, int time, bool **idealis);

static bool checkif_building(Pozicio p, Szintek meretek);
static bool checkif_finish(Pozicio p, Szintek meretek);

/**
 * \endcond
 */
#endif