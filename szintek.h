#ifndef szintek_h
#define szintek_h

#include <stdio.h>
#include <stdbool.h>

/**
 * Egy cella a p�ly�n. A k�perny�n 3 karakter hossz� �s 1 karakter magas.
 */
typedef enum {epulet, /**< �p�let */
              utca,   /**< Utca, amin nem j�rt m�g a j�t�kos */
              u_jart  /**< Utca, amin j�rt m�r a j�t�kos */
} Cella;

/**
 * Egy jelenlegi poz�ci� a cell�k m�trix�ban. Lehet j�t�kos vagy "k�pzeletbeli" pont.
 */
typedef struct {
    int x,  /**< X koordin�ta (0-t�l indexelve) */
    y;      /**< Y koordin�ta (0-t�l indexelve) */
} Pozicio;

/**
 * A szintek t�mbj�t �s m�reteit tartalmaz� strukt�ra. H�romdimenzi�s dinamikus t�mb.
 * Ez a dinamikus t�mb nem lesz �tm�retezve a program fut�sa sor�n, maximum felszabad�tva majd �jra
 * felt�ltve.
 *
 * Utols� elem: terkep[szintszam][mag][hossz]
 *
 * Gyakori haszn�lat: terkep[aktiv_szint][y][x]
 */
typedef struct {
    Cella ***terkep;    /**< A h�romdimenzi�s, dinamikusan foglalt t�mb pointere. */
    int szintszam;      /**< A szintek sz�ma */
    int mag;            /**< A szintek magass�ga */
    int hossz;          /**< A szintek hossza */
    int aktiv_szint;    /**< A jelenleg akt�v szint sorsz�ma, 0-t�l indexelve */
    int iranykonstansok[4][2];  /**< Azok a konstansok, amit egy pont koordin�t�ihoz adva azt a megadott ir�nyba mozd�tjuk el eggyel.
                                 * Az ir�nyok sorrendje megegyezik az Irany enum-mal.
                                 * Pl. ha balra akarjuk mozd�tani a pontot, akkor:
                                 * p.x += iranykonstansok[3][0]; (-1)
                                 * p.y += iranykonstansok[3][1]; (0)
                                 * Az �tvonalkeres� f�ggv�nyek k�z�l sokan haszn�lj�k, ez�rt c�lszer� a szintek adataival egy strukt�r�ba tenni,
                                 * �gy minden f�ggv�nyben el�rhet�, ahol sz�ks�g van r�. */
} Szintek;

/**
 * \cond Doxygen hagyja ki ezt a r�szt, a c f�jlban vannak rendesen dokument�lva ezek a f�ggv�nyek.
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