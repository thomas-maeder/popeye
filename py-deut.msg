# Dies ist py-deut.msg. Sie enthaelt die deutschen Texte
# die Popeye benoetigt wenn er deutsch sprechen soll.
#
# Das Program genpystr liest diese Datei, und erzeugt
# daraus eine Datei py-deut.str, welche in dem Format
# vorliegt, das Popeye versteht.
#
# Diese Datei hier enthaelt pro Zeile einen Text. Jeder
# Text erhaelt eine Nummer. Diese Nummern muessen mit den
# symbolischen Konstanten in pymsg.h uebereinstimmen, und
# in aufsteigender Reihenfolge ohne Luecken angegeben
# werden.
# Jede Zeile dieser Datei die einen Text einleitet, beginnt
# mit Nummer des Textes gefolgt von einem '=' und einem
# Leerzeichen. Alle Zeichen hinter diesem Leerzeichen
# werden als Text bis zum Zeilenende uebernommen.
# Es gibt folgende Ausnahmen:
#  - Die Zeile beginnt mit einem '#'. Solche Zeilen
#    werden ueberlesen.
#  - Die Zeile endet mit einem '\'. Die naechste Zeile
#    gilt dann als Fortsetzungszeile und beginnt nicht
#    mit einer Ziffer. Die gesammte Laenge einer Zeile
#    darf 256 nicht uebersteigen.
#  - Hinter '\' koennen Buchstaben erscheinen, die in die
#    entsprechenden ASCII-Steuerzeichen umgesetzt werden:
#      'n': Zeilenwechsel
#      'r': Wagenruecklauf
#      't': Tabulator
#      'f': Seitenvorschub
#      '\': Das Zeichen '\' selbst.
#
0= Fataler Fehler:\n
1= Es fehlt ein weisser oder schwarzer Koenig\n
2= Zu wenig Hauptspeicher verfuegbar\n
3= Bearbeitung abgebrochen.
4= Unmotivierte Eingabe von RexInklusive oder Exakt
5= Zuviele Loesungen
6= Im HaanerSchach sind Kamikaze-Steine nicht erlaubt
7= Verfuehrung nicht anwendbar\n
8= Zu viel Maerchenschach fuer neutrale Steine
9= Leofamily mit orthodoxen Figuren nicht erlaubt
10= Dummy-Figuren bei Circe nicht erlaubt.
11= FarbeWechselnd nur bei einigen Huepfern erlaubt
12= Springer und Cavalier Majeur nicht erlaubt
13= Circe-Wiedergeburt ausserhalb des Brettes ?
14= Satzspiel mit Koenig im Schach ?
15= Kamikaze + Neutrale Steine oder Kamikaze + Circe + Maerchenfiguren nicht gleichzeitig erlaubt
16= Chromes Schach, Gitterschach, Koeko, Randzueger oder Neutral unvereinbar mit Volage
17= Laengst-/Kuerzestzueger/Schlagzwang/Duellantenschach/AlphabetischesSchach gleichzeitig fuer die gleiche Partei ?
18= \nDie am Zug befindliche Partei kann den Koenig schlagen
19= Monochromes und Bichromes Schach gleichzeitig ergibt keinen Sinn
20= Unzulaessiges Zeichen: `%s'
21= Nur ein Koenig erlaubt
22= Falsch beendete Feldliste - ignoriert
23= Falsche oder fehlende Feldliste - bitte eine eingeben
24= StartZugnummer ausserhalb der Grenzen: %s
25= Eiffelschach, Madrasi, Isardam, Woozles oder paralysierende Steine unvereinbar mit manchen Maerchenbedingungen
26= Circe und Immunschach und RexInklusive nicht erlaubt
27= schwarz
28= weiss
29= Manche Maerchenfiguren mit Laengst-/Kuerzestzueger, Heffalumps unvereinbar
30= Zuviele Zuege in der Forderung
31= Keine koeniglichen Figuren im PlatzwechselCirce oder RexInklusive\n
32= Undefinierter Fehler %s
33= Stoerende Eingabe: %s
34= Eingabe-Fehler:
35= Falsche oder fehlende Steinbezeichnung - wird ignoriert
36= Steinbezeichnung nicht eindeutig
37= Nur eine Angabe von Weiss, Schwarz oder Neutral ist erlaubt - ignoriert
38= Bedingung nicht erkannt
39= Option nicht eindeutig - ignoriert
40= Keine oder falsche Zahlenangabe
41= Option nicht erkannt - Einlesen von Optionen beendet
42= Kommando nicht eindeutig - wird ignoriert
43= Kommando unbekannt
44= Keine Forderung angegeben
45= Datei kann nicht zum Schreiben geoeffnet werden
46= Datei kann nicht zum Lesen geoeffnet werden
47= Zu viele geschachtelte Eingabedateien
48= Keine Farbe angegeben - alle Spezifikationen ignoriert
49= Forderung nicht bekannt
50= Bedingung nicht eindeutig - ignoriert
51= Ende der Eingabe vor EndeProblem
52= Eingabezeilen Ueberlauf
53= Kein AnfangProblem
54= Interner Popeye Fehler in tk switch
55= Loesung beendet.
56= Zugzwang.
57= Drohung:
58= Aber
59= Zeit = 
60= Matt in einem Zug durch\n
61= \n
62= Zuviel Maerchenschach fuer Imitator
63= Zuviele Imitatoren
64= Kamikaze-Koenige nur in Circe RexInklusive
65= Steingewinn Aufgaben mit Circe Parrain sind Frevel
66= Problem uebersprungen
67= AntiCirce mit einigen anderen Bedingungen nicht erlaubt
68= EinsteinSchach nicht mit Maerchenfiguren erlaubt
69= SuperCirce unvertraeglich mit einigen Maerchenbedingungen
70= Stellungen: %s, Trefferrate:
71= Chamaeleonschach und Chamaeleonsteine nicht gleichzeitig erlaubt
72= Pruefe als
73= -Zueger.
74= Forderung wird nicht unterstuetzt.\n
75= Mehrere Ziele nicht erlaubt, wenn eines davon dia oder a=>b ist\n
76= Es koennen maximal zwei Felder fuer ep-Schluessel eingegeben werden.\n
77= Abbruch durch Signal %d nach %s\n
78= transmutierende/reflektierende koenigliche Steine nicht erlaubt.\n
79= Drehung/Spiegelung nicht erlaubt: 
80= \nEin Stein wird ueber den Brettrand verschoben.
81= Ferner muss als erstes nach Zwilling angegeben werden.\n
82= Widerlegt.
83= FrischAufCirce: Stein ist keine Umwandlungsfigur, gg. Steine zuerst eingeben.\n
84= Manche Maerchenbedingungen in Beweispartien nicht erlaubt.\n
85= Isardam und Madrasi/Eiffelschach nicht erlaubt
86= Ohneschach oder ExklusivSchach: kann nicht entscheiden, ob ein Zug legal ist.\n
87= ist bereits besetzt - alter Stein wird geloescht.\n
88= MarsCirce, PlusSchach oder PhantomSchach unvereinbar mit verschiedenen Maerchenbedingungen und -steinen.\n
89= moegliche Stellungen in
90= Unsinnige Kombination
91= VogtlanderSchach nicht erlaubt in Verbindung mit Isardam oder BrunnerSchach\n
92= Circe Assassin unvereinbar mit verschiedenen Maerchenbedingungen und -steinen\n
93= Nur Mattforderung im RepublikanerSchach erlaubt\n
94= , erhoehe HashRateLevel auf %s\n
95= , vermindere HashRateLevel auf %s\n
96= Intelligent Modus nur im h#/=, ser-#/= und ser-h#/=, wenigen Maerchenbedingungen und keinen Maerchensteinen.\n
97= Feld leer - kann keine Figur entfernen/versetzen.\n
98= Option  MaxZeit  nicht unterstuetzt.\n
99= Option  StopNachKurzloesungen  nicht unterstuetzt im direkten Spiel.\n
100= Maerchensteine nicht erlaubt in Single box
101= LaTeXdatei nicht uebersetzbar: Maerchenstein nicht spezifiziert.\n
102= Mehr als %s verschiedene Jaegertypen\n
103= Unvereinbare koenigliche Felder und/oder Steine\n
104= Schlagschach unvereinbar mit Schach- oder Mattforderungen\n
105= Take&MakeSchach unvereinbar mit verschiedenen Maerchenbedingungen\n
106= Magische Steine unvereinbar mit verschiedenen Maerchenfiguren\n
107= Zu viele magische Berechnungen benoetigt\n
108= Republikanerschach nicht mit Bedingungen, die davon abhangen, ob gewisse Zuege Schach bieten.\n
109= Es ist nicht feststellbar, wer am Zug ist\n
110= Option \"Drohung\" nicht anwendbar order maximale Laenge der Drohung zu gross\n
111= Geisterschach und Spukschach unvereinbar mit Circe, Anticirce, Kamikaze, Haaner Schach\n
112= WeissBeginnt nicht anwendbar - ignoriert\n
113= Satzspiel nicht anwendbar - ignoriert\n
114= NachSchluessel nicht anwendbar - ignoriert\n
115= Quodlibet nicht anwendbar - ignoriert\n
116= ZielIstEnde nicht anwendbar - ignoriert\n
117= ZielIstEnde und Quodlibet nicht kompatibel\n
118= Es ist nicht entscheidbar, ob Bedingung fuer Weiss oder Schwarz gilt\n
119= KaefigImmunSchach nur mit KaefigCirce\n
