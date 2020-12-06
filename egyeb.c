#include "egyeb.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "debugmalloc.h"

/*! @file egyeb.c
 * \brief Egyéb függvényeket tartalmazó modul.
    Ide tartozik többek között a megkezdett játékok mentése és betöltése, a dinamikus tömbök foglalása és szabdítása,
    valamint a kilépõ függvény.

 */

/**
 * Bekér a felhasználótól egy számot. Ha az nincs [min, max]-ban, akkor újra kér. Egyébként visszaadja a számot. Az InfoC-s tutorial alapján.
 * @param min Várt szám alsó korlátja
 * @param max Várt szám felsõ korlátja
 * @return Felhasználó választása, ami garantáltan [min, max]-ban van
 */
int szam_beolvas(int min, int max) {
    printf("Írd be a választásod számát! ");
    while (true) {
        int olvasott;
        int sikerult = scanf("%d", &olvasott); getchar();
        if (sikerult != 1) {
            printf("Amit megadtál, nem szám volt!\n");
            scanf("%*[^\n]");
        } else if (olvasott > max || olvasott < min) {
            printf("A megadott szám nem volt megfelelõ! (min: %d, max: %d)\n", min, max);
        } else {
            return olvasott;
        }
    }
}

/**
 * Fájlba ment egy játékot.
 * @param pont Eddig szerzett pontok
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 */
void jatek_ment(int pont, Szintek meretek) {
    FILE *fp = fopen("savegame.fs", "w");
    if (fp == NULL) {
        kilep(13, "A játékmentés fájlja érvénytelen!", meretek);
    }
    fprintf(fp, "%d %d", meretek.aktiv_szint, pont);
    fclose(fp);
}

/**
 * Betölt egy korábban lementett játékot fájlból, majd törli a fájlból ezt a mentést.
 * @param szint Ebbe a változóba menti a teljesített szintek számát
 * @param pont Ebbe a változóba menti az eddigi pontszámot
 * @return 0, ha sikeres a betöltés, 1 ha sikertelen (nem volt mentés, vagy érvénytelen)
 */
int jatek_betolt(int *szint, int *pont) {
    FILE *fp = fopen("savegame.fs", "r");
    if (fp == NULL) {
        printf("A lementett játék fájlja érvénytelen vagy nem található!\n");
        return 1;
    }
    if (fscanf(fp, "%d %d", szint, pont) != 2) {
        fclose(fp);
        printf("Nem található lementett játék, vagy a fájlja érvénytelen!\n");
        return 1;
    }
    fclose(fp);
    FILE *fpw = fopen("savegame.fs", "w");
    fprintf(fpw, "");
    fclose(fpw);
    return 0;
}
/**
 * Kiírja a segítséget a képernyõre.
 */
void segitseg(void) {
    printf("SEGÍTSÉG\n\n"
           "Üdvözöllek az Útvonalválaszt-O nevû játékomban!\n"
           "A játék célja, hogy felmérje a képességed a városi tájfutás egyik kulcselemébõl, az útvonalválasztásból.\n"
           "Ha többet szeretnél megtudni arról, hogy ez pontosan mi is, vagy a játék pontos használati utasításait szeretnéd \n"
           "elolvasni, akkor ajánlom figyelmedbe a felhasználói dokumentációt! De ne aggódj, az alapokat itt is le fogom írni.\n"
           " A játék lényege, hogy minden szinten megtaláld a legrövidebb utat a rajt és a cél között. \n"
           "A rajt mindig a bal felsõ sarokban lesz, innen indul a karakteredet reprezentáló \"o\" karakter, amit a nyilakkal \n"
           "tudsz irányítani. A sárga mezõk az utcák, ezeken tudsz haladni, a szürkék pedig az épületek, ezeket ki kell kerülnöd. \n"
           "A cél mindig a jobb alsó sarokban lesz. Minden szintre 20 másodperced van, ha ez az idõ lejár és még nem értél \n"
           "el a célba, 0 pontot kapsz a szintért. Ha eljutsz a célba, akkor az alapján kapsz pontot, hogy az általad bejárt\n"
           " útvonal mennyivel volt hosszabb az ideális útvonalnál. (Tehát a felhasznált idõ nem számít a pontba, \n"
           "csak annyi a lényeg, hogy beérj a 20 mp-n belül). Az ideális útvonalat látni fogod, miután célba értél. \n"
           "Minden szint után lehetõséged van kilépni, és késõbb folytatni a játékot. Amikor végeztél az összes szinttel,\n"
           " a program megnézni, hogy az összpontszámod alapján felkerültél-e a toplistára, és ha igen, akkor kérni fogja a neved.\n"
           "Jó szórakozást!\n");
}

/**
 * Foglal egy dinamikus, háromdimenziós tömböt.
 * Ha nincs elég memória, kilép, az eddig foglalt területeket felszabadítva.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @return A dinamikus tömb, amit a meghívónak kell felszabadítania a cella_tomb_szabadit() függvénnyel.
 */
Cella*** cella_tomb_foglal(Szintek meretek) {
    Cella ***uj = (Cella ***) malloc(meretek.szintszam * sizeof(Cella**));
    if (uj == NULL) {
        kilep(2, "", (Szintek) {NULL});
    }
    uj[0] = (Cella**) malloc(meretek.szintszam * meretek.mag * sizeof(Cella*));
    if (uj[0] == NULL) {
        free(uj);
        kilep(2, "", (Szintek) {NULL});
    }

    for (int i = 0; i < meretek.szintszam; i++) {
        uj[i] = uj[0] + i * meretek.mag;
    }

    for (int i = 0; i < meretek.szintszam; i++) {
        uj[i][0] = (Cella *) malloc(meretek.mag * meretek.hossz * sizeof(Cella));
        if (uj[i][0] == NULL) {
            for (int j = 0; j < i; j++) {
                free(uj[j][0]);
            }
            free(uj[0]);
            free(uj);
            kilep(2, "", (Szintek) {NULL});
        }
    }

    for (int i = 0; i < meretek.szintszam; i++) {
        for (int j = 0; j < meretek.mag; j++) {
            uj[i][j] = uj[i][0] + j * meretek.hossz;
        }

    }
    return uj;
}

/**
 * Foglal egy dinamikus, kétdimenziós tömböt, aminek minden eleme egy csúcsra mutató pointer lesz, ezeket mind NULL-ra állítja.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @return A foglalt tömb, amit a hívónak kell felszabadítania
 */
Csucs ***csucs_tomb_foglal(Szintek meretek) {
    Csucs ***uj = (Csucs ***) malloc(meretek.mag * sizeof(Csucs**));
    if (uj == NULL) {
        kilep(2, "", meretek);
    }
    uj[0] = (Csucs**) malloc(meretek.hossz * meretek.mag * sizeof(Csucs*));
    if (uj[0] == NULL) {
        free(uj);
        kilep(2, "", meretek);
    }

    for (int i = 0; i < meretek.mag; i++) {
        uj[i] = uj[0] + i * meretek.hossz;
    }

    for (int i = 0; i < meretek.mag; i++) {
        for (int j = 0; j < meretek.hossz; j++) {
            uj[i][j] = NULL;
        }
    }
    return uj;
}

/**
 * Foglal egy dinamikus, kétdimenziós tömböt, aminek minden eleme egy boolean érték, ezeket mind hamisra állítja.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @return A foglalt tömb, amit a hívónak kell felszabadítania
 */
bool **bool_tomb_foglal(Szintek meretek) {
    bool **uj = (bool **) malloc(meretek.mag * sizeof(bool *));
    if (uj == NULL) {
        kilep(2, "", meretek);
    }
    uj[0] = (bool *) malloc(meretek.hossz * meretek.mag * sizeof(bool));
    if (uj[0] == NULL) {
        free(uj);
        kilep(2, "", meretek);
    }

    for (int i = 0; i < meretek.mag; i++) {
        uj[i] = uj[0] + i * meretek.hossz;
    }

    for (int i = 0; i < meretek.mag; i++) {
        for (int j = 0; j < meretek.hossz; j++) {
            uj[i][j] = false;
        }
    }
    return uj;
}

/**
 * Felszabadítja a meretek struktúra dinamukis tömbjét.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 */
void cella_tomb_szabadit(Szintek meretek) {
    for (int i = 0; i < meretek.szintszam; i++) {
        free(meretek.terkep[i][0]);
    }
    free(meretek.terkep[0]);
    free(meretek.terkep);
}

/**
 * Kilép a programból a megadott hibakóddal és hibaüzenettel, és azt az errorlog.txt fájlba is menti.
 * Felszabadítja a dinamikus tömböt, hacsak annak pointere nem NULL. Ha úgy akarjuk meghívni a függvényt,
 * hogy a meretek.terkep már fel lett szabadítva, vagy még nincs lefoglalva, akkor a meretek paraméternek adjunk (Szintek) {NULL}-t!
 * Mivel 2-es (memóriahiány) kóddal sokszor van meghívva a függvény, ennek a hibaüzenete itt van megadva, ilyenkor mes lehet NULL.
 * Figyelem! Ez a függvény egyes IDE-kben sok figyelmeztetést okozhat. Ha ez a függvény valahol meg lett hívva,
 * akkor ott biztosan véget ér a program futása, hiszen ennek a függvénynek exit() a vége. Így ezen függvény meghívása után fellépõ figyelmeztetések
 * nem okozhatnak problémát, hiszen oda már nem is fog eljutni a program.
 * @param code A hibakód
 * @param mes A hibaüzenet, maximum 100 karakter.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 */
void kilep(int code, char mes[100], Szintek meretek) {
    if (code == 2) {
        mes = "Nincs elég memória!";
    }
    FILE *fp = fopen("errorlog.txt", "w");
    if (fp == NULL) {
        printf("Még a hibajelzõ is elromlott :( \n");
    } else {
        if (code != 0) {
            fprintf(fp, "%s\nHibakód: %d\nNézd meg a dokumentációt, segít a hiba kijavításában!", mes, code);
        } else {
            fprintf(fp, "");
        }
    }
    fclose(fp);

    printf("%s\n", mes);
    if (meretek.terkep != NULL) {
        cella_tomb_szabadit(meretek);
    }
    exit(code);
}

/**
 * Kiírja a menü opcióit, majd bekéri a felhasználó választását a szam_beolvas() függvénnyel.
 * @return a felhasználó választása az opciók közül 
 */
int menu(void) {
    printf("\nÚtvonalválaszt-O\n\n");

    printf("1. Új játék\n");
    printf("2. Játék folytatása\n");
    printf("3. Segítség\n");
    printf("4. Toplista\n");
    printf("5. Kilépés\n\n");

    return szam_beolvas(1, 5);
}