# Othello
Konsolbaserat program för körning av strategispelet Othello.
## Sammanfattning
Othello är ett spel mellan två personer som spelas på ett 8x8 rutnät. Spelare tävlar om att få så många brickor i sin färg som möjligt.
Varje bricka har två sidor; en svart och en vit. Genom att fånga brickor kan man vända dem till sin färg. I det här programmet kan man välja att 
spela mot dator eller mot en annan person.
## Bakgrund
Jag ville skapa ett program där man kunde spela Othello eftersom det är ett väldigt enkelt koncept, men väldigt svårt att bemästra. Dessutom ville jag
kunna köra mot en dator på olika svårighetsgrader samt testa att låta datorn möta sig själv. På nätet kan man hitta flera olika sidor att köra Othello, men
de flesta fokuserar på spelare vs spelare i olika online-turneringar och likande. Jag ville att datorn och statistiken var mer i fokus. Dessutom tyckte jag det lät
kul att försöka på ett effektivt sätt programmera ett flexibelt sätt att representera och skriva ut spelplanen med olika grafiska tekniker (såsom animationer och färger).
## Nyckelaspekter
Programmet är uppbyggd på ett sådant sätt att den huvudsakliga utskriften sker i en annan fil, den så kallade Render-Motorn. Denna lagrar statusen på displayen och håller koll på animationer och färger. På så sätt kan spellogiken hållas helt separat från utskriftslogiken, med undantag för några funktionsanrop. Detta gör det enkelt att bygga på med nya funktioner.
## Hur används programmet
När programmet startas, syns startmenyn. Här väljer man alternativ för spelarna med hjälp av piltangenter och Enter. Väljer man att köra mot datorn, får man också välja svårighetsgrad.&nbsp;<br>
![othello-output-menu](https://github.com/user-attachments/assets/ca03b7d4-2e3c-4d33-a446-09ff9d7747d6)
&nbsp;<br>&nbsp;<br>
Väl inne i matchen väljer man var man vill ligga med piltangenter eller WASD och Enter/Space. Rutorna man kan lägga på markeras med ett plus-tecken. Rutan där förra spelaren lade är gul. Om man inte har någonstans att lägga, går turen över till nästa spelare. Till höger syns hur många brickor varje spelare har, samt vems tur det är. Genom att klicka på 'r' kan man återställa fönstret om någonting skulle gå snett med utmatningen.
&nbsp;<br>
![othello-output](https://github.com/user-attachments/assets/2dad2cac-52b9-48c1-b7f4-d8401d0333d5)
&nbsp;<br>&nbsp;<br>
Matchen avslutas när ingen spelare kan göra något drag. Då får man alternativet att antingen köra en ny match, varpå startmenyn öppnas, eller att avsluta programmet.

## Utmaningar
Eftersom programmet enbart använder konsolen för utskrift är det beroende av ASCII-konst och ANSI-koder. Exempelvis kan man inte rita avancerade former eller ändra textstorlek. Dessutom är det inte säkert att programmet fungerar på andra operativsystem än Windows. Detta skulle kunna förbättras med olika libraries, såsom SFML eller Ncurses. Programmet använder inte heller flera kärnor, vilket hämmar prestandan. Genom att använda threads-delen av standardbiblioteket skulle man kunna få bättre prestanda och hålla input, logik och utskrift separat.
## Vad härnäst?
Följande saker står näst på tur över framtida uppdateringar:
* Snyggare meny med fler alternativ för utseende, statistik och datorlogik.
* Animation när brickor placeras och vänds, så att det syns tydligare var en spelare lägger.
* Animera menyn.
* Se vems tur det är på brädet, förslagsvis genom att plus-tecknen som signalerar ett möjligt drag ändrar färg.
* Möjlighet att lämna en match innan den avslutas
* Att kunna spara ner en match i filer
* Att kunna spara statistik i filer
* En visualisering av datorns hjärna, kanske ett träd där man kan se alla drag som söks igenom.
* Visa FPS
