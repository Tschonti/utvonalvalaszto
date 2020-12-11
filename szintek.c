#include "szintek.h"
#include "egyeb.h"
#include "toplista.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "econio.h"

/*! @file szintek.c
 * \brief A szintek betöltését és kezelését leíró függvényeket tartlmazó modul.

 */

/**
 * Betölti a "levels.fs" fájlból a szinteket egy dinamikusan foglalt, háromdimenziós tömbbe.
 * Ha a fájlt nem tudja megnyitni, vagy érvénytelen, akkor kilép a programból a kilep() függvénnyel.
 * @return A Szintek méreteit és tömbjét tartalmazó struktúra
 */
Szintek szintek_betolt(void) {
    Szintek meretek = {NULL, 0, 0, 0, 0, {
            {0, -1},    // Fel
            {1, 0},     // Jobbra
            {0, 1},     // Le
            {-1, 0}     // Balra
    }};

    FILE *fp = fopen("levels.fs", "r");
    if (fp == NULL) {
        fclose(fp);
        kilep(14, "A szinteket tartalmazó fájl érvénytelen vagy nem létezik!\n", meretek);
    }
    if (fscanf(fp, "%d %d %d", &(meretek.szintszam), &(meretek.mag), &(meretek.hossz)) != 3) {
        fclose(fp);
        kilep(15, "A szinteket tartalmazó fájl érvénytelen!\n", meretek);
    }
    fgetc(fp);
    //fgetc(fp);    //Linuxon szükséges ez is!!!

    meretek.terkep = cella_tomb_foglal(meretek);

    for (int szint = 0; szint < meretek.szintszam; szint++) {
        for (int sor = 0; sor < meretek.mag; sor++) {
            for (int cella = 0; cella < meretek.hossz; cella++) {
                char c = fgetc(fp);
                if (c == 'E') {
                    meretek.terkep[szint][sor][cella] = epulet;
                } else if (c == 'U') {
                    meretek.terkep[szint][sor][cella] = utca;
                } else {
                    fclose(fp);
                    kilep(16, "A szinteket tartalmazó fájl érvénytelen!", meretek);
                }
            }
            fgetc(fp);
            //fgetc(fp);    //Linuxon szükséges ez is!!!
        }
        fgetc(fp);
        //fgetc(fp);    //Linuxon szükséges ez is!!!
    }

    fclose(fp);
    return meretek;
}

/**
 * Elindítja a játékot a megadott szintrõl és a megadott kezdõponttal.
 * Végigviszi a játékost az összes szinten a kovi_szint() függvénnyel, ha a játékos nem lép ki közben.
 * Miután az összes szinttel végzett, betölti a toplistát és új eredményt vesz fel az uj_eredmeny() függvénnyel.
 * Végül kiírja a toplistát a képernyõre, és felszabadítja az elfoglalt memóriaterületeket a toplista_nyomtat() függvénnyel.
 *
 * @param szint Errõl a szintrõl fog indulni a játék, 0-tól indexelve
 * @param pont  Ennyi ponttal kezdi a játékos a játékot. Mindkettõ 0, ha új játékot kezd a felhasználó.
 */
void jatek_indul(int szint, int pont) {
    Szintek meretek = szintek_betolt();
    meretek.aktiv_szint = szint;
    while (meretek.aktiv_szint < meretek.szintszam) {
        int idealis = 0;
        int lepesek = kovi_szint(meretek, &idealis);
        printf("A te útvonalad: %d m%c", lepesek*10, 9);
        printf("Az ideális útvonal: %d m%c", idealis*10, 9);
        int igazipont;
        int ujpont;
        if (lepesek == 0) {
            igazipont = 0;
            ujpont = 0;
        } else {
            ujpont = (int) round(((((double) idealis / (double) (lepesek)) / 5) - 0.1) * 100);
            igazipont = ujpont > 0 ? ujpont : 0;
        }
        pont += igazipont;
        printf("Elért pontok: %d%c", ujpont, 9);
        printf("Összes pont: %d\n", pont);
        meretek.aktiv_szint++;
        if (meretek.aktiv_szint < meretek.szintszam) {
            printf("1. Következõ szint\n");
            printf("2. Játék mentése és kilépés\n");
            if (szam_beolvas(1, 2) == 2) {
                jatek_ment(pont, meretek);
                econio_clrscr();
                break;
            }
        }
    }
    if (meretek.aktiv_szint == meretek.szintszam) {
        printf("Játék vége!\n");
        printf("Lássuk a végeredményt! (Nyomj ENTER-t!)");
        scanf("%*[^\n]"); getchar();
        printf("Végeredmény: %d pont\n", pont);
        Toplista tl;
        toplista_betolt(&tl, meretek);
        uj_eredmeny(&tl, pont, meretek);
        toplista_nyomtat(tl);
    }
    cella_tomb_szabadit(meretek);
}
/**
 * A paraméterben megadot szint indítása. Folyamatosan nyomtatja a térképet, számolja az eltelt idõt és a lépéseket, valamit mozgatja a játékos karakterét.
 * Egészen addig, amíg az idõ lejár vagy a játékos eléri a célt. Ekkor meghatározza a legrövidebb útvonalat a legrovidebb() fügvénnyel, és meg is mutatja a játékosnak.
 * A szint futása közben ESC nyomására kilép a programból a kilep() fuggvénnyel.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @param idealis Pointer egy integerre, ahova menti a függvény az adott szint ideális útvonalának hosszát.
 * @return A szerzett pontok száma
 */
static int kovi_szint(Szintek meretek, int *idealis) {
    Pozicio p = {0, 0};
    econio_clrscr();
    int max_ido = 20;
    int idozito = max_ido;

    palya_nyomtat(p, meretek, idozito);
    clock_t kezdo_t, jelen_t, eltelt_t;
    kezdo_t = clock();
    int lepesek = 0;

    econio_rawmode();
    while (true) {
        jelen_t = clock();
        eltelt_t = ((double) (jelen_t - kezdo_t)/CLOCKS_PER_SEC) - (max_ido - idozito);

        if (eltelt_t >= 1) {
            idozito--;
        }
        if (econio_kbhit()) {
            int ch = econio_getch();
            if (ch == KEY_ESCAPE)
                kilep(0, "Köszi a játékot!", meretek);
            Irany irany = egyikse;
            switch (ch) {
                case KEY_UP:
                    irany = fel;
                    break;
                case KEY_RIGHT:
                    irany = jobb;
                    break;
                case KEY_DOWN:
                    irany = le;
                    break;
                case KEY_LEFT:
                    irany = bal;
            }
            if (irany != egyikse) {
                if (!checkif_building((Pozicio) {p.x + meretek.iranykonstansok[irany][0], p.y + meretek.iranykonstansok[irany][1]}, meretek)) {
                    meretek.terkep[meretek.aktiv_szint][p.y][p.x] = u_jart;
                    p.x += meretek.iranykonstansok[irany][0];
                    p.y += meretek.iranykonstansok[irany][1];
                    lepesek++;
                }
            }
        }
        palya_nyomtat(p, meretek, idozito);
        if (checkif_finish(p, meretek)) {
            break;
        }
        if (idozito == 0) {
            lepesek = 0;
            break;
        }
    }
    printf("\n");
    econio_normalmode();
    bool **idut = legrovidebb(meretek, idealis);
    palya_vegso_nyomtat(p, meretek, idozito, idut);
    free(idut[0]);
    free(idut);
    return lepesek;
}
/**
 * Konzolba nyomtatja az aktuális pályát.
 * @param p Pozicio struktúr, a játékos jelenlegi x és y pozíciója.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @param time Hátralevõ idõ másodpercben
 */
static void palya_nyomtat(Pozicio p, Szintek meretek, int time) {
    econio_gotoxy(0, 0);
    econio_textcolor(COL_WHITE);
    econio_textbackground(COL_MAGENTA);
    printf("RAJT\n");
    econio_textcolor(COL_MAGENTA);

    for (int sor = 0; sor < meretek.mag; sor++) {
        for (int cella = 0; cella < meretek.hossz; cella++) {
            char* print = "   ";
            if ( sor == p.y && cella == p.x) {
                print = " o ";
            } else if (meretek.terkep[meretek.aktiv_szint][sor][cella] == u_jart) {
                print = " • ";
            }
            if (meretek.terkep[meretek.aktiv_szint][sor][cella] == epulet) {
                econio_textbackground(COL_DARKGRAY);
                printf("%s", print);
            } else {
                econio_textbackground(COL_YELLOW);
                printf("%s", print);
            }
        }
        printf("\n");
    }
    econio_textbackground(COL_BLACK);
    for (int i = 0; i < meretek.hossz - 1; i++) {
        printf("   ");
    }
    econio_textcolor(COL_WHITE);
    econio_textbackground(COL_MAGENTA);
    printf("CÉL\n");
    econio_textbackground(COL_BLACK);
    if (time <= 5) {
        econio_textcolor(COL_RED);
    }
    printf("Hátralévõ idõ: %2d mp\n\n", time);
    econio_textcolor(COL_RESET);
}

/**
 * A palya_nyomtat() függvényhez hasonlóan kinyomtatja a képernyõre a pályát, de nem csak a játkos útvonalával, hanem az ideális útvonallal is.
 * @param p A játékos pozíciója
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @param time Hátralévõ idõ másodpercben (ilyenkor ez már fix, nem változik)
 * @param idealis Kétdimenziós bool tömb, aminek minden mezõje pontosan akkor igaz, ha az ideális útvonal átmegy azon a mezõn.
 */
static void palya_vegso_nyomtat(Pozicio p, Szintek meretek, int time, bool **idealis) {
    econio_gotoxy(0, 0);
    econio_textcolor(COL_WHITE);
    econio_textbackground(COL_MAGENTA);
    printf("RAJT\n");
    econio_textcolor(COL_MAGENTA);

    for (int sor = 0; sor < meretek.mag; sor++) {
        for (int cella = 0; cella < meretek.hossz; cella++) {
            char* print = "   ";
            if (meretek.terkep[meretek.aktiv_szint][sor][cella] != epulet) {
                if ( sor == p.y && cella == p.x) {
                    econio_textcolor(COL_MAGENTA);
                    print = " o ";
                } else if ( idealis[sor][cella] && meretek.terkep[meretek.aktiv_szint][sor][cella] == u_jart) {
                    econio_textcolor(COL_GREEN);
                    print = " • ";
                } else if (idealis[sor][cella]) {
                    econio_textcolor(COL_BLUE);
                    print = " • ";
                } else if (meretek.terkep[meretek.aktiv_szint][sor][cella] == u_jart) {
                    econio_textcolor(COL_RED);
                    print = " • ";
                }
            }
            if (meretek.terkep[meretek.aktiv_szint][sor][cella] == epulet) {
                econio_textbackground(COL_DARKGRAY);
                printf("%s", print);
            } else {
                econio_textbackground(COL_YELLOW);
                printf("%s", print);
            }
        }
        printf("\n");
    }
    econio_textbackground(COL_BLACK);
    for (int i = 0; i < meretek.hossz - 1; i++) {
        printf("   ");
    }
    econio_textcolor(COL_WHITE);
    econio_textbackground(COL_MAGENTA);
    printf("CÉL\n");
    econio_textbackground(COL_BLACK);
    if (time <= 5) {
        econio_textcolor(COL_RED);
    }
    printf("Hátralévõ idõ: %2d mp\n", time);
    econio_textcolor(COL_RESET);

    printf("Jelmagyarázat:\n");
    econio_textbackground(COL_YELLOW);
    econio_textcolor(COL_RED);
    printf(" • ");
    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
    printf("  A te útvonalad\n");

    econio_textbackground(COL_YELLOW);
    econio_textcolor(COL_BLUE);
    printf(" • ");
    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
    printf("  Az ideális útvonal\n");

    econio_textbackground(COL_YELLOW);
    econio_textcolor(COL_GREEN);
    printf(" • ");
    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
    printf("  Ahol a kettõ egybeesik\n");

    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
}

/**
 * Megnézi, hogy a játékos éppen épület mezõn áll-e, vagy a pályán kívül van-e.
 * Akkor ad vissza igazat, ha a játékos olyan mezõn áll, ahol nem állhatna.
 * @param p Játékos jelenlegi pozíciója
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @return Igaz, ha épület mezõn vagy a pályán kívül áll a játékos, különben hamis
 */
static bool checkif_building(Pozicio p, Szintek meretek) {
    if (p.y < 0 || p.y >= meretek.mag || p.x < 0 || p.x >= meretek.hossz) {
        return true;
    }
    return meretek.terkep[meretek.aktiv_szint][p.y][p.x] == epulet;
}

/**
 * Megnézi, hogy a játékos éppen a cél mezõn áll-e (bal alsó sarok)
 * @param p Játékos jelenlegi pozíciója
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 * @return Igaz, ha a játékos a célban van, egyébként hamis.
 */
static bool checkif_finish(Pozicio p, Szintek meretek) {
    return p.x == meretek.hossz - 1 && p.y == meretek.mag - 1;
}
