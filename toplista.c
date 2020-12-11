#include "toplista.h"
#include "egyeb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*! @file toplista.c
 * \brief A toplista betöltését és szerkesztését leíró függvényeket tartlmazó modul.
 *
 */


/**
 * Fájlból betölti a lementett toplistát.
 * Ha nem találja, akkor létrehozza.
 * Ha nem tudja megynitni a fájlt, vagy az érvénytelen, kilép a kilep() függvénnyel.
 * @param tl Dinamikus tömb, amibe betölti az eredményeket.
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
 */
void toplista_betolt(Toplista *tl, Szintek meretek) {
    FILE *fp = fopen("toplista.fs", "r");

    if (fp == NULL) {
        FILE *fp2 = fopen("toplista.fs", "w");
        if (fp2 == NULL) {
            kilep(10, "Érvénytelen toplista fájl!", meretek);
        }
        fprintf(fp2, "%d\n", 0);
        fclose(fp2);
        fp = fopen("toplista.fs", "r");
    }

    if(fscanf(fp, "%d", &(tl->meret)) != 1) {
        fclose(fp);
        kilep(11, "Érvénytelen toplista fájl!", meretek);
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
                kilep(12, "Érvénytelen toplista fájl!", meretek);
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
 * Megállapítja egy új eredményrõl, hogy az felkerül-e a toplistára. Ha igen, bekéri a felhasználótól a nevét,
 * és meghívja az eredmeny_felvesz() függvényt, majd az új toplistát fájlba írja a toplista_fajlba() függvénnyel.
 * @param tl Dinamikus tömb
 * @param pont Az új eredmény pontszáma
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
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
        printf("Gratulálok, felkerültél a toplistára! Kérlek, add meg a neved (max 50 karkter)!\n");
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
        printf("Szép munka, de a toplistára sajnos nem kerültél fel.\n");
    }
}

/**
 * Egy eredményt berak a dinamikus tömbbe. Ha szükséges, meg is nyújtja a tömböt (egyébként az utolsó eredményt eldobja).
 * Habár a dinamius tömb megnyújtása hosszú mûvelet is lehetne, itt maximum 9 elemû tömböt kell másolni, ami nem probléma a mai gépeknek.
 * @param tl Dinamikus tömb
 * @param new Az eredmény, amit berak a tömbbe
 * @param meretek A Szintek méreteit és tömbjét tartalmazó struktúra
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
 * Kinyomtatja a képernyõre az aktuális toplistát, majd felszabadítja a dinamikus tömböt.
 * A toplista nyomtatása után már sosincs szükségünk a toplistára, ezért innen is fel lehet szabadítani.
 * @param tl A toplista dinamikus tömbje
 */
void toplista_nyomtat(Toplista tl) {
    printf("\nTOPLISTA\n");
    if (tl.meret == 0) {
        printf("Még nincsenek eredmények!\n");
    } else {
        for (int i = 0; i < tl.meret; i++) {
            printf("%d. %s: %d pont\n", tl.hs[i].hely + 1, tl.hs[i].nev, tl.hs[i].pont);
        }
    }
    printf("\n");
    free(tl.hs);
}

/**
 * Fájlba írja az toplistát.
 * @param tl A toplista dinamikus tömbje
 */
static void toplista_fajlba(Toplista tl) {
    FILE *fp = fopen("toplista.fs", "w");
    if (fp == NULL) {
        printf("Érvénytelen toplista fájl!\n");
    } else {
        fprintf(fp, "%d\n", tl.meret);
        for (int i = 0; i < tl.meret; i++) {
            fprintf(fp, "%d %d %s\n", tl.hs[i].hely, tl.hs[i].pont, tl.hs[i].nev);
        }
    }
    fclose(fp);
}
