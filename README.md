# IOS – projekt 2 (synchronizace)

## Autor: Jan Novák <xnovak3i@stud.fit.vutbr.cz>
Zadání je inspirováno knihou Allen B. Downey: The Little Book of Semaphores

## Popis Úlohy (Building H2O)
Molekuly vody jsou vytvářeny ze dvou atomů vodíku a jednoho atomu kyslíku. V systému jsou tři typy
procesů: (0) hlavní proces, (1) kyslík a (2) vodík. Po vytvoření procesů se procesy reprezentující
kyslíky a vodíky řadí do dvou front—jedna pro kyslíky a druhá pro vodíky. Ze začátku fronty vždy
vystoupí jeden kyslík a dva vodíky a vytvoří molekulu. V jednu chvíli je možné vytvářet pouze jednu
molekulu. Po jejím vytvoření je prostor uvolněn dalším atomům pro vytvoření další molekuly. Procesy,
které vytvořily molekulu následně končí. Ve chvíli, kdy již není k dispozici dostatek atomů kyslíku
nebo vodíku pro další molekulu (a ani žádné další již nebudou hlavním procesem vytvořeny) jsou
všechny zbývající atomy kyslíku a vodíku uvolněny z front a procesy jsou ukončeny.

## Specifikace programu

### Spuštění programu
 
Program se spouští s následujícími parametry:
```
$ ./proj2 NO NH TI TB
```
- NO: Počet kyslíků
- NH: Počet vodíků
- TI: Maximální čas milisekundách, po který atom kyslíku/vodíku po svém vytvoření čeká, než se
zařadí do fronty na vytváření molekul. 0<=TI<=1000
- TB: Maximální čas v milisekundách nutný pro vytvoření jedné molekuly. 0<=TB<=1000

### Chybové stavy:
- Pokud některý ze vstupů nebude odpovídat očekávanému formátu nebo bude mimo povolený
rozsah, program vytiskne chybové hlášení na standardní chybový výstup, uvolní všechny dosud
alokované zdroje a ukončí se s kódem (exit code) 1.
- Pokud selže některá z operací se semafory, nebo sdílenou pamětí, postupujte stejně--program
vytiskne chybové hlášení na standardní chybový výstup, uvolní všechny dosud alokované
zdroje a ukončí se s kódem (exit code) 1.

### Implementační detaily:
- Každý proces vykonává své akce a současně zapisuje informace o akcích do souboru s názvem
proj2.out. Součástí výstupních informací o akci je pořadové číslo A prováděné akce (viz popis
výstupů). Akce se číslují od jedničky.
- Použijte sdílenou paměť pro implementaci čítače akcí a sdílených proměnných nutných pro
synchronizaci.
- Použijte semafory pro synchronizaci procesů.
- Nepoužívejte aktivní čekání (včetně cyklického časového uspání procesu) pro účely
synchronizace.
- Pracujte s procesy, ne s vlákny.

### Hlavní proces
- Hlavní proces vytváří ihned po spuštění NO procesů kyslíku a NH procesů vodíku.
- Poté čeká na ukončení všech procesů, které aplikace vytváří. Jakmile jsou tyto procesy
ukončeny, ukončí se i hlavní proces s kódem (exit code) 0.

### Proces Kyslík
- Každý kyslík je jednoznačně identifikován číslem idO, 0<idO<=NO
- Po spuštění vypíše: A: O idO: started
- Následně čeká pomocí volání usleep náhodný čas v intervalu <0,TI>
- Vypíše: A: O idO: going to queue a zařadí se do fronty kyslíků na vytváření molekul.
- Ve chvíli, kdy není vytvářena žádná molekula, jsou z čela front uvolněny kyslík a dva vodíky.
Příslušný proces po uvolnění vypíše: A: O idO: creating molecule noM (noM je číslo molekuly,
ty jsou číslovány postupně od 1).
- Pomocí usleep na náhodný čas v intervalu <0,TB> simuluje dobu vytváření molekuly.
- Po uplynutí času vytváření molekuly informuje vodíky ze stejné molekuly, že je molekula
dokončena.
- Vypíše: A: O idO: molecule noM created a proces končí.
- Pokud již není k dispozici dostatek vodíků (ani nebudou žádné další vytvořeny/zařazeny do
fronty) vypisuje: A: O idO: not enough H a proces končí.

### Proces Vodík
- Každý vodík je jednoznačně identifikován číslem idH, 0<idH<=NH
- Po spuštění vypíše: A: H idH: started
- Následně čeká pomocí volání usleep náhodný čas v intervalu <0,TI>
- Vypíše: A: H idH: going to queue a zařadí se do fronty vodíků na vytváření molekul.
- Ve chvíli, kdy není vytvářena žádná molekula, jsou z čela front uvolněny kyslík a dva vodíky.
Příslušný proces po uvolnění vypíše: A: H idH: creating molecule noM (noM je číslo molekuly,
ty jsou číslovány postupně od 1).
- Následně čeká na zprávu od kyslíku, že je tvorba molekuly dokončena.
- Vypíše: A: H idH: molecule noM created a proces končí.
- Pokud již není k dispozici dostatek kyslíků nebo vodíků (ani nebudou žádné další
vytvořeny/zařazeny do fronty) vypisuje: A: H idH: not enough O or H a process končí.
