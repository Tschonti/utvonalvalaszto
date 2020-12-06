#include <stdlib.h>
#include "utvonalkereso.h"
#include "egyeb.h"
#include "debugmalloc.h"

/*! @file utvonalkereso.c
 * \brief Ez a modul tartalmazza a legrövidebb útvonal megtalálásához szükséges függvényeket.
 *
 * Többek között olyan függvények, amik gráfot építenek, alkalmazzák a Dijkstra-algoritmust,
 * és az eredményt egy kétdimenziós tömbbé alakítják, hogy könnyebben lehessen a képernyõre nyomtatni.
 * A fõbb függvények algoritmusairól az elsõ fejezetben részletesen is írok.

 */


/**
 * Megkeresi a legrövidebb útat az aktív szinten a rajt és a cél között (a fájl többi függvényének segítségével).
 * Minden ilyen segédfüggvényhez elkészíti a szükséges segédtömböket, majd azokat felszabadítja.
 * @param meretek Betöltött szintek méretei és tömbje
 * @param idealis Ebbe a változó menti a függvény az ideális útvonal hosszát
 * Az irányok sorrendje megegyezik enum Irany-nyal.
 * @return Egy olyan kétdimenziós tömb, aminek azon mezõi igazak, amin átmegy az ideális útvonal, a többi hamis.
 */
bool **legrovidebb(Szintek meretek, int *idealis) {
    Csucs ***epitesnel_vizsgalt = csucs_tomb_foglal(meretek);
    Csucs *start = (Csucs *) malloc(sizeof(Csucs));
    csucs_init(start, (Pozicio){0, 0}, meretek);
    epitesnel_vizsgalt[0][0] = start;

    // Csúcsok száma a gráfban - ez lesz majd a tömb hossza is
    int hossz = 1;
    graf_epit(start, epitesnel_vizsgalt, meretek, &hossz);
    free(epitesnel_vizsgalt[0]);
    free(epitesnel_vizsgalt);

    Csucs **lista = (Csucs **) malloc(hossz * sizeof(Csucs *));

    bool **lapitasnal_vizsgalt = bool_tomb_foglal(meretek);
    // Hány csúcsot raktunk eddig be a tömbbe.
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
 * Rekurzív függvény, ami felépíti a gráf adatstruktúrát. Az elágazások lesznek a gráf csúcsai, az õket összekötõ utcák pedig az élek.
 * Egy futása egy adott csúcsot köt össze szomszédjaival, majd mindegyik szomszédjára meghívja magát. Közben szémolja, hogy összesen hány csúcsot talál.
 * @param cs Annak a csúcsnak a pointere, amelyiket össze szeretnénk kötni a szomszédaival.
 * @param vizsgalt Kétdimenziós tömb (méretei megegyeznek a szint méreteivel), aminek minden mezõje NULL, ha ahhoz a mezõhöz még nem csináltunk csúcsot.
 * Amikor egy új csúcsot készítünk, ebben a tömbben a megfelelõ mezõt a csúcs pointerére állítjuk.
 * @param meretek Betöltött szintek méretei és tömbje
 * Az irányok sorrendje megegyezik enum Irany-nyal.
 * @param meret Ebbe a változóba menti a csúcsok számát.
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
                if (utak_szama(uj, meretek) == 1) {  // zsákutca
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
 * Rekurzív függvény, ami csinál egy egydimneziós tömböt a gráfhoz, aminek minden eleme egy csúcsra mutat. Ez késõbb segíteni fogja az eligazodást a gráfban.
 * @param cs Kiinduló csúcs
 * @param lista Az épülõ tömb
 * @param n Index, amit a rekurzív függvények mindegyike elér. Számlálja, hogy eddig hány csúcsot tettünk a tömbbe.
 * @param vizsgalt Kétdimenziós tömb, aminek minden mezõje pontosan akkor igaz, ha az ahhoz a mezõhöz tartozó csúcs már vizsgálva volt.
 * @param hossz Csúcsok száma a gráfban
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
 * Függvény, ami megvalósítja a Dijkstra algoritmust a rajt és cél között.(Azaz meghatározza a legrövidebb útat a két pont között.)
 * @param lista Tömb, aminek minden elem a gráf egyik csúcsára mutat.
 * @param meret Csúcsok száma a gráfba, azaz a tömb mérete.
 */
static void dijkstra(Csucs **lista, int meret) {
    // ha van még meg nem látogatott csúcs...
    while (van_nem_latogatott(lista, meret)) {
        int index = legkozelebbi(lista, meret);     // a jelenleg legkozelebbit vizsgáljuk
        if (lista[index]->cel) {                    // ha ez a cél, végeztünk
            return;
        }
        for (int i = 0; i < 4; i++) {
            if (lista[index]->elek[i].csucs != NULL) {
                if (!lista[index]->elek[i].csucs->vizsgalt) {                       // Felkeressük az összes, még nem vizsgált szomszédját
                    int tav = lista[index]->tavolsag + lista[index]->elek[i].suly;  // Kiszámoljuk, hogy ha a jelenleg vizsgált csúcson keresztül közelítenénk meg, milyen messze lenne
                    if (tav < lista[index]->elek[i].csucs->tavolsag) {              // Ha ez a távolság kevesebb, mint a jelenlegi távolsága...
                        lista[index]->elek[i].csucs->tavolsag = tav;                // akkor frissítjük a távolság és az elõzõ mezõt
                        for (int j = 0; j < 4; j++) {
                            /* Meghatározzuk, melyik irányba kell elindulni ebbõl a csúcsból, hogy visszajussunk a vizsgált csúcsba, azon az élen, amin jöttünk.
                             * A második feltételre azért van szükség, mert ha két csúcs között két közvetlen él lenne, így akkor is azt jelöljük meg elõzõnek, amelyiket most épp vizsgáltuk
                             * (vagyis amelyik éppen a legrövidebb, de lehet nem vizsgáltuk még mindet) */
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
 * Felszabadítja a gráf minden csúcsát, majd a tömböt amiben tároltuk õket.
 * @param lista A csúcsok tömbje
 * @param meret A tömb mérete
 */
static void laposgraf_szabadit(Csucs ***lista, int meret) {
    for (int i = 0; i < meret; i++) {
        free((*lista)[i]);
    }
    free(*lista);
}

/**
 * Függvény, ami megvizsálja hogy az adott koordináta a pályán belül van-e és a mezõ utca-e.
 * @param p A pont x és y koordinátja Pozíció struktúrában
 * @param meretek Betöltött szintek méretei és tömbje
 * @return Igaz, ha a koordináták által mutatott mezõ utca
 */
static bool utca_teszt(Pozicio p, Szintek meretek) {
    return (p.x >= 0 && p.x < meretek.hossz && p.y >= 0 && p.y < meretek.mag && meretek.terkep[meretek.aktiv_szint][p.y][p.x] != epulet);
}

/**
 * Függvény, ami meghatározza, hogy az adott koordináták által meghatározott mezõnek hány szomszédja utca. (Akkor fogunk egy mezõt csúcsnak tekinteni, ha legalább 3
 * utca szomszédja van). Mivel a rajtot és a célt is csúcsnak szeretnénk tekinteni (hiszen közöttük keressük a legrövidebb utat),
 * náluk automatikusan 3-at fog visszaadni a függvény.
 * @param p A pont x és y koordinátja Pozíció struktúrában
 * @param meretek Betöltött szintek méretei és tömbje
 * @return Szomszédos utca mezõk száma
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
 * Ha egy mezõnek két szomszédja utca, akkor ez a függvény meghatározza, hogy melyik irányba kell tovább menni, hogy az utcán maradjunk.
 * @param regi Az elõzõ pozíció x és y koordinátája Pozíció struktúrában (erre nem szeretnénk visszamenni)
 * @param uj Meghíváskor a jelenlegi pozíció koordinátái, ami frissítve lesz a következõ koordinátákra, úgy hogy az utca maradjon, de ne abba az irányba lépjünk, ahonnan jöttünk.
 * @param meretek Betöltött szintek méretei és tömbje
 * Az irányok sorrendje megegyezik enum Irany-nyal.
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
 * Függvény, ami meghatározza, hogy az adott x-y elmozdulással melyik irányba haladunk.
 * A két koordináta közül pontosan egynek 0-nak, egynek pedig 1-nek vagy -1-nek kell lennie.
 * Más x-y párra nem feltétlenül ad helyes eredményt.
 * @param p Az x és y koordináta Pozíció struktúrában
 * @return Az irány enum-ja
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
 * Kezdeti állapotba állít egy csúcsot. Pointereit lenullázza, megvizsgálja hogy a rajt vagy esetleg a cél-e.
 * Ha a csúcs nem a rajt, akkor a távolságot egy olyan nagy értékre állítja, amilyen távolságok a gráfban biztos nem lesznek.
 * (A Dijkstra-algoritmus szerint végtelenre kellene állítani, de ilyen lehetõség nincs C-ben)
 * @param cs A beállítani kíívánt csúcs pointere
 * @param p A csúcs x és y koordinátája Pozíció struktúrában
 * @param meretek Betöltött szintek méretei és tömbje
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
 * Függvény, ami meghatározza, hogy van-e még meg nem látogatott csúcs a gráfban.
 * @param lista A gráf csúcsait tartalmazó tömb.
 * @param meret A tömb mérete, azaz csúcsok száma
 * @return Igaz, ha van még meg nem látogatott csúcs, egyébként hamis
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
 * Függvény, ami meghatározza azt a csúcsot, ami még nem volt vizsgálva, és a távolsága a legkisebb a kezdõ csúcstól.
 * @param lista A gráf csúcsait tartalmazó tömb.
 * @param meret A tömb mérete, azaz csúcsok száma
 * @return A megfelelõ csúcs indexe a tömbben
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
 * Létrehoz egy dinamikusan foglalt kétdimenziós tömböt, aminek mérete megegyezik a pálya méretével,
 * és mezõi pontosan akkor igazak, ha a gráfban az ideális útvonal a rajt és a cél között átmegy az adott mezõn.
 * @param lista A gráf csúcsait tartalmazó tömb.
 * @param meret A tömb mérete, azaz csúcsok száma
 * @param meretek Betöltött szintek méretei és tömbje
 * @param idealis Pointer, ahova a függvény elmenti az ideális útvonal hosszát
 * @return A dinamikusan foglalt kétdimenziós bool tömb, amit a hívónak fel kell szabadítania.
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