#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "econio.h"
#include "szintek.h"
#include "egyeb.h"
#include "toplista.h"
#include "debugmalloc.h"

#ifdef _WIN32
#include <windows.h>
#endif

/*! \mainpage Útvonalválaszt-O Programozói Dokumentáció
 * \section intro Bevezetõ
 * Ezen programozói dokumentáció célja az, hogy a program belsõ felépítését, mûködését bemutassa.
 * Ha a játékszabályok érdekelnek, akkor ajánlom a felhasználói dokumentációt.
 * Itt a fõoldalon el fogom magyarázni a megvalósított módszereket, hogy miért és milyen adatstruktúrákat használtam,
 * a program által olvasott vagy írt fájlok szintaktikai követelményeit,
 * valamint felsorolom azokat a hibaüzeneteket, amiket a program kiírhat, és hogy hogyan javítsd ki õket.
 * Az egyes modulok, függvények, adatszerkezetek, változók és paraméterek részletes leírásait a fájlok aloldalain (vagy pdf-es verzió esetén a következõ fejezetekben) találod.
 *
 * \section mukodes A program belsõ mûködése
 * A program konzolban fut, így esztétikailag nem a legszebb, de funkcionalitásából emiatt nem vesztett semmit.
 * A menü irányítása egyszerûen a felhasználó által begépelt számokkal történik, a szintek futása közben azonban a c-econio
 * modul segítségével kezelem a felhasználói inputot. (A c-econio modul a githubon: https://github.com/czirkoszoltan/c-econio)
 * Ennek a modulnak a segítségével tudom a konzolban a szöveg és a háttér színét változtatni, tetszõleges helyre szöveget írni, valamint
 * felhasználói inputot kapni anélkül, hogy neki mindig enter-t kéne nyomnia. Ez a könyvtár is a forrásfájlok között van, így a program fordításához csak
 * a szabványos könyvtárakra és a forrásfájlokra van szükség.
 *
 * \subsection adatszerkezet A program adatszerkezetei
 * A legnagyobb kihívás a program fejlesztése közben egyértelmûen a legrövidebb útvonalat megtaláló függvények megírása volt. Ehhez szükség volt egy olyan
 * adatstruktúrára, ami egy súlyozott gráfhoz hasonlóan mûködik, de az összes elemén is könnyû végigmenni.
 *
 * Ehhez egy különleges láncolt listát készítettem: A megszokott láncolt listától eltérõen, itt minden elem akár 4 másikhoz is kapcsolódik,
 * sõt minden kapcsolatnak (élnek a gráfban) van súlya is. A gráfot reprezentáló adatszerkezetet egy kicsi, 4x4-es pálya példáján keresztül fogom bemutatni,
 * és hogy könnyebb legyen megérteni, készítettem hozzá egy ábrát is. Az ábra bal alján látható a pálya. A gráf csúcsai minden pályán a rajt, a cél, valamint
 * az elágazások (de a sima kanyarok nem!). Minden csúcs struktúrában van egy 4 elemû lista, aminek minden eleme egy él struktúra. Egy élnek két mezõje van: az él súlya,
 * és annak a csúcsnak a pointere, ahová az él mutat. Mivel a pointereken csak egyik irányban tudunk mozogni, ezért a gráfban valójában ha két csúcs között van kapcsolat,
 * akkor köztük két, egymással ellentétes irányítású él van, melyek súlya megegyezik, és ez a két csúcs távolsága. Az éleket tartalmazó tömb sorrendje megegyezik az Irany enum-mal,
 * tehát pl. az elek[2] a lefele mutató élet jelenti. Ha egy csúcsnak egy adott irányban nincs szomszédja, akkor annak az élnek a súlya 0, pointere pedig NULL-ra mutat.
 * Ebben az adatstruktúrában könnyedén eljuthatunk bármelyik csúcsból a szomszédjaihoz, ami nagyon fontos lesz a Dijkstra-algoritmus során (errõl késõbb lesz szó).
 * Azonban nehézkes a gráf összes csúcsát sorba venni, hiszen mivel a gráfban vannak körök, könnyen végtelen ciklusba keveredhetünk. Ezért készítünk egy olyan tömböt (listát),
 * aminek minden eleme egy csúcsra mutat. Amikor ezt a tömböt megépítjük, figyelünk arra, hogy minden csúcs pontosan egyszer szerepeljen a tömbben,
 * és így ezután könnyen sorra vehetjük az összes csúcsot, amire szintén szükségünk lesz késõbb. Fontos megemlíteni a tömbbel kapcsolatban, hogy a csúcsok sorrendje nem érdekel minket,
 * bár a kezdõ csúcs mindig a nulladik helyen lesz. Fontos megemlíteni még, hogy a gráf vagy a tömb a program futása során soha nem fog átméretezõdni. A láncolt listához hasonló
 * adatszerkezetre azért volt szükség, hogy egy csúcs szomszédjait könnyen elérjük. Ezen a különleges láncolt listán viszont nem tudunk csak egyszerûen végig menni, és minden
 * elemét vizsgálni, ezért van szükség a sima tömbre. A játékot kb. 20 cella magas és 20 cella hosszú szinten érdekes játszani, és egy ilyen szinten nem nagyon lesz 20-30-nál
 * több csúcs. Így igaz, hogy minden csúcsot gyakorlatilag több helyen is tárolunk, ez összeségében nem fog sok helyet foglalni, viszont a további algoritmusok megírását nagyban segíti.
 * A csúcsok száma akkor lehet sok, ha a szinten egymás mellett több oszlop vagy sor is utca, hiszen ekkor tulajdonképpen minden cella egy elágazás. A program ilyenkor is
 * megfelelõen mûködik, azonban ilyen szinteket nincs értelme készíteni, elvégrre csak négy irányban lehet mozogni, így csak sok, egyenlõ hosszúságú útvonal-lehetõséget jönne létre,
 * ami az útvonalválasztási feladatot nem nehezítené.
 * \image html nhf_adatstrukt.png
 * \image latex nhf_adatstrukt.png
 *
 * A gráfon kívül a program többször használ még kétdimenziós tömböt, amelynek minden értéke a pálya egy mezõjére mutat, valamilyen formában. Ezek közül a leggyakrabban használt
 * a Szintek struktúra terkep mezõje, ahova fájlból töltõdnek be a pályák (itt a szintek száma még egy harmadik dimenziót is létrehoz). Ezen kívül szükség van még egy ilyen tömbre
 * a gráf megépítésekor. Itt miután egy csúcsnak elkészítettük a struktúráját és összekötöttük a szomszédjaival, egy ilyen tömbbe berakjuk a rá mutató pointert, arra az x-y helyre,
 * ahol a pályán elhelyezkedik. Így amikor a többi csúcs vizsgálásánál újra visszajutunk ehhez a csúcshoz, már tudjuk, hogy ezt a csúcsot nem kell újra elkészíteni, sõt össze is
 * tudjuk kötni a másik csúccsal. Amikor a gráfból egy sima, egydimenziós tömböt csinálunk, akkor is egy ugyanilyen tömböt használunk, ám annak értekei boolean értékek, hiszen itt
 * a pointerre nincs szükségünk. Az eddig felsorolt tömbök mind dinamikusan foglaltak voltak, ami kézen fekvõ, hiszen méretük állandó.
 *
 * Van azonban a programban egy olyan tömb is, aminek mérete nem állandó, ez pedig az eredményeket tároló tömb, a toplista. Azonban ez is dinamikus tömbként van kialakítva,
 * hiszen mérete csak akkor változik, ha új eredményt kell betenni a listába, és annak hossza nem érte még el a tízet. Így maximum kilenc eredményt kell egy ilyen realloc()-kor
 * másolni, és azt is csak ritkán.
 *
 * \subsection algoritmus A program legfontosabb algoritmusai
 * A program legfontosabb algoritmusai mind a legrövidebb útvonal meghatározáshoz szükségesek.
 * \subsubsection epit A gráf építése
 * Az algoritmust megvalósító függvény: graf_epit()
 *
 * Az elsõ lépés az ideális útvonal meghatározásában az egy gráf építése, amire majd használható lesz a Dijkstra-algoritmus. A feladat tehát az, hogy egy olyan kétdimenziós
 * tömbbõl, amiben minden mezõ utca vagy épület, készítsünk egy olyan gráf adatstruktúrát, amiben azok a mezõk lesznek a csúcsok, ahol legalább 3 utca találkozik, az élek pedig
 * az õket összekötõ utcák. Erre egy rekurzív algoritmus használtam. Habár a rajtban és a célban csak két utca találkozik, ezek is csúcsok lesznek a gráfban, hiszen ezen két
 * pont között keressük a legrövidebb utat. Az algoritmus pszeudokódja egy adott csúcsból (a rajtból indítjuk):
 * 1. Próbáljunk meg elindulni a gráfból egy irányba!
 * 2. Ha arra utca van, akkor kövessük az utcát egészen addig, amíg olyan mezõhöz nem érünk, aminek 3 szomszédja is utca, azaz itt elágazás lesz. Közben számoljuk hányat lépünk!
 * Ha arra épület volt, akkor ebben az irányban a csúcsnak nincs éle.
 * 3. Amikor egy csúcshoz jutottunk, vizsgáljuk meg, hogy jártunk-e már ebben a csúcsban!
 * 4. Ha nem, akkor készítsük el a csúcsot, és határozzuk meg, melyik irányból érkeztünk ide.
 * 5. Állítsuk be a megfelelõ éleket mindkét csúcson, hogy egymásra mutassanak, a súly pedig a lépések száma legyen.
 * 6. Jegyezzük meg, hogy jártunk már ebben a csúcsban is, hogy többször ne vizsgáljuk meg.
 * 7. Alkalmazzuk ugyanezt az algoritmust most erre a csúcsra is!
 * 8. Ha a 3. pontban igen volt a válasz, akkor is készítsük el a megfelelõ éleket, azonban más dolgunk nincs, mert az a csúcs már vizsgálva volt (vagyis vizsgálat alatt van,
 * csak közben a függvény többször meghívta magát, így még nem érte körbe az összes irányon.)
 * Az algoritmus közben ne felejtsük el számolni a csúcsok számát sem, hiszen ez alapján kell a megfelelõ méretû memóriaterületet foglalni ahhoz a listához, amiben a csúcsokra
 * mutató pointereket tároltuk.
 *
 * \subsubsection kilapit A gráf "kilapítása"
 * Az algoritmust megvalósító függvény: graf_kilapit()
 *
 * Miután meg van a gráfunk, ami gyakorlatilag egy kétdimenziós láncolt lista, hiszen négyféle irányban tudunk benne mozogni, szeretnénk egy egydimenziós tömböt is. Ennek nem kell
 * láncolt listának lennie, ennek csak az lesz a szerepe, hogy könnyen és gyorsan végig tudjunk menni az összes csúcson, szomszédságtól függetlenül. Ehhez az elõzõhöz hasonló
 * rekurzív algoritmust használunk. Elindulunk a kezdõ csúcsból mind a négy irányba, és ha olyan csúcsot találunk, amit még nem raktunk bele a listába, akkor belerakjuk.
 *
 * \subsubsection dijkstra A Dijkstra-algoritmus
 * Az algoritmust megvalósító függvény: dijkstra()
 *
 * Most hogy megvan a gráf és a lista is, alkalmazhatjuk a híres Dijkstra-algoritmust,
 * ami egy gráfban megtalálja két csúcs között a legrövidebb utat. Ennek az algoritmusnak a pszeudókódja röviden:
 * 1. A gráf csúcsait rendezzük két halmazba: vizsgált és nem vizsgált. Elõször legyen minden a nem vizsgált halmazban.
 * 2. Minden csúcshoz tartozzon egy mezõ, ami azt adja meg, hogy milyen messze van a kezdõ csúcstól. Ezt elõször állítsuk minden csúcsnál
 * végtelenre (ez esetben 10 000-re állítottam, ezt sem fogja semmilyen szint túllépni), kivéve a kezdõ csúcsnál, ott legyen 0.
 * 3. Minden csúcshoz tartozzon egy elõzõ mezõ is, ami rámutat az elõzõ csúcsra, vagyis hogy ha a kezdõ csúcsból ebbe a csúcsba a legrövidebb úton jövünk,
 * akkor melyik csúcsból érkeznénk ide. Ez a mezõ legyen üres minden csúcsnál kezdésnek.
 * 4. A következõ részt addig ismételjük, amíg a meg nem vizsgált csúcsok halmaza nem üres.
 * 5. Válasszuk ki a meg nem vizsgált csúcsok közül azt, amelyiknek a távolsága a kezdõ csúcstól a legnagyobb.
 * 6. Ha ez a csúcs a cél, akkor végeztünk, kiléphetünk. (Így ugyan nem lesz meg minden pontnak a távolsága a kezdõ ponttól, de erre nincs is szükség ebben a programban.
 * A cél távolsága a kezdõ csúcstól pedig ezután már nem változna az algoritmus futása során.)
 * A keresett távolság a cél csúcs távolság mezõjében van, az elõzõ mezõket követve pedig megkapjuk az útvonalat a kezdõ csúcsig.
 * 7. Keressük fel a csúcs összes, eddig még nem vizsgált szomszédját! Minden szomszédos csúcs esetén számoljuk ki, hogy milyen hosszú úton jutunk el oda, ha az éppen
 * vizsgált csúcson keresztül megyünk. Ezt úgy kapjuk meg, hogy a vizsgált csúcs távolságához hozzáadjuk a szomszédos csúcsba vezetõ él súlyát.
 * Ha ez az érték kisebb, mint a szomszédos csúcs távolság mezõje, akkor frissítsük azt, és az elõzõ mezõt állítsuk be, hogy a vizsgált csúcsra mutasson.
 * Ezt a lépést ismételjük a vizsgált csúcs összes, eddig még nem vizsgált szomszédjára.
 * 8. A vizsgált csúcsot rakjuk át a nem vizsgált halmazból a vizsgált halmazba.
 * 9. Ugorjunk vissza a 4. lépéshez.
 *
 * Habár a pszeudókódból kevésbé látszik, nagy szükség volt a sima, egydimenziós listára is az algoritmus közben, hiszen nagyban leegyszerûsítette azt, hogy megmondjuk, hány
 * csúcsot nem vizsgáltunk még (van_nem_latogatott()) és azt is, hogy megmondjuk, melyik a legközelebbi csúcs (legkozelebbi()).
 *
 * \subsubsection matrix Bool mátrix létrehozása
 * Az algoritmust megvalósító függvény: matrix_letrehoz()
 *
 * Az algoritmus futása után még fontos, hogy az ideális útvonalat olyan formában kell átadni a függvénynek, ami a képernyõre rajzolja a térképet, azaz minden
 * mezõrõl egyértelmûen el lehessen dönteni, arra ment-e az ideális útvonal. Az algoritmus futása után csak egymásra mutató csúcsokat kapunk, amik a térképen
 * elágazásokat jelentenek, még ezeket is össze kell kötni. Tehát egy olyan kétdimenziós tömböt fogunk létrehozni, aminek egy mezõje pontosan akkor igaz, ha
 * arra megy az ideális útvonal. Ehhez a célból indulunk, és követjük a csúcs struktúrájában, a Dijsktra-algoritmus által meghatározott elõzõ mezõt, azaz hogy melyik
 * csúcs felé kell indulnunk. Ahhoz, hogy minden egyes mezõt a két csúcs között megjelöljünk, újra felhasználjuk a gráf építésénél már használt függvényeket. (utak_szama(),
 * kovi_utca())
 *
 * És máris készen vagyunk az ideális útvonal meghatározásával, amit már egy egyszerû nyomtató függvénnyel meg tudunk mutatni a játékosnak.
 *
 * \subsection test A program tesztelése
 * A program tesztelését fõleg kézzel végeztem, azaz minél többször, minél többféle szinttel próbáltam. A memóriakezeléshez viszont használtam a Valgrind nevû programot.
 * Mivel ez a program Windowson nem mûködik, WSL-ben, a Clion-on keresztül tudtam futtatni, és a program jelenlegi verziójára nem jelez semmilyen hibát.
 *
 * \section fajlok A program által olvasott vagy írt fájlok
 * A program négy fájlt kezel, amiknek a szerkesztése nem szükséges a program futásához, de mivel komoly tartalmi megkötések vannak, ha egyszer belenyúlsz, könnyen hibákba ütközhetsz.
 * Ebben a fejezetben leírom, hogy írj olyan fájlokat, amiket a program elfogad, a következõben pedig abban segítek, hogy megpróbálom megmondani, hol
 * rontottad el, a hibaüzenet alapján.
 *
 * A program három fõ fájlja ".fs" kiterjesztést alkalmaz, amit csak azért használtam, hogy a kevésbé hozzáértõk ne merjenek, vagy tudjanak belenyúlni.
 *
 * \subsection szintek "levels.fs"
 * Ez a fájl tartalmazza a szintek szöveges reprezentációját, és ez talán az egyetlen, amibe egy felhasználónak érdemes lehet belenyúlnia, ha már megunta az alap szinteket.
 * Természetesen ha más szinteket töltesz be, akkor a toplistának nem lesz sok értelme, de a program ezt nem fogja tudni.
 *
 * A fájl elsõ sorában három egész számnak kell állnia, szóközzel elválasztva.
 * 1. A fájlban szereplõ szintek száma. Ha kevesebb szint van a fájlban, hibát fog dobni a program, ha több, akkor a maradékot figyelmen kívül hagyja.
 * 2. A szintek magassága (azaz hány karakter magasak) Fontos, hogy ez az összes szintre vonatkozik! Ha egy szint is nem ilyen magas, hibát fog dobni a program.
 * 3. A szintek hosszúsága (ugyanez igaz erre is, mint a magasságra)
 * Ebbõl tehát egyértelmû, hogy a játék által egyszerre betöltött szinteknek a mérete meg kell hogy egyezzen.
 *
 * Ezután jönnek a szintek sorai. Minden szinthez annyi sor és a sorokban annyi karakter tartozik, amennyit az elsõ sorban meghatározott a fájl. Minden karakter vagy
 * "U" (utca) vagy "E" (épület) kell hogy legyen, majd egy sortörés. A szinteket egy üres sor válassza el egymástól. Ha a fájl bármilyen más karaktert tartalmaz, hibát fog
 * dobni a program.
 *
 * \subsection toplista "toplista.fs"
 * Ez a fájl tartalmazza az aktuális toplistát. Belenyúlni és átírni az eredményedet csalás! Ha valamit átírtál benne, inkább töröld a fájlt, és az elsõ alkalomkor
 * a program el fogja készíteni az üres toplistát. De azért leírom, hogy kell használni...
 *
 * A fájl elsõ sorában egyetlen számnak kell lennie, ami azt jelenti, hogy hány eredmény van a fájlban. Ez a szám nem lehet nagyobb tíznél, hiszen a program a legjobb 10
 * eredményt tárolja. Ezután minden egyes sor egy eredményt jelent. Egy sor két szám kezd:
 * 1. Az adott eredmény helyezése, 0-tól indexelve (a holtversenyek miatt fontos)
 * 2. Az adott eredmény pontja
 * Ezeket szóközzel kell elválasztani, majd újabb szóköz után jön a játékos neve, egészen a sor végéig (szóközt is tartalmazhat)
 *
 * \subsection savegame "savegame.fs"
 * Na ebbe a fájlba aztán végképp csalás belenyúlni, ugyanis itt tárolódik egy lementett játéknak az adatai. A fájl mindösszesen két számot tartalmazhat, szóközzel elválasztva:
 * 1. A teljesített szintek száma (azaz a következõ szint indexe)
 * 2. Az eddig elért pontok összege
 *
 * \subsection errors "errorlog.txt"
 * Ha a program futása során valamilyen hiba lép fel, akkor a program a képernyõre írás mellett ide is menti a hibakódot és -üzenetet. Az elõzõ hibaüzenet akkor törlõdik,
 * amikor következõre szabályosan lépünk ki a programból. Ennek a fájlnak a szerkesztése teljesen felesleges, a program nem olvas belõle. A következõ fejezet fog foglalkozni
 * az egyes hibaüzenetek pontos jelentésével.
 *
 * \section hibak Hibakódok és hibaüzenetek
 *  Ha a program futása során valamilyen hiba lép fel, akkor a program a képernyõre írás mellett ide az errorlogs.txt fájlba menti a hibakódot és -üzenetet. Itt le fog írni,
 *  mi okozhatja az egyes hibákat, és hogy javítsuk õket. Ha bármi egyéb hiba lépne fel, vagy nem sikerül megjavítani, keress engem bátran: feketesamu(kuakc)gmail(pont)com
 *
 *  \subsubsection top1 10, Érvénytelen toplista fájl!
 *  A program nem tudta megnyitni a "toplista.fs" fájlt se olvasó, se író módban. Lehetséges, hogy nincs jogosultsága ehhez ennek a programnak?
 *
 *  \subsubsection top2 11, Érvénytelen toplista fájl!
 *  A toplista.fs fájl elsõ sora érvénytelen, mindössze egy számot kellene tartalmaznia! Lásd az elõzõ fejezetet a pontos leírásért!
 *
 *  \subsubsection top3 12, Érvénytelen toplista fájl!
 *  A toplista.fs fájl valamelyik sorában nem sikerült két szám beolvasása, valószínûleg valamelyik hiányzik. Lásd az elõzõ fejezetet a pontos leírásért!
 *
 *  \subsubsection save 13, A játékmentés fájlja érvénytelen!
 *  A program nem tudta megnyitni a "savegame.fs" fájlt író módban. Lehetséges, hogy nincs jogosultsága ehhez ennek a programnak?
 *
 *  \subsubsection szint1 14, A szinteket tartalmazó fájl érvénytelen vagy nem létezik!
 *  A program nem tudta olvasó módban megnyitni a "levels.fs" fájlt. Lehetséges, hogy nem létezi? A nhf.exe fájlal egy mappában kell lennie!
 *
 *  \subsubsection szint2 15, A szinteket tartalmazó fájl érvénytelen!
 *  A "levels.fs" fájl elsõ sora érvénytelen, nem sikerült a 3 szám beolvasása. Lásd az elõzõ fejezetet a pontos leírásért!
 *
 *  \subsubsection szint3 16, A szinteket tartalmazó fájl érvénytelen!
 *  A "levels.fs" fájl valamelyik sorában érvénytelen karakter van. Lásd az elõzõ fejezetet a pontos leírásért!
 *
 *  \subsubsection mem 2, Nincs elég memória!
 *  A program futása során memóriát kért az operációs rendszertõl, de az nem volt képes teljesíteni a kívánságát. Mivel a mai számítógépek memóriái elég nagyok
 *  az én programomnak szükséges memóriaterületéhez képest, csak úgy tudom elképzelni ezt a hibaüzenetet, hogy valamelyik fájl elsõ sorába egy irdatlan nagy számot írtál.
 *
 * \section linux Futtatás Linuxon
 * A programot Windows-on fejlesztettem, de Windows Subsystem for Linux-on tesztelve lett. Mivel a két operációs rendszer máshogy kezeli a fájlokban a sorköz jelet,
 * az alap kód windowson fog jól mûködni. A Linuxon futtatáshoz (legalábbis nekem csak így mûködött) ki kell venni a kommentet jelzõ két "/" jelet a következõre helyekrõl:
 * szintek.c, 31., 48. és 51. sor. Ezután újra kell fordítani a programkódot, és elvileg mûködni fog a szintek beolvasása. A toplista fájlnál is hasonló hiba léphet fel.

 */

/*! @file main.c
 * \brief A program fõ fájlja, ami a fõmeüt kezeli és meghívja a megfelelõ modul megfelelõ függvényét.
 *
 */

/**
 * Beállítja a konzol kódolását, kezeli a fõmenüt és meghívja a megfelelõ függvényeket.
 * @return Kilépési kód, de valójában soha nem ezzel lép ki a program, hanem a kilep() függvénynek megadott kóddal.
 */
int main() {
    #ifdef _WIN32
        SetConsoleCP(1250);
        SetConsoleOutputCP(1250);
    #endif
    econio_set_title("Útvonalválaszt-O");
    while (true) {
        int v = menu();
        switch (v) {
            case 1:
            case 2: {
                int szint = 0;
                int pont = 0;
                if (v == 2) {
                    if (jatek_betolt(&szint, &pont) == 1) {
                        break;
                    }
                }
                jatek_indul(szint, pont);
                break;
            }
            case 3:
                econio_clrscr();
                segitseg();
                break;
            case 4: {
                Toplista tl;
                toplista_betolt(&tl, (Szintek) {NULL});
                econio_clrscr();
                toplista_nyomtat(tl);
                break;
            }
            case 5:
                kilep(0, "Köszi a játékot!", (Szintek) {NULL});
            default:;
        }
    }
}