#include <stdlib.h>
#include "utvonalkereso.h"
#include "egyeb.h"
#include "debugmalloc.h"

/*! @file utvonalkereso.c
 * \brief Ez a modul tartalmazza a legr�videbb �tvonal megtal�l�s�hoz sz�ks�ges f�ggv�nyeket.
 *
 * T�bbek k�z�tt olyan f�ggv�nyek, amik gr�fot �p�tenek, alkalmazz�k a Dijkstra-algoritmust,
 * �s az eredm�nyt egy k�tdimenzi�s t�mbb� alak�tj�k, hogy k�nnyebben lehessen a k�perny�re nyomtatni.
 * A f�bb f�ggv�nyek algoritmusair�l az els� fejezetben r�szletesen is �rok.

 */


/**
 * Megkeresi a legr�videbb �tat az akt�v szinten a rajt �s a c�l k�z�tt (a f�jl t�bbi f�ggv�ny�nek seg�ts�g�vel).
 * Minden ilyen seg�df�ggv�nyhez elk�sz�ti a sz�ks�ges seg�dt�mb�ket, majd azokat felszabad�tja.
 * @param meretek Bet�lt�tt szintek m�retei �s t�mbje
 * @param idealis Ebbe a v�ltoz� menti a f�ggv�ny az ide�lis �tvonal hossz�t
 * Az ir�nyok sorrendje megegyezik enum Irany-nyal.
 * @return Egy olyan k�tdimenzi�s t�mb, aminek azon mez�i igazak, amin �tmegy az ide�lis �tvonal, a t�bbi hamis.
 */
bool **legrovidebb(Szintek meretek, int *idealis) {
    Csucs ***epitesnel_vizsgalt = csucs_tomb_foglal(meretek);
    Csucs *start = (Csucs *) malloc(sizeof(Csucs));
    csucs_init(start, (Pozicio){0, 0}, meretek);
    epitesnel_vizsgalt[0][0] = start;

    // Cs�csok sz�ma a gr�fban - ez lesz majd a t�mb hossza is
    int hossz = 1;
    graf_epit(start, epitesnel_vizsgalt, meretek, &hossz);
    free(epitesnel_vizsgalt[0]);
    free(epitesnel_vizsgalt);

    Csucs **lista = (Csucs **) malloc(hossz * sizeof(Csucs *));

    bool **lapitasnal_vizsgalt = bool_tomb_foglal(meretek);
    // H�ny cs�csot raktunk eddig be a t�mbbe.
    int n = 0;
    graf_kilapit(start, &lista, &n, lapitasnal_vizsgalt, hossz);
    free(lapitasnal_vizsgalt[0]);
    free(lapitasnal_vizsgalt);

    dijkstra(lista, hossz);

    bool **matrix = matrix_letrehoz(lista, hossz, meretek, idealis);

    laposgraf_szabadit(&lista, hossz);

    return matrix;
}

/**
 * Rekurz�v f�ggv�ny, ami fel�p�ti a gr�f adatstrukt�r�t. Az el�gaz�sok lesznek a gr�f cs�csai, az �ket �sszek�t� utc�k pedig az �lek.
 * Egy fut�sa egy adott cs�csot k�t �ssze szomsz�djaival, majd mindegyik szomsz�dj�ra megh�vja mag�t. K�zben sz�molja, hogy �sszesen h�ny cs�csot tal�l.
 * @param cs Annak a cs�csnak a pointere, amelyiket �ssze szeretn�nk k�tni a szomsz�daival.
 * @param vizsgalt K�tdimenzi�s t�mb (m�retei megegyeznek a szint m�reteivel), aminek minden mez�je NULL, ha ahhoz a mez�h�z m�g nem csin�ltunk cs�csot.
 * Amikor egy �j cs�csot k�sz�t�nk, ebben a t�mbben a megfelel� mez�t a cs�cs pointer�re �ll�tjuk.
 * @param meretek Bet�lt�tt szintek m�retei �s t�mbje
 * Az ir�nyok sorrendje megegyezik enum Irany-nyal.
 * @param meret Ebbe a v�ltoz�ba menti a cs�csok sz�m�t.
 */
static void graf_epit(Csucs* cs, Csucs ***vizsgalt, Szintek meretek, int *meret) {
    int elhossz;
    Pozicio uj, regi;

    for (int i = 0; i < 4; i++) {
        uj = (Pozicio){cs->x + meretek.iranykonstansok[i][0], cs->y + meretek.iranykonstansok[i][1]};
        if (cs->elek[i].csucs == NULL) {
            if (utca_teszt(uj, meretek)) {
                regi = (Pozicio){cs->x, cs->y};
                elhossz = 1;

                while (utak_szama(uj, meretek) == 2) {
                    elhossz++;
                    kovi_utca(&regi, &uj, meretek);
                }
                if (utak_szama(uj, meretek) == 1) {  // zs�kutca
                    continue;
                }
                Irany ujirany = irany_hataroz((Pozicio) {uj.x - regi.x, uj.y - regi.y});

                cs->elek[i].suly = elhossz;

                if (vizsgalt[uj.y][uj.x] == NULL) {
                    Csucs *ujcsucs = (Csucs *) malloc(sizeof(Csucs));
                    csucs_init(ujcsucs, uj, meretek);

                    ujcsucs->elek[ujirany].csucs = cs;
                    ujcsucs->elek[ujirany].suly = elhossz;

                    cs->elek[i].csucs = ujcsucs;

                    vizsgalt[uj.y][uj.x] = ujcsucs;

                    (*meret)++;
                    graf_epit(ujcsucs, vizsgalt, meretek, meret);
                } else {
                    Csucs *talalt = vizsgalt[uj.y][uj.x];

                    talalt->elek[ujirany].csucs = cs;
                    talalt->elek[ujirany].suly = elhossz;
                    cs->elek[i].csucs = talalt;
                }

            }
        }
    }
}

/**
 * Rekurz�v f�ggv�ny, ami csin�l egy egydimnezi�s t�mb�t a gr�fhoz, aminek minden eleme egy cs�csra mutat. Ez k�s�bb seg�teni fogja az eligazod�st a gr�fban.
 * @param cs Kiindul� cs�cs
 * @param lista Az �p�l� t�mb
 * @param n Index, amit a rekurz�v f�ggv�nyek mindegyike el�r. Sz�ml�lja, hogy eddig h�ny cs�csot tett�nk a t�mbbe.
 * @param vizsgalt K�tdimenzi�s t�mb, aminek minden mez�je pontosan akkor igaz, ha az ahhoz a mez�h�z tartoz� cs�cs m�r vizsg�lva volt.
 * @param hossz Cs�csok sz�ma a gr�fban
 */
static void graf_kilapit(Csucs *cs, Csucs ***lista, int *n, bool **vizsgalt, int hossz) {
    if(!vizsgalt[cs->y][cs->x]) {
        vizsgalt[cs->y][cs->x] = true;
        for (int i = 0; i < 4; i++) {
            if (cs->elek[i].csucs != NULL) {
                graf_kilapit(cs->elek[i].csucs, lista, n, vizsgalt, hossz);
            }
        }
        (*lista)[hossz - *n - 1] = cs;
        (*n)++;
    }
}

/**
 * F�ggv�ny, ami megval�s�tja a Dijkstra algoritmust a rajt �s c�l k�z�tt.(Azaz meghat�rozza a legr�videbb �tat a k�t pont k�z�tt.)
 * @param lista T�mb, aminek minden elem a gr�f egyik cs�cs�ra mutat.
 * @param meret Cs�csok sz�ma a gr�fba, azaz a t�mb m�rete.
 */
static void dijkstra(Csucs **lista, int meret) {
    // ha van m�g meg nem l�togatott cs�cs...
    while (van_nem_latogatott(lista, meret)) {
        int index = legkozelebbi(lista, meret);     // a jelenleg legkozelebbit vizsg�ljuk
        if (lista[index]->cel) {                    // ha ez a c�l, v�gezt�nk
            return;
        }
        for (int i = 0; i < 4; i++) {
            if (lista[index]->elek[i].csucs != NULL) {
                if (!lista[index]->elek[i].csucs->vizsgalt) {                       // Felkeress�k az �sszes, m�g nem vizsg�lt szomsz�dj�t
                    int tav = lista[index]->tavolsag + lista[index]->elek[i].suly;  // Kisz�moljuk, hogy ha a jelenleg vizsg�lt cs�cson kereszt�l k�zel�ten�nk meg, milyen messze lenne
                    if (tav < lista[index]->elek[i].csucs->tavolsag) {              // Ha ez a t�vols�g kevesebb, mint a jelenlegi t�vols�ga...
                        lista[index]->elek[i].csucs->tavolsag = tav;                // akkor friss�tj�k a t�vols�g �s az el�z� mez�t
                        for (int j = 0; j < 4; j++) {
                            /* Meghat�rozzuk, melyik ir�nyba kell elindulni ebb�l a cs�csb�l, hogy visszajussunk a vizsg�lt cs�csba, azon az �len, amin j�tt�nk.
                             * A m�sodik felt�telre az�rt van sz�ks�g, mert ha k�t cs�cs k�z�tt k�t k�zvetlen �l lenne, �gy akkor is azt jel�lj�k meg el�z�nek, amelyiket most �pp vizsg�ltuk
                             * (vagyis amelyik �ppen a legr�videbb, de lehet nem vizsg�ltuk m�g mindet) */
                            if (lista[index]->elek[i].csucs->elek[j].csucs == lista[index] && lista[index]->elek[i].csucs->elek[j].suly == tav - lista[index]->tavolsag) {
                                lista[index]->elek[i].csucs->elozo = j;
                                break;
                            }
                        }
                    }
                }
            }
        }
        lista[index]->vizsgalt = true;
    }
}
/**
 * Felszabad�tja a gr�f minden cs�cs�t, majd a t�mb�t amiben t�roltuk �ket.
 * @param lista A cs�csok t�mbje
 * @param meret A t�mb m�rete
 */
static void laposgraf_szabadit(Csucs ***lista, int meret) {
    for (int i = 0; i < meret; i++) {
        free((*lista)[i]);
    }
    free(*lista);
}

/**
 * F�ggv�ny, ami megvizs�lja hogy az adott koordin�ta a p�ly�n bel�l van-e �s a mez� utca-e.
 * @param p A pont x �s y koordin�tja Poz�ci� strukt�r�ban
 * @param meretek Bet�lt�tt szintek m�retei �s t�mbje
 * @return Igaz, ha a koordin�t�k �ltal mutatott mez� utca
 */
static bool utca_teszt(Pozicio p, Szintek meretek) {
    return (p.x >= 0 && p.x < meretek.hossz && p.y >= 0 && p.y < meretek.mag && meretek.terkep[meretek.aktiv_szint][p.y][p.x] != epulet);
}

/**
 * F�ggv�ny, ami meghat�rozza, hogy az adott koordin�t�k �ltal meghat�rozott mez�nek h�ny szomsz�dja utca. (Akkor fogunk egy mez�t cs�csnak tekinteni, ha legal�bb 3
 * utca szomsz�dja van). Mivel a rajtot �s a c�lt is cs�csnak szeretn�nk tekinteni (hiszen k�z�tt�k keress�k a legr�videbb utat),
 * n�luk automatikusan 3-at fog visszaadni a f�ggv�ny.
 * @param p A pont x �s y koordin�tja Poz�ci� strukt�r�ban
 * @param meretek Bet�lt�tt szintek m�retei �s t�mbje
 * @return Szomsz�dos utca mez�k sz�ma
 */
static int utak_szama(Pozicio p, Szintek meretek) {
    if ((p.x == meretek.hossz - 1 && p.y == meretek.mag - 1) || (p.x == 0 && p.y == 0)) {
        return 3;
    }
    int utak = 0;
    if (p.y < meretek.mag - 1 && meretek.terkep[meretek.aktiv_szint][p.y + 1][p.x] != epulet) {
        utak++;
    } if (p.x < meretek.hossz - 1 && meretek.terkep[meretek.aktiv_szint][p.y][p.x + 1] != epulet) {
        utak++;
    } if (p.y > 0 && meretek.terkep[meretek.aktiv_szint][p.y - 1][p.x] != epulet) {
        utak++;
    } if (p.x > 0 && meretek.terkep[meretek.aktiv_szint][p.y][p.x - 1] != epulet) {
        utak++;
    }
    return utak;
}

/**
 * Ha egy mez�nek k�t szomsz�dja utca, akkor ez a f�ggv�ny meghat�rozza, hogy melyik ir�nyba kell tov�bb menni, hogy az utc�n maradjunk.
 * @param regi Az el�z� poz�ci� x �s y koordin�t�ja Poz�ci� strukt�r�ban (erre nem szeretn�nk visszamenni)
 * @param uj Megh�v�skor a jelenlegi poz�ci� koordin�t�i, ami friss�tve lesz a k�vetkez� koordin�t�kra, �gy hogy az utca maradjon, de ne abba az ir�nyba l�pj�nk, ahonnan j�tt�nk.
 * @param meretek Bet�lt�tt szintek m�retei �s t�mbje
 * Az ir�nyok sorrendje megegyezik enum Irany-nyal.
 */
static void kovi_utca(Pozicio *regi, Pozicio *uj, Szintek meretek) {
    for (int i = 0; i < 4; i++) {
        uj->x += meretek.iranykonstansok[i][0];
        uj->y += meretek.iranykonstansok[i][1];
        if (uj->x < meretek.hossz && uj->y < meretek.mag && uj->x >= 0 && uj->y >= 0) {
            if (meretek.terkep[meretek.aktiv_szint][uj->y][uj->x] != epulet && (uj->x != regi->x || uj->y != regi->y)) {
                regi->y = uj->y - meretek.iranykonstansok[i][1];
                regi->x = uj->x - meretek.iranykonstansok[i][0];
                return;
            }
            else {
                uj->x -= meretek.iranykonstansok[i][0];
                uj->y -= meretek.iranykonstansok[i][1];
            }
        }
        else {
            uj->x -= meretek.iranykonstansok[i][0];
            uj->y -= meretek.iranykonstansok[i][1];
        }
    }
}

/**
 * F�ggv�ny, ami meghat�rozza, hogy az adott x-y elmozdul�ssal melyik ir�nyba haladunk.
 * A k�t koordin�ta k�z�l pontosan egynek 0-nak, egynek pedig 1-nek vagy -1-nek kell lennie.
 * M�s x-y p�rra nem felt�tlen�l ad helyes eredm�nyt.
 * @param p Az x �s y koordin�ta Poz�ci� strukt�r�ban
 * @return Az ir�ny enum-ja
 */
static Irany irany_hataroz(Pozicio p) {
    if (p.x == 1) {
        return bal;
    } if (p.x == -1) {
        return jobb;
    } if (p.y == 1) {
        return fel;
    }
    return le;
}

/**
 * Kezdeti �llapotba �ll�t egy cs�csot. Pointereit lenull�zza, megvizsg�lja hogy a rajt vagy esetleg a c�l-e.
 * Ha a cs�cs nem a rajt, akkor a t�vols�got egy olyan nagy �rt�kre �ll�tja, amilyen t�vols�gok a gr�fban biztos nem lesznek.
 * (A Dijkstra-algoritmus szerint v�gtelenre kellene �ll�tani, de ilyen lehet�s�g nincs C-ben)
 * @param cs A be�ll�tani k��v�nt cs�cs pointere
 * @param p A cs�cs x �s y koordin�t�ja Poz�ci� strukt�r�ban
 * @param meretek Bet�lt�tt szintek m�retei �s t�mbje
 */
static void csucs_init(Csucs *cs, Pozicio p, Szintek meretek) {
    cs->x = p.x;
    cs->y = p.y;
    cs->elozo = 5;
    cs->vizsgalt = false;
    for (int i = 0; i < 4; i++) {
        cs->elek[i] = (El) {0, NULL};
    }
    cs->cel = (p.y == meretek.mag - 1 && p.x == meretek.hossz - 1);
    cs->rajt = (p.y == 0 && p.x == 0);
    cs->tavolsag = cs->rajt ? 0 : 10000;
}

/**
 * F�ggv�ny, ami meghat�rozza, hogy van-e m�g meg nem l�togatott cs�cs a gr�fban.
 * @param lista A gr�f cs�csait tartalmaz� t�mb.
 * @param meret A t�mb m�rete, azaz cs�csok sz�ma
 * @return Igaz, ha van m�g meg nem l�togatott cs�cs, egy�bk�nt hamis
 */
static bool van_nem_latogatott(Csucs **lista, int meret) {
    for (int i = 0; i < meret; i++) {
        if (!lista[i]->vizsgalt) {
            return true;
        }
    }
    return false;
}

/**
 * F�ggv�ny, ami meghat�rozza azt a cs�csot, ami m�g nem volt vizsg�lva, �s a t�vols�ga a legkisebb a kezd� cs�cst�l.
 * @param lista A gr�f cs�csait tartalmaz� t�mb.
 * @param meret A t�mb m�rete, azaz cs�csok sz�ma
 * @return A megfelel� cs�cs indexe a t�mbben
 */
static int legkozelebbi(Csucs **lista, int meret) {
    int ind = 0;
    int min = 100000;
    for (int i = 0; i < meret; i++) {
        if (lista[i]->tavolsag < min && !lista[i]->vizsgalt) {
            min = lista[i]->tavolsag;
            ind = i;
        }
    }
    return ind;
}

/**
 * L�trehoz egy dinamikusan foglalt k�tdimenzi�s t�mb�t, aminek m�rete megegyezik a p�lya m�ret�vel,
 * �s mez�i pontosan akkor igazak, ha a gr�fban az ide�lis �tvonal a rajt �s a c�l k�z�tt �tmegy az adott mez�n.
 * @param lista A gr�f cs�csait tartalmaz� t�mb.
 * @param meret A t�mb m�rete, azaz cs�csok sz�ma
 * @param meretek Bet�lt�tt szintek m�retei �s t�mbje
 * @param idealis Pointer, ahova a f�ggv�ny elmenti az ide�lis �tvonal hossz�t
 * @return A dinamikusan foglalt k�tdimenzi�s bool t�mb, amit a h�v�nak fel kell szabad�tania.
 */
static bool **matrix_letrehoz(Csucs **lista, int meret, Szintek meretek, int *idealis) {
    bool **matrix = bool_tomb_foglal(meretek);
    int index = 0;
    for (int i = 0; i < meret; i++) {
        if (lista[i]->cel) {
            *idealis = lista[i]->tavolsag;
            index = i;
            break;
        }
    }
    Pozicio regi = {0, 0};
    Pozicio uj = {lista[index]->x, lista[index]->y};
    while (true) {
        while (utak_szama(uj, meretek) == 2) {
            matrix[uj.y][uj.x] = true;
            kovi_utca(&regi, &uj, meretek);
        }
        matrix[uj.y][uj.x] = true;
        for (int i = 0; i < meret; i++) {
            if (lista[i]->x == uj.x && lista[i]->y == uj.y) {
                index = i;
            }
        }
        if (lista[index]->rajt) {
            break;
        }

        regi.x = uj.x;
        regi.y = uj.y;
        uj.x += meretek.iranykonstansok[lista[index]->elozo][0];
        uj.y += meretek.iranykonstansok[lista[index]->elozo][1];
    }
    return matrix;
}