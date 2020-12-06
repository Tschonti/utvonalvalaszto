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

/*! \mainpage �tvonalv�laszt-O Programoz�i Dokument�ci�
 * \section intro Bevezet�
 * Ezen programoz�i dokument�ci� c�lja az, hogy a program bels� fel�p�t�s�t, m�k�d�s�t bemutassa.
 * Ha a j�t�kszab�lyok �rdekelnek, akkor aj�nlom a felhaszn�l�i dokument�ci�t.
 * Itt a f�oldalon el fogom magyar�zni a megval�s�tott m�dszereket, hogy mi�rt �s milyen adatstrukt�r�kat haszn�ltam,
 * a program �ltal olvasott vagy �rt f�jlok szintaktikai k�vetelm�nyeit,
 * valamint felsorolom azokat a hiba�zeneteket, amiket a program ki�rhat, �s hogy hogyan jav�tsd ki �ket.
 * Az egyes modulok, f�ggv�nyek, adatszerkezetek, v�ltoz�k �s param�terek r�szletes le�r�sait a f�jlok aloldalain (vagy pdf-es verzi� eset�n a k�vetkez� fejezetekben) tal�lod.
 *
 * \section mukodes A program bels� m�k�d�se
 * A program konzolban fut, �gy eszt�tikailag nem a legszebb, de funkcionalit�s�b�l emiatt nem vesztett semmit.
 * A men� ir�ny�t�sa egyszer�en a felhaszn�l� �ltal beg�pelt sz�mokkal t�rt�nik, a szintek fut�sa k�zben azonban a c-econio
 * modul seg�ts�g�vel kezelem a felhaszn�l�i inputot. (A c-econio modul a githubon: https://github.com/czirkoszoltan/c-econio)
 * Ennek a modulnak a seg�ts�g�vel tudom a konzolban a sz�veg �s a h�tt�r sz�n�t v�ltoztatni, tetsz�leges helyre sz�veget �rni, valamint
 * felhaszn�l�i inputot kapni an�lk�l, hogy neki mindig enter-t k�ne nyomnia. Ez a k�nyvt�r is a forr�sf�jlok k�z�tt van, �gy a program ford�t�s�hoz csak
 * a szabv�nyos k�nyvt�rakra �s a forr�sf�jlokra van sz�ks�g.
 *
 * \subsection adatszerkezet A program adatszerkezetei
 * A legnagyobb kih�v�s a program fejleszt�se k�zben egy�rtelm�en a legr�videbb �tvonalat megtal�l� f�ggv�nyek meg�r�sa volt. Ehhez sz�ks�g volt egy olyan
 * adatstrukt�r�ra, ami egy s�lyozott gr�fhoz hasonl�an m�k�dik, de az �sszes elem�n is k�nny� v�gigmenni.
 *
 * Ehhez egy k�l�nleges l�ncolt list�t k�sz�tettem: A megszokott l�ncolt list�t�l elt�r�en, itt minden elem ak�r 4 m�sikhoz is kapcsol�dik,
 * s�t minden kapcsolatnak (�lnek a gr�fban) van s�lya is. A gr�fot reprezent�l� adatszerkezetet egy kicsi, 4x4-es p�lya p�ld�j�n kereszt�l fogom bemutatni,
 * �s hogy k�nnyebb legyen meg�rteni, k�sz�tettem hozz� egy �br�t is. Az �bra bal alj�n l�that� a p�lya. A gr�f cs�csai minden p�ly�n a rajt, a c�l, valamint
 * az el�gaz�sok (de a sima kanyarok nem!). Minden cs�cs strukt�r�ban van egy 4 elem� lista, aminek minden eleme egy �l strukt�ra. Egy �lnek k�t mez�je van: az �l s�lya,
 * �s annak a cs�csnak a pointere, ahov� az �l mutat. Mivel a pointereken csak egyik ir�nyban tudunk mozogni, ez�rt a gr�fban val�j�ban ha k�t cs�cs k�z�tt van kapcsolat,
 * akkor k�zt�k k�t, egym�ssal ellent�tes ir�ny�t�s� �l van, melyek s�lya megegyezik, �s ez a k�t cs�cs t�vols�ga. Az �leket tartalmaz� t�mb sorrendje megegyezik az Irany enum-mal,
 * teh�t pl. az elek[2] a lefele mutat� �let jelenti. Ha egy cs�csnak egy adott ir�nyban nincs szomsz�dja, akkor annak az �lnek a s�lya 0, pointere pedig NULL-ra mutat.
 * Ebben az adatstrukt�r�ban k�nnyed�n eljuthatunk b�rmelyik cs�csb�l a szomsz�djaihoz, ami nagyon fontos lesz a Dijkstra-algoritmus sor�n (err�l k�s�bb lesz sz�).
 * Azonban neh�zkes a gr�f �sszes cs�cs�t sorba venni, hiszen mivel a gr�fban vannak k�r�k, k�nnyen v�gtelen ciklusba keveredhet�nk. Ez�rt k�sz�t�nk egy olyan t�mb�t (list�t),
 * aminek minden eleme egy cs�csra mutat. Amikor ezt a t�mb�t meg�p�tj�k, figyel�nk arra, hogy minden cs�cs pontosan egyszer szerepeljen a t�mbben,
 * �s �gy ezut�n k�nnyen sorra vehetj�k az �sszes cs�csot, amire szint�n sz�ks�g�nk lesz k�s�bb. Fontos megeml�teni a t�mbbel kapcsolatban, hogy a cs�csok sorrendje nem �rdekel minket,
 * b�r a kezd� cs�cs mindig a nulladik helyen lesz. Fontos megeml�teni m�g, hogy a gr�f vagy a t�mb a program fut�sa sor�n soha nem fog �tm�retez�dni. A l�ncolt list�hoz hasonl�
 * adatszerkezetre az�rt volt sz�ks�g, hogy egy cs�cs szomsz�djait k�nnyen el�rj�k. Ezen a k�l�nleges l�ncolt list�n viszont nem tudunk csak egyszer�en v�gig menni, �s minden
 * elem�t vizsg�lni, ez�rt van sz�ks�g a sima t�mbre. A j�t�kot kb. 20 cella magas �s 20 cella hossz� szinten �rdekes j�tszani, �s egy ilyen szinten nem nagyon lesz 20-30-n�l
 * t�bb cs�cs. �gy igaz, hogy minden cs�csot gyakorlatilag t�bb helyen is t�rolunk, ez �sszes�g�ben nem fog sok helyet foglalni, viszont a tov�bbi algoritmusok meg�r�s�t nagyban seg�ti.
 * A cs�csok sz�ma akkor lehet sok, ha a szinten egym�s mellett t�bb oszlop vagy sor is utca, hiszen ekkor tulajdonk�ppen minden cella egy el�gaz�s. A program ilyenkor is
 * megfelel�en m�k�dik, azonban ilyen szinteket nincs �rtelme k�sz�teni, elv�grre csak n�gy ir�nyban lehet mozogni, �gy csak sok, egyenl� hossz�s�g� �tvonal-lehet�s�get j�nne l�tre,
 * ami az �tvonalv�laszt�si feladatot nem nehez�ten�.
 * \image html nhf_adatstrukt.png
 * \image latex nhf_adatstrukt.png
 *
 * A gr�fon k�v�l a program t�bbsz�r haszn�l m�g k�tdimenzi�s t�mb�t, amelynek minden �rt�ke a p�lya egy mez�j�re mutat, valamilyen form�ban. Ezek k�z�l a leggyakrabban haszn�lt
 * a Szintek strukt�ra terkep mez�je, ahova f�jlb�l t�lt�dnek be a p�ly�k (itt a szintek sz�ma m�g egy harmadik dimenzi�t is l�trehoz). Ezen k�v�l sz�ks�g van m�g egy ilyen t�mbre
 * a gr�f meg�p�t�sekor. Itt miut�n egy cs�csnak elk�sz�tett�k a strukt�r�j�t �s �sszek�t�tt�k a szomsz�djaival, egy ilyen t�mbbe berakjuk a r� mutat� pointert, arra az x-y helyre,
 * ahol a p�ly�n elhelyezkedik. �gy amikor a t�bbi cs�cs vizsg�l�s�n�l �jra visszajutunk ehhez a cs�cshoz, m�r tudjuk, hogy ezt a cs�csot nem kell �jra elk�sz�teni, s�t �ssze is
 * tudjuk k�tni a m�sik cs�ccsal. Amikor a gr�fb�l egy sima, egydimenzi�s t�mb�t csin�lunk, akkor is egy ugyanilyen t�mb�t haszn�lunk, �m annak �rtekei boolean �rt�kek, hiszen itt
 * a pointerre nincs sz�ks�g�nk. Az eddig felsorolt t�mb�k mind dinamikusan foglaltak voltak, ami k�zen fekv�, hiszen m�ret�k �lland�.
 *
 * Van azonban a programban egy olyan t�mb is, aminek m�rete nem �lland�, ez pedig az eredm�nyeket t�rol� t�mb, a toplista. Azonban ez is dinamikus t�mbk�nt van kialak�tva,
 * hiszen m�rete csak akkor v�ltozik, ha �j eredm�nyt kell betenni a list�ba, �s annak hossza nem �rte m�g el a t�zet. �gy maximum kilenc eredm�nyt kell egy ilyen realloc()-kor
 * m�solni, �s azt is csak ritk�n.
 *
 * \subsection algoritmus A program legfontosabb algoritmusai
 * A program legfontosabb algoritmusai mind a legr�videbb �tvonal meghat�roz�shoz sz�ks�gesek.
 * \subsubsection epit A gr�f �p�t�se
 * Az algoritmust megval�s�t� f�ggv�ny: graf_epit()
 *
 * Az els� l�p�s az ide�lis �tvonal meghat�roz�s�ban az egy gr�f �p�t�se, amire majd haszn�lhat� lesz a Dijkstra-algoritmus. A feladat teh�t az, hogy egy olyan k�tdimenzi�s
 * t�mbb�l, amiben minden mez� utca vagy �p�let, k�sz�ts�nk egy olyan gr�f adatstrukt�r�t, amiben azok a mez�k lesznek a cs�csok, ahol legal�bb 3 utca tal�lkozik, az �lek pedig
 * az �ket �sszek�t� utc�k. Erre egy rekurz�v algoritmus haszn�ltam. Hab�r a rajtban �s a c�lban csak k�t utca tal�lkozik, ezek is cs�csok lesznek a gr�fban, hiszen ezen k�t
 * pont k�z�tt keress�k a legr�videbb utat. Az algoritmus pszeudok�dja egy adott cs�csb�l (a rajtb�l ind�tjuk):
 * 1. Pr�b�ljunk meg elindulni a gr�fb�l egy ir�nyba!
 * 2. Ha arra utca van, akkor k�vess�k az utc�t eg�szen addig, am�g olyan mez�h�z nem �r�nk, aminek 3 szomsz�dja is utca, azaz itt el�gaz�s lesz. K�zben sz�moljuk h�nyat l�p�nk!
 * Ha arra �p�let volt, akkor ebben az ir�nyban a cs�csnak nincs �le.
 * 3. Amikor egy cs�cshoz jutottunk, vizsg�ljuk meg, hogy j�rtunk-e m�r ebben a cs�csban!
 * 4. Ha nem, akkor k�sz�ts�k el a cs�csot, �s hat�rozzuk meg, melyik ir�nyb�l �rkezt�nk ide.
 * 5. �ll�tsuk be a megfelel� �leket mindk�t cs�cson, hogy egym�sra mutassanak, a s�ly pedig a l�p�sek sz�ma legyen.
 * 6. Jegyezz�k meg, hogy j�rtunk m�r ebben a cs�csban is, hogy t�bbsz�r ne vizsg�ljuk meg.
 * 7. Alkalmazzuk ugyanezt az algoritmust most erre a cs�csra is!
 * 8. Ha a 3. pontban igen volt a v�lasz, akkor is k�sz�ts�k el a megfelel� �leket, azonban m�s dolgunk nincs, mert az a cs�cs m�r vizsg�lva volt (vagyis vizsg�lat alatt van,
 * csak k�zben a f�ggv�ny t�bbsz�r megh�vta mag�t, �gy m�g nem �rte k�rbe az �sszes ir�nyon.)
 * Az algoritmus k�zben ne felejts�k el sz�molni a cs�csok sz�m�t sem, hiszen ez alapj�n kell a megfelel� m�ret� mem�riater�letet foglalni ahhoz a list�hoz, amiben a cs�csokra
 * mutat� pointereket t�roltuk.
 *
 * \subsubsection kilapit A gr�f "kilap�t�sa"
 * Az algoritmust megval�s�t� f�ggv�ny: graf_kilapit()
 *
 * Miut�n meg van a gr�funk, ami gyakorlatilag egy k�tdimenzi�s l�ncolt lista, hiszen n�gyf�le ir�nyban tudunk benne mozogni, szeretn�nk egy egydimenzi�s t�mb�t is. Ennek nem kell
 * l�ncolt list�nak lennie, ennek csak az lesz a szerepe, hogy k�nnyen �s gyorsan v�gig tudjunk menni az �sszes cs�cson, szomsz�ds�gt�l f�ggetlen�l. Ehhez az el�z�h�z hasonl�
 * rekurz�v algoritmust haszn�lunk. Elindulunk a kezd� cs�csb�l mind a n�gy ir�nyba, �s ha olyan cs�csot tal�lunk, amit m�g nem raktunk bele a list�ba, akkor belerakjuk.
 *
 * \subsubsection dijkstra A Dijkstra-algoritmus
 * Az algoritmust megval�s�t� f�ggv�ny: dijkstra()
 *
 * Most hogy megvan a gr�f �s a lista is, alkalmazhatjuk a h�res Dijkstra-algoritmust,
 * ami egy gr�fban megtal�lja k�t cs�cs k�z�tt a legr�videbb utat. Ennek az algoritmusnak a pszeud�k�dja r�viden:
 * 1. A gr�f cs�csait rendezz�k k�t halmazba: vizsg�lt �s nem vizsg�lt. El�sz�r legyen minden a nem vizsg�lt halmazban.
 * 2. Minden cs�cshoz tartozzon egy mez�, ami azt adja meg, hogy milyen messze van a kezd� cs�cst�l. Ezt el�sz�r �ll�tsuk minden cs�csn�l
 * v�gtelenre (ez esetben 10 000-re �ll�tottam, ezt sem fogja semmilyen szint t�ll�pni), kiv�ve a kezd� cs�csn�l, ott legyen 0.
 * 3. Minden cs�cshoz tartozzon egy el�z� mez� is, ami r�mutat az el�z� cs�csra, vagyis hogy ha a kezd� cs�csb�l ebbe a cs�csba a legr�videbb �ton j�v�nk,
 * akkor melyik cs�csb�l �rkezn�nk ide. Ez a mez� legyen �res minden cs�csn�l kezd�snek.
 * 4. A k�vetkez� r�szt addig ism�telj�k, am�g a meg nem vizsg�lt cs�csok halmaza nem �res.
 * 5. V�lasszuk ki a meg nem vizsg�lt cs�csok k�z�l azt, amelyiknek a t�vols�ga a kezd� cs�cst�l a legnagyobb.
 * 6. Ha ez a cs�cs a c�l, akkor v�gezt�nk, kil�phet�nk. (�gy ugyan nem lesz meg minden pontnak a t�vols�ga a kezd� pontt�l, de erre nincs is sz�ks�g ebben a programban.
 * A c�l t�vols�ga a kezd� cs�cst�l pedig ezut�n m�r nem v�ltozna az algoritmus fut�sa sor�n.)
 * A keresett t�vols�g a c�l cs�cs t�vols�g mez�j�ben van, az el�z� mez�ket k�vetve pedig megkapjuk az �tvonalat a kezd� cs�csig.
 * 7. Keress�k fel a cs�cs �sszes, eddig m�g nem vizsg�lt szomsz�dj�t! Minden szomsz�dos cs�cs eset�n sz�moljuk ki, hogy milyen hossz� �ton jutunk el oda, ha az �ppen
 * vizsg�lt cs�cson kereszt�l megy�nk. Ezt �gy kapjuk meg, hogy a vizsg�lt cs�cs t�vols�g�hoz hozz�adjuk a szomsz�dos cs�csba vezet� �l s�ly�t.
 * Ha ez az �rt�k kisebb, mint a szomsz�dos cs�cs t�vols�g mez�je, akkor friss�ts�k azt, �s az el�z� mez�t �ll�tsuk be, hogy a vizsg�lt cs�csra mutasson.
 * Ezt a l�p�st ism�telj�k a vizsg�lt cs�cs �sszes, eddig m�g nem vizsg�lt szomsz�dj�ra.
 * 8. A vizsg�lt cs�csot rakjuk �t a nem vizsg�lt halmazb�l a vizsg�lt halmazba.
 * 9. Ugorjunk vissza a 4. l�p�shez.
 *
 * Hab�r a pszeud�k�db�l kev�sb� l�tszik, nagy sz�ks�g volt a sima, egydimenzi�s list�ra is az algoritmus k�zben, hiszen nagyban leegyszer�s�tette azt, hogy megmondjuk, h�ny
 * cs�csot nem vizsg�ltunk m�g (van_nem_latogatott()) �s azt is, hogy megmondjuk, melyik a legk�zelebbi cs�cs (legkozelebbi()).
 *
 * \subsubsection matrix Bool m�trix l�trehoz�sa
 * Az algoritmust megval�s�t� f�ggv�ny: matrix_letrehoz()
 *
 * Az algoritmus fut�sa ut�n m�g fontos, hogy az ide�lis �tvonalat olyan form�ban kell �tadni a f�ggv�nynek, ami a k�perny�re rajzolja a t�rk�pet, azaz minden
 * mez�r�l egy�rtelm�en el lehessen d�nteni, arra ment-e az ide�lis �tvonal. Az algoritmus fut�sa ut�n csak egym�sra mutat� cs�csokat kapunk, amik a t�rk�pen
 * el�gaz�sokat jelentenek, m�g ezeket is �ssze kell k�tni. Teh�t egy olyan k�tdimenzi�s t�mb�t fogunk l�trehozni, aminek egy mez�je pontosan akkor igaz, ha
 * arra megy az ide�lis �tvonal. Ehhez a c�lb�l indulunk, �s k�vetj�k a cs�cs strukt�r�j�ban, a Dijsktra-algoritmus �ltal meghat�rozott el�z� mez�t, azaz hogy melyik
 * cs�cs fel� kell indulnunk. Ahhoz, hogy minden egyes mez�t a k�t cs�cs k�z�tt megjel�lj�nk, �jra felhaszn�ljuk a gr�f �p�t�s�n�l m�r haszn�lt f�ggv�nyeket. (utak_szama(),
 * kovi_utca())
 *
 * �s m�ris k�szen vagyunk az ide�lis �tvonal meghat�roz�s�val, amit m�r egy egyszer� nyomtat� f�ggv�nnyel meg tudunk mutatni a j�t�kosnak.
 *
 * \subsection test A program tesztel�se
 * A program tesztel�s�t f�leg k�zzel v�geztem, azaz min�l t�bbsz�r, min�l t�bbf�le szinttel pr�b�ltam. A mem�riakezel�shez viszont haszn�ltam a Valgrind nev� programot.
 * Mivel ez a program Windowson nem m�k�dik, WSL-ben, a Clion-on kereszt�l tudtam futtatni, �s a program jelenlegi verzi�j�ra nem jelez semmilyen hib�t.
 *
 * \section fajlok A program �ltal olvasott vagy �rt f�jlok
 * A program n�gy f�jlt kezel, amiknek a szerkeszt�se nem sz�ks�ges a program fut�s�hoz, de mivel komoly tartalmi megk�t�sek vannak, ha egyszer beleny�lsz, k�nnyen hib�kba �tk�zhetsz.
 * Ebben a fejezetben le�rom, hogy �rj olyan f�jlokat, amiket a program elfogad, a k�vetkez�ben pedig abban seg�tek, hogy megpr�b�lom megmondani, hol
 * rontottad el, a hiba�zenet alapj�n.
 *
 * A program h�rom f� f�jlja ".fs" kiterjeszt�st alkalmaz, amit csak az�rt haszn�ltam, hogy a kev�sb� hozz��rt�k ne merjenek, vagy tudjanak beleny�lni.
 *
 * \subsection szintek "levels.fs"
 * Ez a f�jl tartalmazza a szintek sz�veges reprezent�ci�j�t, �s ez tal�n az egyetlen, amibe egy felhaszn�l�nak �rdemes lehet beleny�lnia, ha m�r megunta az alap szinteket.
 * Term�szetesen ha m�s szinteket t�ltesz be, akkor a toplist�nak nem lesz sok �rtelme, de a program ezt nem fogja tudni.
 *
 * A f�jl els� sor�ban h�rom eg�sz sz�mnak kell �llnia, sz�k�zzel elv�lasztva.
 * 1. A f�jlban szerepl� szintek sz�ma. Ha kevesebb szint van a f�jlban, hib�t fog dobni a program, ha t�bb, akkor a marad�kot figyelmen k�v�l hagyja.
 * 2. A szintek magass�ga (azaz h�ny karakter magasak) Fontos, hogy ez az �sszes szintre vonatkozik! Ha egy szint is nem ilyen magas, hib�t fog dobni a program.
 * 3. A szintek hossz�s�ga (ugyanez igaz erre is, mint a magass�gra)
 * Ebb�l teh�t egy�rtelm�, hogy a j�t�k �ltal egyszerre bet�lt�tt szinteknek a m�rete meg kell hogy egyezzen.
 *
 * Ezut�n j�nnek a szintek sorai. Minden szinthez annyi sor �s a sorokban annyi karakter tartozik, amennyit az els� sorban meghat�rozott a f�jl. Minden karakter vagy
 * "U" (utca) vagy "E" (�p�let) kell hogy legyen, majd egy sort�r�s. A szinteket egy �res sor v�lassza el egym�st�l. Ha a f�jl b�rmilyen m�s karaktert tartalmaz, hib�t fog
 * dobni a program.
 *
 * \subsection toplista "toplista.fs"
 * Ez a f�jl tartalmazza az aktu�lis toplist�t. Beleny�lni �s �t�rni az eredm�nyedet csal�s! Ha valamit �t�rt�l benne, ink�bb t�r�ld a f�jlt, �s az els� alkalomkor
 * a program el fogja k�sz�teni az �res toplist�t. De az�rt le�rom, hogy kell haszn�lni...
 *
 * A f�jl els� sor�ban egyetlen sz�mnak kell lennie, ami azt jelenti, hogy h�ny eredm�ny van a f�jlban. Ez a sz�m nem lehet nagyobb t�zn�l, hiszen a program a legjobb 10
 * eredm�nyt t�rolja. Ezut�n minden egyes sor egy eredm�nyt jelent. Egy sor k�t sz�m kezd:
 * 1. Az adott eredm�ny helyez�se, 0-t�l indexelve (a holtversenyek miatt fontos)
 * 2. Az adott eredm�ny pontja
 * Ezeket sz�k�zzel kell elv�lasztani, majd �jabb sz�k�z ut�n j�n a j�t�kos neve, eg�szen a sor v�g�ig (sz�k�zt is tartalmazhat)
 *
 * \subsection savegame "savegame.fs"
 * Na ebbe a f�jlba azt�n v�gk�pp csal�s beleny�lni, ugyanis itt t�rol�dik egy lementett j�t�knak az adatai. A f�jl mind�sszesen k�t sz�mot tartalmazhat, sz�k�zzel elv�lasztva:
 * 1. A teljes�tett szintek sz�ma (azaz a k�vetkez� szint indexe)
 * 2. Az eddig el�rt pontok �sszege
 *
 * \subsection errors "errorlog.txt"
 * Ha a program fut�sa sor�n valamilyen hiba l�p fel, akkor a program a k�perny�re �r�s mellett ide is menti a hibak�dot �s -�zenetet. Az el�z� hiba�zenet akkor t�rl�dik,
 * amikor k�vetkez�re szab�lyosan l�p�nk ki a programb�l. Ennek a f�jlnak a szerkeszt�se teljesen felesleges, a program nem olvas bel�le. A k�vetkez� fejezet fog foglalkozni
 * az egyes hiba�zenetek pontos jelent�s�vel.
 *
 * \section hibak Hibak�dok �s hiba�zenetek
 *  Ha a program fut�sa sor�n valamilyen hiba l�p fel, akkor a program a k�perny�re �r�s mellett ide az errorlogs.txt f�jlba menti a hibak�dot �s -�zenetet. Itt le fog �rni,
 *  mi okozhatja az egyes hib�kat, �s hogy jav�tsuk �ket. Ha b�rmi egy�b hiba l�pne fel, vagy nem siker�l megjav�tani, keress engem b�tran: feketesamu(kuakc)gmail(pont)com
 *
 *  \subsubsection top1 10, �rv�nytelen toplista f�jl!
 *  A program nem tudta megnyitni a "toplista.fs" f�jlt se olvas�, se �r� m�dban. Lehets�ges, hogy nincs jogosults�ga ehhez ennek a programnak?
 *
 *  \subsubsection top2 11, �rv�nytelen toplista f�jl!
 *  A toplista.fs f�jl els� sora �rv�nytelen, mind�ssze egy sz�mot kellene tartalmaznia! L�sd az el�z� fejezetet a pontos le�r�s�rt!
 *
 *  \subsubsection top3 12, �rv�nytelen toplista f�jl!
 *  A toplista.fs f�jl valamelyik sor�ban nem siker�lt k�t sz�m beolvas�sa, val�sz�n�leg valamelyik hi�nyzik. L�sd az el�z� fejezetet a pontos le�r�s�rt!
 *
 *  \subsubsection save 13, A j�t�kment�s f�jlja �rv�nytelen!
 *  A program nem tudta megnyitni a "savegame.fs" f�jlt �r� m�dban. Lehets�ges, hogy nincs jogosults�ga ehhez ennek a programnak?
 *
 *  \subsubsection szint1 14, A szinteket tartalmaz� f�jl �rv�nytelen vagy nem l�tezik!
 *  A program nem tudta olvas� m�dban megnyitni a "levels.fs" f�jlt. Lehets�ges, hogy nem l�tezi? A nhf.exe f�jlal egy mapp�ban kell lennie!
 *
 *  \subsubsection szint2 15, A szinteket tartalmaz� f�jl �rv�nytelen!
 *  A "levels.fs" f�jl els� sora �rv�nytelen, nem siker�lt a 3 sz�m beolvas�sa. L�sd az el�z� fejezetet a pontos le�r�s�rt!
 *
 *  \subsubsection szint3 16, A szinteket tartalmaz� f�jl �rv�nytelen!
 *  A "levels.fs" f�jl valamelyik sor�ban �rv�nytelen karakter van. L�sd az el�z� fejezetet a pontos le�r�s�rt!
 *
 *  \subsubsection mem 2, Nincs el�g mem�ria!
 *  A program fut�sa sor�n mem�ri�t k�rt az oper�ci�s rendszert�l, de az nem volt k�pes teljes�teni a k�v�ns�g�t. Mivel a mai sz�m�t�g�pek mem�ri�i el�g nagyok
 *  az �n programomnak sz�ks�ges mem�riater�let�hez k�pest, csak �gy tudom elk�pzelni ezt a hiba�zenetet, hogy valamelyik f�jl els� sor�ba egy irdatlan nagy sz�mot �rt�l.
 *
 * \section linux Futtat�s Linuxon
 * A programot Windows-on fejlesztettem, de Windows Subsystem for Linux-on tesztelve lett. Mivel a k�t oper�ci�s rendszer m�shogy kezeli a f�jlokban a sork�z jelet,
 * az alap k�d windowson fog j�l m�k�dni. A Linuxon futtat�shoz (legal�bbis nekem csak �gy m�k�d�tt) ki kell venni a kommentet jelz� k�t "/" jelet a k�vetkez�re helyekr�l:
 * szintek.c, 31., 48. �s 51. sor. Ezut�n �jra kell ford�tani a programk�dot, �s elvileg m�k�dni fog a szintek beolvas�sa. A toplista f�jln�l is hasonl� hiba l�phet fel.

 */

/*! @file main.c
 * \brief A program f� f�jlja, ami a f�me�t kezeli �s megh�vja a megfelel� modul megfelel� f�ggv�ny�t.
 *
 */

/**
 * Be�ll�tja a konzol k�dol�s�t, kezeli a f�men�t �s megh�vja a megfelel� f�ggv�nyeket.
 * @return Kil�p�si k�d, de val�j�ban soha nem ezzel l�p ki a program, hanem a kilep() f�ggv�nynek megadott k�ddal.
 */
int main() {
    #ifdef _WIN32
        SetConsoleCP(1250);
        SetConsoleOutputCP(1250);
    #endif
    econio_set_title("�tvonalv�laszt-O");
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
                kilep(0, "K�szi a j�t�kot!", (Szintek) {NULL});
            default:;
        }
    }
}