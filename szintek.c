#include "szintek.h"
#include "egyeb.h"
#include "toplista.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "econio.h"

/*! @file szintek.c
 * \brief A szintek bet�lt�s�t �s kezel�s�t le�r� f�ggv�nyeket tartlmaz� modul.

 */

/**
 * Bet�lti a "levels.fs" f�jlb�l a szinteket egy dinamikusan foglalt, h�romdimenzi�s t�mbbe.
 * Ha a f�jlt nem tudja megnyitni, vagy �rv�nytelen, akkor kil�p a programb�l a kilep() f�ggv�nnyel.
 * @return A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
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
        kilep(14, "A szinteket tartalmaz� f�jl �rv�nytelen vagy nem l�tezik!\n", meretek);
    }
    if (fscanf(fp, "%d %d %d", &(meretek.szintszam), &(meretek.mag), &(meretek.hossz)) != 3) {
        fclose(fp);
        kilep(15, "A szinteket tartalmaz� f�jl �rv�nytelen!\n", meretek);
    }
    fgetc(fp);
    //fgetc(fp);    //Linuxon sz�ks�ges ez is!!!

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
                    kilep(16, "A szinteket tartalmaz� f�jl �rv�nytelen!", meretek);
                }
            }
            fgetc(fp);
            //fgetc(fp);    //Linuxon sz�ks�ges ez is!!!
        }
        fgetc(fp);
        //fgetc(fp);    //Linuxon sz�ks�ges ez is!!!
    }

    fclose(fp);
    return meretek;
}

/**
 * Elind�tja a j�t�kot a megadott szintr�l �s a megadott kezd�ponttal.
 * V�gigviszi a j�t�kost az �sszes szinten a kovi_szint() f�ggv�nnyel, ha a j�t�kos nem l�p ki k�zben.
 * Miut�n az �sszes szinttel v�gzett, bet�lti a toplist�t �s �j eredm�nyt vesz fel az uj_eredmeny() f�ggv�nnyel.
 * V�g�l ki�rja a toplist�t a k�perny�re, �s felszabad�tja az elfoglalt mem�riater�leteket a toplista_nyomtat() f�ggv�nnyel.
 *
 * @param szint Err�l a szintr�l fog indulni a j�t�k, 0-t�l indexelve
 * @param pont  Ennyi ponttal kezdi a j�t�kos a j�t�kot. Mindkett� 0, ha �j j�t�kot kezd a felhaszn�l�.
 */
void jatek_indul(int szint, int pont) {
    Szintek meretek = szintek_betolt();
    meretek.aktiv_szint = szint;
    while (meretek.aktiv_szint < meretek.szintszam) {
        int idealis = 0;
        int lepesek = kovi_szint(meretek, &idealis);
        printf("A te �tvonalad: %d m%c", lepesek*10, 9);
        printf("Az ide�lis �tvonal: %d m%c", idealis*10, 9);
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
        printf("El�rt pontok: %d%c", ujpont, 9);
        printf("�sszes pont: %d\n", pont);
        meretek.aktiv_szint++;
        if (meretek.aktiv_szint < meretek.szintszam) {
            printf("1. K�vetkez� szint\n");
            printf("2. J�t�k ment�se �s kil�p�s\n");
            if (szam_beolvas(1, 2) == 2) {
                jatek_ment(pont, meretek);
                econio_clrscr();
                break;
            }
        }
    }
    if (meretek.aktiv_szint == meretek.szintszam) {
        printf("J�t�k v�ge!\n");
        printf("L�ssuk a v�geredm�nyt! (Nyomj ENTER-t!)");
        scanf("%*[^\n]"); getchar();
        printf("V�geredm�ny: %d pont\n", pont);
        Toplista tl;
        toplista_betolt(&tl, meretek);
        uj_eredmeny(&tl, pont, meretek);
        toplista_nyomtat(tl);
    }
    cella_tomb_szabadit(meretek);
}
/**
 * A param�terben megadot szint ind�t�sa. Folyamatosan nyomtatja a t�rk�pet, sz�molja az eltelt id�t �s a l�p�seket, valamit mozgatja a j�t�kos karakter�t.
 * Eg�szen addig, am�g az id� lej�r vagy a j�t�kos el�ri a c�lt. Ekkor meghat�rozza a legr�videbb �tvonalat a legrovidebb() f�gv�nnyel, �s meg is mutatja a j�t�kosnak.
 * A szint fut�sa k�zben ESC nyom�s�ra kil�p a programb�l a kilep() fuggv�nnyel.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @param idealis Pointer egy integerre, ahova menti a f�ggv�ny az adott szint ide�lis �tvonal�nak hossz�t.
 * @return A szerzett pontok sz�ma
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
                kilep(0, "K�szi a j�t�kot!", meretek);
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
 * Konzolba nyomtatja az aktu�lis p�ly�t.
 * @param p Pozicio strukt�r, a j�t�kos jelenlegi x �s y poz�ci�ja.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @param time H�tralev� id� m�sodpercben
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
                print = " � ";
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
    printf("C�L\n");
    econio_textbackground(COL_BLACK);
    if (time <= 5) {
        econio_textcolor(COL_RED);
    }
    printf("H�tral�v� id�: %2d mp\n\n", time);
    econio_textcolor(COL_RESET);
}

/**
 * A palya_nyomtat() f�ggv�nyhez hasonl�an kinyomtatja a k�perny�re a p�ly�t, de nem csak a j�tkos �tvonal�val, hanem az ide�lis �tvonallal is.
 * @param p A j�t�kos poz�ci�ja
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @param time H�tral�v� id� m�sodpercben (ilyenkor ez m�r fix, nem v�ltozik)
 * @param idealis K�tdimenzi�s bool t�mb, aminek minden mez�je pontosan akkor igaz, ha az ide�lis �tvonal �tmegy azon a mez�n.
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
                    print = " � ";
                } else if (idealis[sor][cella]) {
                    econio_textcolor(COL_BLUE);
                    print = " � ";
                } else if (meretek.terkep[meretek.aktiv_szint][sor][cella] == u_jart) {
                    econio_textcolor(COL_RED);
                    print = " � ";
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
    printf("C�L\n");
    econio_textbackground(COL_BLACK);
    if (time <= 5) {
        econio_textcolor(COL_RED);
    }
    printf("H�tral�v� id�: %2d mp\n", time);
    econio_textcolor(COL_RESET);

    printf("Jelmagyar�zat:\n");
    econio_textbackground(COL_YELLOW);
    econio_textcolor(COL_RED);
    printf(" � ");
    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
    printf("  A te �tvonalad\n");

    econio_textbackground(COL_YELLOW);
    econio_textcolor(COL_BLUE);
    printf(" � ");
    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
    printf("  Az ide�lis �tvonal\n");

    econio_textbackground(COL_YELLOW);
    econio_textcolor(COL_GREEN);
    printf(" � ");
    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
    printf("  Ahol a kett� egybeesik\n");

    econio_textcolor(COL_RESET);
    econio_textbackground(COL_RESET);
}

/**
 * Megn�zi, hogy a j�t�kos �ppen �p�let mez�n �ll-e, vagy a p�ly�n k�v�l van-e.
 * Akkor ad vissza igazat, ha a j�t�kos olyan mez�n �ll, ahol nem �llhatna.
 * @param p J�t�kos jelenlegi poz�ci�ja
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @return Igaz, ha �p�let mez�n vagy a p�ly�n k�v�l �ll a j�t�kos, k�l�nben hamis
 */
static bool checkif_building(Pozicio p, Szintek meretek) {
    if (p.y < 0 || p.y >= meretek.mag || p.x < 0 || p.x >= meretek.hossz) {
        return true;
    }
    return meretek.terkep[meretek.aktiv_szint][p.y][p.x] == epulet;
}

/**
 * Megn�zi, hogy a j�t�kos �ppen a c�l mez�n �ll-e (bal als� sarok)
 * @param p J�t�kos jelenlegi poz�ci�ja
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @return Igaz, ha a j�t�kos a c�lban van, egy�bk�nt hamis.
 */
static bool checkif_finish(Pozicio p, Szintek meretek) {
    return p.x == meretek.hossz - 1 && p.y == meretek.mag - 1;
}
