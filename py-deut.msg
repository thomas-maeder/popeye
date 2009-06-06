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
7= Verfuehrung in weniger als zwei Zuegen
8= Neutrale Steine und Orphan oder Freund nicht erlaubt
9= Leofamily mit orthodoxen Figuren nicht erlaubt
10= Dummy-Figuren bei Circe nicht erlaubt.
11= FarbeWechselnd nur bei einigen Huepfern erlaubt
12= Springer und Cavalier Majeur nicht erlaubt
13= Circe-Wiedergeburt ausserhalb des Brettes ?
14= Satzspiel mit Koenig im Schach ?
15= Kuerzest-/Laengst-/Verfolgungszueger/Schlagzwang/Duellantenschach mit Neutralen Steinen nicht erlaubt
16= Kamikaze + Neutrale Steine oder Kamikaze + Circe + Maerchenfiguren nicht gleichzeitig erlaubt
17= Chromes Schach, Gitterschach, Koeko, Randzueger oder Neutral unvereinbar mit Volage
18= Laengst-/Kuerzestzueger/Schlagzwang/Duellantenschach/AlphabetischesSchach gleichzeitig fuer die gleiche Partei ?
19= Die am Zug befindliche Partei kann den Koenig schlagen
20= Monochromes und Bichromes Schach gleichzeitig ergibt keinen Sinn
21= Unzulaessiges Zeichen: `%s'
22= Nur ein Koenig erlaubt
23= Falsch beendete Feldliste - ignoriert
24= Falsche oder fehlende Feldliste - bitte eine eingeben
25= StartZugnummer ausserhalb der Grenzen: %s
26= Eiffelschach, Madrasi, Isardam, Woozles oder paralysierende Steine unvereinbar mit manchen Maerchenbedingungen
27= Circe und Immunschach und RexInklusive nicht erlaubt
28= schwarz
29= weiss
30= Manche Maerchenfiguren mit Laengst-/Kuerzestzueger, Heffalumps unvereinbar
31= Bei Koeko ist Circe mit Neutralen Steinen nicht erlaubt
32= Zuviele Zuege in der Forderung
33= Keine koeniglichen Figuren im PlatzwechselCirce oder RexInklusive\n
34= Undefinierter Fehler %s
35= Stoerende Eingabe: %s
36= Eingabe-Fehler:
37= Falsche oder fehlende Steinbezeichnung - wird ignoriert
38= Steinbezeichnung nicht eindeutig
39= Nur eine Angabe von Weiss, Schwarz oder Neutral ist erlaubt - ignoriert
40= Bedingung nicht erkannt
41= Option nicht eindeutig - ignoriert
42= Keine oder falsche Zahlenangabe
43= Option nicht erkannt - Einlesen von Optionen beendet
44= Kommando nicht eindeutig - wird ignoriert
45= Kommando unbekannt
46= Keine Forderung angegeben
47= Datei kann nicht zum Schreiben geoeffnet werden
48= Datei kann nicht zum Lesen geoeffnet werden
49= Zu viele geschachtelte Eingabedateien
50= Keine Farbe angegeben - alle Spezifikationen ignoriert
51= Forderung nicht bekannt
52= Bedingung nicht eindeutig - ignoriert
53= Ende der Eingabe vor EndeProblem
54= Eingabezeilen Ueberlauf
55= Kein AnfangProblem
56= Interner Popeye Fehler in tk switch
57= Loesung beendet.
58= Zugzwang.\n
59= Drohung:\n
60= Aber\n
61= Zeit = 
62= Matt in einem Zug durch\n
63= \n
64= Zuviel Maerchenschach fuer Imitator
65= Zuviele Imitatoren
66= Kamikaze-Koenige nur in Circe RexInklusive
67= Steingewinn Aufgaben mit Circe Parrain sind Frevel
68= Problem uebersprungen
69= neutrale Steine und Echecs Bicolores nicht erlaubt
70= AntiCirce mit einigen anderen Bedingungen nicht erlaubt
71= EinsteinSchach nicht mit Maerchenfiguren erlaubt
72= SuperCirce unvertraeglich mit einigen Maerchenbedingungen
73= Stellungen: %s, Trefferrate:
74= Chamaeleonschach und Chamaeleonsteine nicht gleichzeitig erlaubt
75= Pruefe als
76= -Zueger.
77= Forderung wird nicht unterstuetzt.\n
78= Maerchenfiguren in Beweispartien nicht erlaubt.\n
79= Es koennen maximal zwei Felder fuer ep-Schluessel eingegeben werden.\n
80= Abbruch durch Signal %d nach %s\n
81= transmutierende/reflektierende koenigliche Steine nicht erlaubt.\n
82= Drehung/Spiegelung nicht erlaubt: 
83= \nEin Stein wird ueber den Brettrand verschoben.
84= Ferner muss als erstes nach Zwilling angegeben werden.\n
85= Widerlegt.\n
86= FrischAufCirce: Stein ist keine Umwandlungsfigur, gg. Steine zuerst eingeben.\n
87= Manche Maerchenbedingungen in Beweispartien nicht erlaubt.\n
88= Isardam und Madrasi/Eiffelschach nicht erlaubt
89= Ohneschach oder ExklusivSchach: kann nicht entscheiden, ob ein Zug legal ist.\n
90= ist bereits besetzt - alter Stein wird geloescht.\n
91= MarsCirce, PlusSchach oder PhantomSchach unvereinbar mit verschiedenen Maerchenbedingungen und -steinen.\n
92= moegliche Stellungen in
93= Unsinnige Kombination
94= VogtlanderSchach nicht erlaubt in Verbindung mit Isardam oder BrunnerSchach\n
95= Circe Assassin unvereinbar mit verschiedenen Maerchenbedingungen und -steinen\n
96= Rueckkehr- und Platzwechsel-Forderungen sind unvereinbar mit manchen Maerchenbedingungen\n
97= Nur Mattforderung im RepublikanerSchach erlaubt\n
98= , erhoehe HashRateLevel auf %s\n
99= , vermindere HashRateLevel auf %s\n
100= Intelligent Modus nur im h#/=, ser-#/= und ser-h#/=, wenigen Maerchenbedingungen und keinen Maerchensteinen.\n
101= Feld leer - kann keine Figur entfernen/versetzen.\n
102= Option  MaxZeit  nicht unterstuetzt.\n
103= Option  StopNachKurzloesungen  nicht unterstuetzt im direkten Spiel.\n
104= Maerchensteine nicht erlaubt in Single box
105= LaTeXdatei nicht uebersetzbar: Maerchenstein nicht spezifiziert.\n
106= Mehr als %s verschiedene Jaegertypen\n
107= Unvereinbare koenigliche Felder und/oder Steine\n
108= Schlagschach unvereinbar mit Schach- oder Mattforderungen\n
109= Take&MakeSchach unvereinbar mit verschiedenen Maerchenbedingungen\n
110= Magische Steine unvereinbar mit verschiedenen Maerchenfiguren\n
111= Zu viele magische Berechnungen benoetigt\n
112= Republikanerschach nicht mit Bedingungen, die davon abhangen, ob gewisse Zuege Schach bieten.\n
113= Es ist nicht feststellbar, wer am Zug ist\n
114= Option \"Drohung\" und exact-Forderung unvereinbar\n
115= Geisterschach und Spukschach unvereinbar mit Circe, Anticirce, Kamikaze, Haaner Schach\n
116= WeissBeginnt nicht anwendbar - ignoriert\n
117= Satzspiel nicht anwendbar - ignoriert\n
118= NachSchluessel nicht anwendbar - ignoriert\n
119= Mehrere Ziele nicht erlaubt, wenn eines davon dia oder a=>b ist\n
