#include "egyeb.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "debugmalloc.h"

/*! @file egyeb.c
 * \brief Egy�b f�ggv�nyeket tartalmaz� modul.
    Ide tartozik t�bbek k�z�tt a megkezdett j�t�kok ment�se �s bet�lt�se, a dinamikus t�mb�k foglal�sa �s szabd�t�sa,
    valamint a kil�p� f�ggv�ny.

 */

/**
 * Bek�r a felhaszn�l�t�l egy sz�mot. Ha az nincs [min, max]-ban, akkor �jra k�r. Egy�bk�nt visszaadja a sz�mot. Az InfoC-s tutorial alapj�n.
 * @param min V�rt sz�m als� korl�tja
 * @param max V�rt sz�m fels� korl�tja
 * @return Felhaszn�l� v�laszt�sa, ami garant�ltan [min, max]-ban van
 */
int szam_beolvas(int min, int max) {
    printf("�rd be a v�laszt�sod sz�m�t! ");
    while (true) {
        int olvasott;
        int sikerult = scanf("%d", &olvasott); getchar();
        if (sikerult != 1) {
            printf("Amit megadt�l, nem sz�m volt!\n");
            scanf("%*[^\n]");
        } else if (olvasott > max || olvasott < min) {
            printf("A megadott sz�m nem volt megfelel�! (min: %d, max: %d)\n", min, max);
        } else {
            return olvasott;
        }
    }
}

/**
 * F�jlba ment egy j�t�kot.
 * @param pont Eddig szerzett pontok
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 */
void jatek_ment(int pont, Szintek meretek) {
    FILE *fp = fopen("savegame.fs", "w");
    if (fp == NULL) {
        kilep(13, "A j�t�kment�s f�jlja �rv�nytelen!", meretek);
    }
    fprintf(fp, "%d %d", meretek.aktiv_szint, pont);
    fclose(fp);
}

/**
 * Bet�lt egy kor�bban lementett j�t�kot f�jlb�l, majd t�rli a f�jlb�l ezt a ment�st.
 * @param szint Ebbe a v�ltoz�ba menti a teljes�tett szintek sz�m�t
 * @param pont Ebbe a v�ltoz�ba menti az eddigi pontsz�mot
 * @return 0, ha sikeres a bet�lt�s, 1 ha sikertelen (nem volt ment�s, vagy �rv�nytelen)
 */
int jatek_betolt(int *szint, int *pont) {
    FILE *fp = fopen("savegame.fs", "r");
    if (fp == NULL) {
        printf("A lementett j�t�k f�jlja �rv�nytelen vagy nem tal�lhat�!\n");
        return 1;
    }
    if (fscanf(fp, "%d %d", szint, pont) != 2) {
        fclose(fp);
        printf("Nem tal�lhat� lementett j�t�k, vagy a f�jlja �rv�nytelen!\n");
        return 1;
    }
    fclose(fp);
    FILE *fpw = fopen("savegame.fs", "w");
    fprintf(fpw, "");
    fclose(fpw);
    return 0;
}
/**
 * Ki�rja a seg�ts�get a k�perny�re.
 */
void segitseg(void) {
    printf("SEG�TS�G\n\n"
           "�dv�z�llek az �tvonalv�laszt-O nev� j�t�komban!\n"
           "A j�t�k c�lja, hogy felm�rje a k�pess�ged a v�rosi t�jfut�s egyik kulcselem�b�l, az �tvonalv�laszt�sb�l.\n"
           "Ha t�bbet szeretn�l megtudni arr�l, hogy ez pontosan mi is, vagy a j�t�k pontos haszn�lati utas�t�sait szeretn�d \n"
           "elolvasni, akkor aj�nlom figyelmedbe a felhaszn�l�i dokument�ci�t! De ne agg�dj, az alapokat itt is le fogom �rni.\n"
           " A j�t�k l�nyege, hogy minden szinten megtal�ld a legr�videbb utat a rajt �s a c�l k�z�tt. \n"
           "A rajt mindig a bal fels� sarokban lesz, innen indul a karakteredet reprezent�l� \"o\" karakter, amit a nyilakkal \n"
           "tudsz ir�ny�tani. A s�rga mez�k az utc�k, ezeken tudsz haladni, a sz�rk�k pedig az �p�letek, ezeket ki kell ker�ln�d. \n"
           "A c�l mindig a jobb als� sarokban lesz. Minden szintre 20 m�sodperced van, ha ez az id� lej�r �s m�g nem �rt�l \n"
           "el a c�lba, 0 pontot kapsz a szint�rt. Ha eljutsz a c�lba, akkor az alapj�n kapsz pontot, hogy az �ltalad bej�rt\n"
           " �tvonal mennyivel volt hosszabb az ide�lis �tvonaln�l. (Teh�t a felhaszn�lt id� nem sz�m�t a pontba, \n"
           "csak annyi a l�nyeg, hogy be�rj a 20 mp-n bel�l). Az ide�lis �tvonalat l�tni fogod, miut�n c�lba �rt�l. \n"
           "Minden szint ut�n lehet�s�ged van kil�pni, �s k�s�bb folytatni a j�t�kot. Amikor v�gezt�l az �sszes szinttel,\n"
           " a program megn�zni, hogy az �sszpontsz�mod alapj�n felker�lt�l-e a toplist�ra, �s ha igen, akkor k�rni fogja a neved.\n"
           "J� sz�rakoz�st!\n");
}

/**
 * Foglal egy dinamikus, h�romdimenzi�s t�mb�t.
 * Ha nincs el�g mem�ria, kil�p, az eddig foglalt ter�leteket felszabad�tva.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @return A dinamikus t�mb, amit a megh�v�nak kell felszabad�tania a cella_tomb_szabadit() f�ggv�nnyel.
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
 * Foglal egy dinamikus, k�tdimenzi�s t�mb�t, aminek minden eleme egy cs�csra mutat� pointer lesz, ezeket mind NULL-ra �ll�tja.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @return A foglalt t�mb, amit a h�v�nak kell felszabad�tania
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
 * Foglal egy dinamikus, k�tdimenzi�s t�mb�t, aminek minden eleme egy boolean �rt�k, ezeket mind hamisra �ll�tja.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 * @return A foglalt t�mb, amit a h�v�nak kell felszabad�tania
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
 * Felszabad�tja a meretek strukt�ra dinamukis t�mbj�t.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 */
void cella_tomb_szabadit(Szintek meretek) {
    for (int i = 0; i < meretek.szintszam; i++) {
        free(meretek.terkep[i][0]);
    }
    free(meretek.terkep[0]);
    free(meretek.terkep);
}

/**
 * Kil�p a programb�l a megadott hibak�ddal �s hiba�zenettel, �s azt az errorlog.txt f�jlba is menti.
 * Felszabad�tja a dinamikus t�mb�t, hacsak annak pointere nem NULL. Ha �gy akarjuk megh�vni a f�ggv�nyt,
 * hogy a meretek.terkep m�r fel lett szabad�tva, vagy m�g nincs lefoglalva, akkor a meretek param�ternek adjunk (Szintek) {NULL}-t!
 * Mivel 2-es (mem�riahi�ny) k�ddal sokszor van megh�vva a f�ggv�ny, ennek a hiba�zenete itt van megadva, ilyenkor mes lehet NULL.
 * Figyelem! Ez a f�ggv�ny egyes IDE-kben sok figyelmeztet�st okozhat. Ha ez a f�ggv�ny valahol meg lett h�vva,
 * akkor ott biztosan v�get �r a program fut�sa, hiszen ennek a f�ggv�nynek exit() a v�ge. �gy ezen f�ggv�ny megh�v�sa ut�n fell�p� figyelmeztet�sek
 * nem okozhatnak probl�m�t, hiszen oda m�r nem is fog eljutni a program.
 * @param code A hibak�d
 * @param mes A hiba�zenet, maximum 100 karakter.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 */
void kilep(int code, char mes[100], Szintek meretek) {
    if (code == 2) {
        mes = "Nincs el�g mem�ria!";
    }
    FILE *fp = fopen("errorlog.txt", "w");
    if (fp == NULL) {
        printf("M�g a hibajelz� is elromlott :( \n");
    } else {
        if (code != 0) {
            fprintf(fp, "%s\nHibak�d: %d\nN�zd meg a dokument�ci�t, seg�t a hiba kijav�t�s�ban!", mes, code);
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
 * Ki�rja a men� opci�it, majd bek�ri a felhaszn�l� v�laszt�s�t a szam_beolvas() f�ggv�nnyel.
 * @return a felhaszn�l� v�laszt�sa az opci�k k�z�l 
 */
int menu(void) {
    printf("\n�tvonalv�laszt-O\n\n");

    printf("1. �j j�t�k\n");
    printf("2. J�t�k folytat�sa\n");
    printf("3. Seg�ts�g\n");
    printf("4. Toplista\n");
    printf("5. Kil�p�s\n\n");

    return szam_beolvas(1, 5);
}