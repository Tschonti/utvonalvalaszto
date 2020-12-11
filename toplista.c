#include "toplista.h"
#include "egyeb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*! @file toplista.c
 * \brief A toplista bet�lt�s�t �s szerkeszt�s�t le�r� f�ggv�nyeket tartlmaz� modul.
 *
 */


/**
 * F�jlb�l bet�lti a lementett toplist�t.
 * Ha nem tal�lja, akkor l�trehozza.
 * Ha nem tudja megynitni a f�jlt, vagy az �rv�nytelen, kil�p a kilep() f�ggv�nnyel.
 * @param tl Dinamikus t�mb, amibe bet�lti az eredm�nyeket.
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 */
void toplista_betolt(Toplista *tl, Szintek meretek) {
    FILE *fp = fopen("toplista.fs", "r");

    if (fp == NULL) {
        FILE *fp2 = fopen("toplista.fs", "w");
        if (fp2 == NULL) {
            kilep(10, "�rv�nytelen toplista f�jl!", meretek);
        }
        fprintf(fp2, "%d\n", 0);
        fclose(fp2);
        fp = fopen("toplista.fs", "r");
    }

    if(fscanf(fp, "%d", &(tl->meret)) != 1) {
        fclose(fp);
        kilep(11, "�rv�nytelen toplista f�jl!", meretek);
    } else {
        fgetc(fp);

        tl->hs = (Eredmeny *) malloc(tl->meret * sizeof(Eredmeny));
        if (tl->hs == NULL) {
            fclose(fp);
            kilep(2, "", meretek);
        }

        for (int i = 0; i < tl->meret; i++) {
            if (fscanf(fp, "%d %d", &(tl->hs[i].hely), &(tl->hs[i].pont)) != 2) {
                free(tl->hs);
                fclose(fp);
                kilep(12, "�rv�nytelen toplista f�jl!", meretek);
            } else {
                fgetc(fp);
                fgets((tl->hs[i].nev), 51, fp);
                strtok(tl->hs[i].nev, "\n");
            }
        }
    }
    fclose(fp);
}

/**
 * Meg�llap�tja egy �j eredm�nyr�l, hogy az felker�l-e a toplist�ra. Ha igen, bek�ri a felhaszn�l�t�l a nev�t,
 * �s megh�vja az eredmeny_felvesz() f�ggv�nyt, majd az �j toplist�t f�jlba �rja a toplista_fajlba() f�ggv�nnyel.
 * @param tl Dinamikus t�mb
 * @param pont Az �j eredm�ny pontsz�ma
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 */
void uj_eredmeny(Toplista *tl, int pont, Szintek meretek) {
    int helyezes = tl->meret;
    for (int i = 0; i < tl->meret; i++) {
        if (pont >= tl->hs[i].pont) {
            helyezes = i;
            break;
        }
    }
    if (helyezes < 10) {
        printf("Gratul�lok, felker�lt�l a toplist�ra! K�rlek, add meg a neved (max 50 karkter)!\n");
        char name[51];
        fgets(name, 51, stdin);
        if (strlen(name) == 50) {
            scanf("%*[^\n]");
        }
        strtok(name, "\n");
        Eredmeny new;
        new.hely = helyezes;
        new.pont = pont;
        strcpy(new.nev, name);
        eredmeny_felvesz(tl, new, meretek);
        toplista_fajlba(*tl);
    }
    else {
        printf("Sz�p munka, de a toplist�ra sajnos nem ker�lt�l fel.\n");
    }
}

/**
 * Egy eredm�nyt berak a dinamikus t�mbbe. Ha sz�ks�ges, meg is ny�jtja a t�mb�t (egy�bk�nt az utols� eredm�nyt eldobja).
 * Hab�r a dinamius t�mb megny�jt�sa hossz� m�velet is lehetne, itt maximum 9 elem� t�mb�t kell m�solni, ami nem probl�ma a mai g�peknek.
 * @param tl Dinamikus t�mb
 * @param new Az eredm�ny, amit berak a t�mbbe
 * @param meretek A Szintek m�reteit �s t�mbj�t tartalmaz� strukt�ra
 */
static void eredmeny_felvesz(Toplista *tl, Eredmeny new, Szintek meretek) {
    if (tl->meret < 10) {
        Eredmeny *temp = (Eredmeny *) realloc(tl->hs, (tl->meret + 1) * sizeof(Eredmeny));
        if (temp == NULL) {
            free(tl->hs);
            kilep(2, "", meretek);
        }
        tl->hs = temp;
        tl->meret++;
    }
    if (tl->meret == 1) {
        (tl->hs)[0] = new;
        return;
    }
    for (int i = tl->meret - 1; i > new.hely; i--) {
        (tl->hs)[i] = (tl->hs)[i - 1];
    }

    (tl->hs)[new.hely] = new;
    for(int i = new.hely + 1; i < tl->meret; i++) {
        if ((tl->hs)[i].pont < (tl->hs)[i - 1].pont) {
            (tl->hs)[i].hely++;
        } else {
            (tl->hs)[i].hely = (tl->hs)[i - 1].hely;
        }
    }
}

/**
 * Kinyomtatja a k�perny�re az aktu�lis toplist�t, majd felszabad�tja a dinamikus t�mb�t.
 * A toplista nyomtat�sa ut�n m�r sosincs sz�ks�g�nk a toplist�ra, ez�rt innen is fel lehet szabad�tani.
 * @param tl A toplista dinamikus t�mbje
 */
void toplista_nyomtat(Toplista tl) {
    printf("\nTOPLISTA\n");
    if (tl.meret == 0) {
        printf("M�g nincsenek eredm�nyek!\n");
    } else {
        for (int i = 0; i < tl.meret; i++) {
            printf("%d. %s: %d pont\n", tl.hs[i].hely + 1, tl.hs[i].nev, tl.hs[i].pont);
        }
    }
    printf("\n");
    free(tl.hs);
}

/**
 * F�jlba �rja az toplist�t.
 * @param tl A toplista dinamikus t�mbje
 */
static void toplista_fajlba(Toplista tl) {
    FILE *fp = fopen("toplista.fs", "w");
    if (fp == NULL) {
        printf("�rv�nytelen toplista f�jl!\n");
    } else {
        fprintf(fp, "%d\n", tl.meret);
        for (int i = 0; i < tl.meret; i++) {
            fprintf(fp, "%d %d %s\n", tl.hs[i].hely, tl.hs[i].pont, tl.hs[i].nev);
        }
    }
    fclose(fp);
}
