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
  4= Zuviele Loesungen
  5= Im HaanerSchach sind Kamikaze-Steine nicht erlaubt
  6= Verfuehrung nicht anwendbar\n
  7= Zu viel Maerchenschach fuer neutrale Steine
  8= Leofamily mit orthodoxen Figuren nicht erlaubt
  9= Dummy-Figuren bei Circe nicht erlaubt.
 10= SprungbockFarbeWechselnd nur bei einigen Huepfern erlaubt
 11= Springer und Cavalier Majeur nicht erlaubt
 12= Circe-Wiedergeburt ausserhalb des Brettes ?
 13= Satzspiel mit Koenig im Schach ?
 14= Kamikaze + Neutrale Steine oder Kamikaze + Circe + Maerchenfiguren nicht gleichzeitig erlaubt
 15= En-Passant-Schluessel mit Wurmloechern nicht unterstuetzt
 16= Laengst-/Kuerzestzueger/Schlagzwang/Duellantenschach/AlphabetischesSchach gleichzeitig fuer die gleiche Partei ?
 17= \nDie am Zug befindliche Partei kann den Koenig schlagen
 18= Monochromes und Bichromes Schach gleichzeitig ergibt keinen Sinn
 19= Unzulaessiges Zeichen: `%s'
 20= Nur ein Koenig erlaubt
 21= Falsch beendete Feldliste - ignoriert
 22= Falsche oder fehlende Feldliste - bitte eine eingeben
 23= StartZugnummer ausserhalb der Grenzen: %s
 24= Eiffelschach, Madrasi, Isardam, Woozles oder paralysierende Steine unvereinbar mit manchen Maerchenbedingungen
 25= Circe und Immunschach und RexInklusive nicht erlaubt
 26= schwarz
 27= weiss
 28= Manche Maerchenfiguren mit Heffalumps unvereinbar
 29= Zuviele Zuege in der Forderung
 30= Keine koeniglichen Figuren im PlatzwechselCirce oder RexInklusive\n
 31= Undefinierter Fehler %s
 32= Stoerende Eingabe: %s
 33= Eingabe-Fehler:
 34= Falsche oder fehlende Steinbezeichnung - wird ignoriert
 35= Steinbezeichnung nicht eindeutig
 36= AnnanSchach und Steine umfaerbende Bedingungen nicht erlaubt
 37= Bedingung nicht erkannt
 38= Option nicht eindeutig - ignoriert
 39= Keine oder falsche Zahlenangabe
 40= Option nicht erkannt - Einlesen von Optionen beendet
 41= Kommando nicht eindeutig - wird ignoriert
 42= Kommando unbekannt
 43= Keine Forderung angegeben
 44= Datei kann nicht zum Schreiben geoeffnet werden
 45= Datei kann nicht zum Lesen geoeffnet werden
 46= Zu viele geschachtelte Eingabedateien
 47= Keine Farbe angegeben - alle Spezifikationen ignoriert
 48= Forderung nicht bekannt
 49= Bedingung nicht eindeutig - ignoriert
 50= Ende der Eingabe vor EndeProblem
 51= Eingabezeilen Ueberlauf
 52= Kein AnfangProblem
 53= Interner Popeye Fehler in tk switch
 54= Loesung beendet.
 55= Zugzwang.
 56= Drohung:
 57= Aber
 58= Zeit = 
 59= Matt in einem Zug durch\n
 60= \n
 61= Zuviel Maerchenschach fuer Imitator
 62= Zuviele Imitatoren
 63= Kamikaze-Koenige nur in Circe RexInklusive
 64= Steingewinn Aufgaben mit Circe Parrain sind Frevel
 65= Problem uebersprungen
 66= AntiCirce mit einigen anderen Bedingungen nicht erlaubt
 67= EinsteinSchach nicht mit Maerchenfiguren erlaubt
 68= SuperCirce unvertraeglich mit einigen Maerchenbedingungen
 69= Stellungen: %s, Trefferrate:
 70= Chamaeleonschach und Chamaeleonsteine nicht gleichzeitig erlaubt
 71= Pruefe als
 72= -Zueger.
 73= Forderung wird nicht unterstuetzt.\n
 74= Mehrere Ziele nicht erlaubt, wenn eines davon dia oder a=>b ist\n
 75= Es koennen maximal zwei Felder fuer ep-Schluessel eingegeben werden.\n
 76= Abbruch durch Signal %d nach %s\n
 77= transmutierende/reflektierende koenigliche Steine nicht erlaubt.\n
 78= Drehung/Spiegelung nicht erlaubt: 
 79= \nEin Stein wird ueber den Brettrand verschoben.
 80= Ferner muss als erstes nach Zwilling angegeben werden.\n
 81= Widerlegt.
 82= FrischAufCirce: Stein ist keine Umwandlungsfigur, gg. Steine zuerst eingeben.\n
 83= Manche Maerchenbedingungen in Beweispartien nicht erlaubt.\n
 84= Isardam und Madrasi/Eiffelschach nicht erlaubt
 85= kann nicht entscheiden, ob dieser Zug legal ist.\n
 86= ist bereits besetzt - alter Stein geht in die Unterwelt.\n
 87= MarsCirce, PlusSchach oder PhantomSchach unvereinbar mit verschiedenen Maerchenbedingungen und -steinen.\n
 88= moegliche Stellungen in
 89= Unsinnige Kombination
 90= VogtlanderSchach nicht erlaubt in Verbindung mit Isardam oder BrunnerSchach\n
 91= Circe Assassin unvereinbar mit verschiedenen Maerchenbedingungen und -steinen\n
 92= Nur Mattforderung im RepublikanerSchach erlaubt\n
 93= , erhoehe HashRateLevel auf %s\n
 94= , vermindere HashRateLevel auf %s\n
 95= Intelligent Modus nur im h#/=, ser-#/= und ser-h#/=, wenigen Maerchenbedingungen und keinen Maerchensteinen.\n
 96= Feld leer - kann keine Figur entfernen/versetzen.\n
 97= Option  MaxZeit  nicht unterstuetzt.\n
 98= Option  StopNachKurzloesungen  nicht unterstuetzt im direkten Spiel.\n
 99= Maerchensteine nicht erlaubt in Single box
100= LaTeXdatei nicht uebersetzbar: Maerchenstein nicht spezifiziert.\n
101= Mehr als %s verschiedene Jaegertypen\n
102= Unvereinbare koenigliche Felder und/oder Steine\n
103= Schlagschach unvereinbar mit Schach- oder Mattforderungen\n
104= Take&MakeSchach unvereinbar mit verschiedenen Maerchenbedingungen\n
105= Magische Steine unvereinbar mit verschiedenen Maerchenfiguren\n
106= Zu viele magische Berechnungen benoetigt\n
107= Republikanerschach nicht mit Bedingungen, die davon abhangen, ob gewisse Zuege Schach bieten.\n
108= Es ist nicht feststellbar, wer am Zug ist\n
109= Option \"Drohung\" nicht anwendbar order maximale Laenge der Drohung zu gross\n
110= Geisterschach und Spukschach unvereinbar mit Circe, Anticirce, Kamikaze, Haaner Schach\n
111= WeissBeginnt nicht anwendbar - ignoriert\n
112= Satzspiel nicht anwendbar - ignoriert\n
113= NachSchluessel nicht anwendbar - ignoriert\n
114= Quodlibet nicht anwendbar - ignoriert\n
115= ZielIstEnde nicht anwendbar - ignoriert\n
116= ZielIstEnde und Quodlibet nicht kompatibel\n
117= Es ist nicht entscheidbar, ob Bedingung fuer Weiss oder Schwarz gilt\n
118= KaefigImmunSchach nur mit KaefigCirce\n
119= Kann nicht entscheiden, ob dieser Zug widerlegt wird\n
