#include "output/plaintext/language_dependant.h"

#include "debugging/assert.h"
#include <stdlib.h>
#include <string.h>

char const **GlobalTokenTab; /* set according to language */

char const *GlobalTokenString[LanguageCount][GlobalTokenCount] =
{
  { /* francais */
    /* 0*/  "DebutProbleme"
  },
  { /* Deutsch */
    /* 0*/  "AnfangProblem"
  },
  { /* english */
    /* 0*/  "beginproblem"
  }
};

char const **ProblemTokenTab; /* set according to language */

char const *ProblemTokenString[LanguageCount][ProblemTokenCount] =
{
  { /* francais */
    /* 0*/  "FinProbleme",
    /* 1*/  "asuivre"
  },
  { /* Deutsch */
    /* 0*/  "EndeProblem",
    /* 1*/  "WeiteresProblem"
  },
  { /* english */
    /* 0*/  "endproblem",
    /* 1*/  "nextproblem"
  }
};

char const **EndTwinTokenTab; /* set according to language */

char const *EndTwinTokenString[LanguageCount][EndTwinTokenCount] =
{
  { /* francais */
    /* 0*/  "jumeau",
    /* 1*/  "zeroposition"
  },
  { /* Deutsch */
    /* 0*/  "Zwilling",
    /* 1*/  "NullStellung"
  },
  { /* english */
    /* 0*/  "twin",
    /* 1*/  "zeroposition"
  }
};

char const **InitialTwinTokenTab; /* set according to language */

char const *InitialTwinTokenString[LanguageCount][InitialTwinTokenCount] =
{
  { /* francais */
    /* 0*/  "remarque",
    /* 1*/  "senonce",
    /* 2*/  "auteur",
    /* 3*/  "source",
    /* 4*/  "pieces",
    /* 5*/  "condition",
    /* 6*/  "option",
    /* 7*/  "enonce",
    /* 8*/  "protocol",
    /* 9*/  "\n",
    /*10*/  "titre",
    /*11*/  "LaTeX",
    /*12*/  "PiecesLaTeX",
    /*13*/  "prix",
    /*14*/  "PositionInitialePartie",
    /*15*/  "Forsyth"
  },
  { /* Deutsch */
    /* 0*/  "Bemerkung",
    /* 1*/  "sForderung",
    /* 2*/  "Autor",
    /* 3*/  "Quelle",
    /* 4*/  "Steine",
    /* 5*/  "Bedingung",
    /* 6*/  "Option",
    /* 7*/  "Forderung",
    /* 8*/  "Protokoll",
    /* 9*/  "\n",
    /*10*/  "Titel",
    /*11*/  "LaTeX",
    /*12*/  "LaTeXSteine",
    /*13*/  "Auszeichnung",
    /*14*/  "PartieAnfangsStellung",
    /*15*/  "Forsyth"
  },
  { /* english */
    /* 0*/  "remark",
    /* 1*/  "sstipulation",
    /* 2*/  "author",
    /* 3*/  "origin",
    /* 4*/  "pieces",
    /* 5*/  "condition",
    /* 6*/  "option",
    /* 7*/  "stipulation",
    /* 8*/  "protocol",
    /* 9*/  "\n",
    /*10*/  "title",
    /*11*/  "LaTeX",
    /*12*/  "LaTeXPieces",
    /*13*/  "award",
    /*14*/  "InitialGameArray",
    /*15*/  "Forsyth"
  }
};

typedef PieceChar PieTable[nr_piece_walks];

static PieTable PieNamString[LanguageCount] =
{
  { /* French PieNamString */
  /*  0*/ {'.',' '},  /* vide */
  /*  1*/ {' ',' '},  /* hors echiquier */
  /*  2*/ {'r',' '},  /* roi */
  /*  3*/ {'p',' '},  /* pion */
  /*  4*/ {'d',' '},  /* dame */
  /*  5*/ {'c',' '},  /* cavalier */
  /*  6*/ {'t',' '},  /* tour */
  /*  7*/ {'f',' '},  /* fou */
  /*  8*/ {'l','e'},  /* leo */
  /*  9*/ {'m','a'},  /* mao */
  /* 10*/ {'p','a'},  /* pao */
  /* 11*/ {'v','a'},  /* vao */
  /* 12*/ {'r','o'},  /* rose */
  /* 13*/ {'s',' '},  /* sauterelle */
  /* 14*/ {'n',' '},  /* noctambule */
  /* 15*/ {'z',' '},  /* zebre */
  /* 16*/ {'c','h'},  /* chameau */
  /* 17*/ {'g','i'},  /* girafe */
  /* 18*/ {'c','c'},  /* cavalier racine carree cinquante */
  /* 19*/ {'b','u'},  /* bucephale  (cheval d'Alexandre le Grand) */
  /* 20*/ {'v','i'},  /* vizir */
  /* 21*/ {'a','l'},  /* alfil */
  /* 22*/ {'f','e'},  /* fers */
  /* 23*/ {'d','a'},  /* dabbabba */
  /* 24*/ {'l','i'},  /* lion */
  /* 25*/ {'e','q'},  /* equisauteur (non-stop) */
  /* 26*/ {'l',' '},  /* locuste */
  /* 27*/ {'p','b'},  /* pion berolina */
  /* 28*/ {'a','m'},  /* amazone */
  /* 29*/ {'i','m'},  /* imperatrice */
  /* 30*/ {'p','r'},  /* princesse */
  /* 31*/ {'g',' '},  /* gnou */
  /* 32*/ {'a','n'},  /* antilope */
  /* 33*/ {'e','c'},  /* ecureuil */
  /* 34*/ {'v',' '},  /* varan */
  /* 35*/ {'d','r'},  /* dragon */
  /* 36*/ {'k','a'},  /* kangourou */
  /* 37*/ {'c','s'},  /* cavalier spirale */
  /* 38*/ {'u','u'},  /* UbiUbi */
  /* 39*/ {'h','a'},  /* hamster: sauterelle a 180 degre */
  /* 40*/ {'e',' '},  /* elan: sauterelle a 45 degre */
  /* 41*/ {'a','i'},  /* aigle: sauterelle a 90 degre */
  /* 42*/ {'m',' '},  /* moineaux: sauterelle a 135 degre */
  /* 43*/ {'a','r'},  /* archeveque */
  /* 44*/ {'f','r'},  /* fou rebondissant */
  /* 45*/ {'c','a'},  /* cardinal */
  /* 46*/ {'s','n'},  /* noctambule sauteur */
  /* 47*/ {'p','i'},  /* piece impuissant */
  /* 48*/ {'c','n'},  /* chameau noctambule */
  /* 49*/ {'z','n'},  /* zebre noctambule */
  /* 50*/ {'g','n'},  /* gnou noctambule */
  /* 51*/ {'s','c'},  /* chameau sauteur */
  /* 52*/ {'s','z'},  /* zebre sauteur */
  /* 53*/ {'s','g'},  /* gnou sauteur */
  /* 54*/ {'c','d'},  /* cavalier spirale diagonale */
  /* 55*/ {'c','r'},  /* cavalier rebondissant */
  /* 56*/ {'e','a'},  /* equisauteur anglais */
  /* 57*/ {'c','t'},  /* CAT (= cavalier de troie) */
  /* 58*/ {'b','s'},  /* berolina superpion */
  /* 59*/ {'s','p'},  /* superpion */
  /* 60*/ {'t','l'},  /* tour-lion */
  /* 61*/ {'f','l'},  /* fou-lion */
  /* 62*/ {'s','i'},  /* sirene */
  /* 63*/ {'t','r'},  /* triton */
  /* 64*/ {'n','e'},  /* nereide */
  /* 65*/ {'o',' '},  /* orphan */
  /* 66*/ {'e','h'},  /* "edgehog", "randschwein" */
  /* 67*/ {'m','o'},  /* moa */
  /* 68*/ {'t','c'},  /* tour/fou chasseur */
  /* 69*/ {'f','c'},  /* fou/tour chasseur */
  /* 70*/ {'a','o'},  /* noctambule mao */
  /* 71*/ {'o','a'},  /* noctambule moa */
  /* 72*/ {'s','t'},  /* sauterelle tour */
  /* 73*/ {'s','f'},  /* sauterelle fou */
  /* 74*/ {'r','e'},  /* roi des elfes */
  /* 75*/ {'b','t'},  /* boy-scout */
  /* 76*/ {'g','t'},  /* girl-scout */
  /* 77*/ {'s','k'},  /* skylla */
  /* 78*/ {'c','y'},  /* charybdis */
  /* 79*/ {'s','a'},  /* sauterelle contA */
  /* 80*/ {'r','l'},  /* rose lion */
  /* 81*/ {'r','s'},  /* rose sauteur */
  /* 82*/ {'o','k'},  /* okapi */
  /* 83*/ {'3','7'},  /* 3:7-cavalier */
  /* 84*/ {'s','2'},  /* sauterelle-2 */
  /* 85*/ {'s','3'},  /* sauterelle-3 */
  /* 86*/ {'1','6'},  /* 1:6-cavalier */
  /* 87*/ {'2','4'},  /* 2:4-cavalier */
  /* 88*/ {'3','5'},  /* 3:5-cavalier */
  /* 89*/ {'d','s'},  /* double sauterelle */
  /* 90*/ {'s','r'},  /* roi sauteur */
  /* 91*/ {'o','r'},  /* orix */
  /* 92*/ {'1','5'},  /* 1:5-cavalier */
  /* 93*/ {'2','5'},  /* 2:5-cavalier */
  /* 94*/ {'g','l'},  /* gral */
    /* 95*/ {'l','t'},  /* tour locuste */
    /* 96*/ {'l','f'},  /* fou locuste */
    /* 97*/ {'l','n'},  /* noctambule locuste */
    /* 98*/ {'v','s'},  /* vizir sauteur */
    /* 99*/ {'f','s'},  /* fers sauteur */
  /*100*/ {'b','i'},  /* bison */
  /*101*/ {'e','l'},  /* elephant */
  /*102*/ {'n','a'},  /* Nao */
  /*103*/ {'e','t'},  /* elan tour: sauterelle tour a 45 degre */
  /*104*/ {'a','t'},  /* aigle tour: sauterelle tour a 90 degre */
  /*105*/ {'m','t'},  /* moineaux tour: sauterelle tour a 135 degre */
  /*106*/ {'e','f'},  /* elan fou: sauterelle fou a 45 degre */
  /*107*/ {'a','f'},  /* aigle fou: sauterelle fou a 90 degre */
  /*108*/ {'m','f'},  /* moineaux fou: sauterelle fou a 135 degre */
  /*109*/ {'r','a'},  /* rao: rose chinois */
  /*110*/ {'s','o'},  /* scorpion: roi des elfes et sauterelle */
  /*111*/ {'m','g'},  /* marguerite */
  /*112*/ {'3','6'},  /* 3:6-cavalier */
    /*113*/ {'n','l'},  /* noctambule lion */
    /*114*/ {'m','l'},  /* noctambule mao lion */
    /*115*/ {'m','m'},  /* noctambule moa lion */
    /*116*/ {'a',' '},  /* ami */
    /*117*/ {'d','n'},  /* dauphin */
    /*118*/ {'l','a'},  /* lapin */
    /*119*/ {'b','o'},  /* bob */
    /*120*/ {'q','a'},  /* equi anglais */
    /*121*/ {'q','f'},  /* equi francais */
    /*122*/ {'q','q'},  /* querquisite */
    /*123*/ {'b','1'},  /* bouncer */
    /*124*/ {'b','2'},  /* tour-bouncer */
    /*125*/ {'b','3'},  /* fou-bouncer */
    /*126*/ {'p','c'},  /* pion chinois */
    /*127*/ {'c','l'},  /* cavalier radial */
    /*128*/ {'p','v'},  /* pion renverse */
    /*129*/ {'l','r'},  /*rose locuste */
    /*130*/ {'z','e'},  /*zebu */
    /*131*/ {'n','r'},  /*noctambule rebondissant */
    /*132*/ {'n','2'},  /*spiralspringer (2,0) */
    /*133*/ {'n','4'},  /*spiralspringer (4,0) */
    /*134*/ {'n','1'},  /*diagonalspiralspringer (1,1) */
    /*135*/ {'n','3'},  /*diagonalspiralspringer (3,3) */
    /*136*/ {'q','n'},  /*quintessence */
    /*137*/ {'d','t'},  /* double tour-sauterelle */
    /*138*/ {'d','f'},  /* double fou-sauterelle */
    /*139*/ {'n','o'},   /* orix (non-stop) */
    /*140*/ {'s','b'},   /* treehopper */
    /*141*/ {'s','e'},   /* leafhopper */
    /*142*/ {'s','m'},   /* greater treehopper */
    /*143*/ {'s','u'},   /* greater leafhopper */
    /*144*/ {'k','l'},   /* kangarou lion */
    /*145*/ {'k','o'},   /* kao */
    /*146*/ {'s','v'},   /* knighthopper */
    /*147*/ {'b','a'},   /* bateau */
    /*148*/ {'g','y'},   /* gryphon */
    /*149*/ {'c','m'},   /* cavalier marin */
    /*150*/ {'p','o'},   /* pos\'eidon */
    /*151*/ {'p','m'},   /* pion marin */
    /*152*/ {'b','m'},   /* bateau marin */
    /*153*/ {'e','p'},   /* epine */
    /*154*/ {'d','g'},   /* Senora */
    /*155*/ {'f','a'},   /* Faro */
    /*156*/ {'l','o'},   /* Loco */
    /*157*/ {'s','l'},   /* Saltador */
    /*158*/ {'a','s'},   /* MaoSauteur */
    /*159*/ {'o','s'}    /* MoaSauteur */
  },{ /* German PieNamString */
  /*  0*/ {'.',' '},  /* leer */
  /*  1*/ {' ',' '},  /* ausserhalb des Brettes */
  /*  2*/ {'k',' '},  /* Koenig     0,1 + 1,1 S */
  /*  3*/ {'b',' '},  /* Bauer */
  /*  4*/ {'d',' '},  /* Dame       0,1 + 1,1 R */
  /*  5*/ {'s',' '},  /* Springer   1,2 S */
  /*  6*/ {'t',' '},  /* Turm       0,1 R */
  /*  7*/ {'l',' '},  /* Laeufer    1,1 R */
  /*  8*/ {'l','e'},  /* Leo */
  /*  9*/ {'m','a'},  /* Mao */
  /* 10*/ {'p','a'},  /* Pao */
  /* 11*/ {'v','a'},  /* Vao */
  /* 12*/ {'r','o'},  /* Rose */
  /* 13*/ {'g',' '},  /* Grashuepfer 0,1 + 1,1 H */
  /* 14*/ {'n',' '},  /* Nachtreiter 1,2 R */
  /* 15*/ {'z',' '},  /* Zebra       2,3 S */
  /* 16*/ {'c','a'},  /* Kamel       1,3 S */
  /* 17*/ {'g','i'},  /* Giraffe     1,4 S */
  /* 18*/ {'w','f'},  /* Wurzel-50-Springer 5,5 + 1,7 S */
  /* 19*/ {'b','u'},  /* Wurzel-25-Springer 3,4 + 5,0 S  (Bukephalos: Pferd Alexanders des Grossen) */
  /* 20*/ {'w','e'},  /* Wesir */
  /* 21*/ {'a','l'},  /* Alfil */
  /* 22*/ {'f','e'},  /* Fers */
  /* 23*/ {'d','a'},  /* Dabbaba */
  /* 24*/ {'l','i'},  /* Lion */
  /* 25*/ {'n','e'},  /* Non-stop Equihopper */
  /* 26*/ {'h',' '},  /* Heuschrecke */
  /* 27*/ {'b','b'},  /* Berolina Bauer */
  /* 28*/ {'a','m'},  /* Amazone    (Dame + Springer) */
  /* 29*/ {'k','n'},  /* Kaiserin   (Turm + Springer) */
  /* 30*/ {'p','r'},  /* Prinzessin (Laeufer + Springer) */
  /* 31*/ {'g','n'},  /* Gnu        (Springer + Kamel) */
  /* 32*/ {'a','n'},  /* Antilope    3,4 S */
  /* 33*/ {'e','i'},  /* Eichhoernchen      (Springer + Alfil + Dabbabba) */
  /* 34*/ {'w','a'},  /* Waran      (Turm + Nachtreiter) */
  /* 35*/ {'d','r'},  /* Drache     (Springer + Bauer) */
  /* 36*/ {'k','a'},  /* Kaenguruh */
  /* 37*/ {'s','s'},  /* Spiralspringer */
  /* 38*/ {'u','u'},  /* UbiUbi */
  /* 39*/ {'h','a'},  /* Hamster: 180 Grad Grashuepfer */
  /* 40*/ {'e','l'},  /* Elch: 45 Grad Grashuepfer */
  /* 41*/ {'a','d'},  /* Adler: 90 Grad Grashuepfer */
  /* 42*/ {'s','p'},  /* Spatz: 135 Grad Grashuepfer */
  /* 43*/ {'e','r'},  /* Erzbischof */
  /* 44*/ {'r','l'},  /* Reflektierender Laeufer */
  /* 45*/ {'k','l'},  /* Kardinal */
  /* 46*/ {'n','h'},  /* Nachtreiterhuepfer */
  /* 47*/ {'d','u'},  /* Dummy */
  /* 48*/ {'c','r'},  /* Kamelreiter */
  /* 49*/ {'z','r'},  /* Zebrareiter */
  /* 50*/ {'g','r'},  /* Gnureiter */
  /* 51*/ {'c','h'},  /* Kamelreiterhuepfer */
  /* 52*/ {'z','h'},  /* Zebrareiterhuepfer */
  /* 53*/ {'g','h'},  /* Gnureiterhuepfer */
  /* 54*/ {'d','s'},  /* DiagonalSpiralSpringer */
  /* 55*/ {'r','s'},  /* Reflektierender Springer */
  /* 56*/ {'e','q'},  /* Equihopper */
  /* 57*/ {'c','t'},  /* CAT (= cavalier de troie) */
  /* 58*/ {'b','s'},  /* Berolina Superbauer */
  /* 59*/ {'s','b'},  /* Superbauer */
  /* 60*/ {'t','l'},  /* Turm-Lion */
  /* 61*/ {'l','l'},  /* Laeufer-Lion */
  /* 62*/ {'s','i'},  /* Sirene */
  /* 63*/ {'t','r'},  /* Triton */
  /* 64*/ {'n','d'},  /* Nereide */
  /* 65*/ {'o',' '},  /* Orphan */
  /* 66*/ {'r','d'},  /* Randschwein */
  /* 67*/ {'m','o'},  /* Moa */
  /* 68*/ {'t','j'},  /* Turm/Laeufer-Jaeger */
  /* 69*/ {'l','j'},  /* Laeufer/Turm-Jaeger */
  /* 70*/ {'a','o'},  /* Maoreiter */
  /* 71*/ {'o','a'},  /* Moareiter */
  /* 72*/ {'t','h'},  /* Turmhuepfer */
  /* 73*/ {'l','h'},  /* Laeuferhuepfer */
  /* 74*/ {'e','k'},  /* Erlkoenig */
  /* 75*/ {'p','f'},  /* Pfadfinder */
  /* 76*/ {'p','n'},  /* Pfadfinderin */
  /* 77*/ {'s','k'},  /* Skylla */
  /* 78*/ {'c','y'},  /* Charybdis */
  /* 79*/ {'k','g'},  /* KontraGrashuepfer */
  /* 80*/ {'r','n'},  /* RosenLion */
  /* 81*/ {'r','h'},  /* RosenHuepfer */
  /* 82*/ {'o','k'},  /* Okapi */
  /* 83*/ {'3','7'},  /* 3:7-Springer */
  /* 84*/ {'g','2'},  /* Grashuepfer-2 */
  /* 85*/ {'g','3'},  /* Grashuepfer-3 */
  /* 86*/ {'1','6'},  /* 1:6-Springer */
  /* 87*/ {'2','4'},  /* 2:4-Springer */
  /* 88*/ {'3','5'},  /* 3:5-Springer */
  /* 89*/ {'d','g'},  /* DoppelGrashuepfer */
  /* 90*/ {'k','h'},  /* Koenighuepfer */
  /* 91*/ {'o','r'},  /* Orix */
  /* 92*/ {'1','5'},  /* 1:5-Springer */
  /* 93*/ {'2','5'},  /* 2:5-Springer */
  /* 94*/ {'g','l'},  /* Gral */
    /* 95*/ {'h','t'},  /* Turmheuschrecke */
    /* 96*/ {'h','l'},  /* Laeuferheuschrecke */
    /* 97*/ {'h','n'},  /* Nachtreiterheuschrecke */
    /* 98*/ {'w','r'},  /* Wesirreiter */
    /* 99*/ {'f','r'},  /* Fersreiter */
  /*100*/ {'b','i'},  /* Bison */
  /*101*/ {'e','t'},  /* Elefant */
  /*102*/ {'n','a'},  /* Nao */
  /*103*/ {'t','c'},  /* Turmelch: 45 Grad Grashuepfer */
  /*104*/ {'t','a'},  /* Turmadler: 90 Grad Grashuepfer */
  /*105*/ {'t','s'},  /* Turmspatz: 135 Grad Grashuepfer */
  /*106*/ {'l','c'},  /* Laeuferelch: 45 Grad Grashuepfer */
  /*107*/ {'l','a'},  /* Laeuferadler: 90 Grad Grashuepfer */
  /*108*/ {'l','s'},  /* Laeuferspatz: 135 Grad Grashuepfer */
  /*109*/ {'r','a'},  /* Rao: Chinesische Rose */
  /*110*/ {'s','o'},  /* Skorpion: Erlkoenig + Grashuepfer */
  /*111*/ {'m','g'},  /* Marguerite */
  /*112*/ {'3','6'},  /* 3:6 Springer */
    /*113*/ {'n','l'},  /* Nachtreiterlion */
    /*114*/ {'m','l'},  /* Maoreiterlion */
    /*115*/ {'m','m'},  /* Moareiterlion */
    /*116*/ {'f',' '},  /* Freund */
    /*117*/ {'d','e'},  /* Delphin */
    /*118*/ {'h','e'},  /* Hase: Lion-Huepfer ueber 2 Boecke */
    /*119*/ {'b','o'},  /* Bob: Lion-Huepfer ueber 4 Boecke */
    /*120*/ {'q','e'},  /* EquiEnglisch */
    /*121*/ {'q','f'},  /* EquiFranzoesisch */
    /*122*/ {'o','d'},  /* Odysseus */
    /*123*/ {'b','1'},  /* Bouncer */
    /*124*/ {'b','2'},  /* Turm-bouncer */
    /*125*/ {'b','3'},  /* Laeufer-bouncer */
    /*126*/ {'c','b'},  /* Chinesischer Bauer */
    /*127*/ {'r','p'},  /* Radialspringer */
    /*128*/ {'r','b'},  /* ReversBauer */
    /*129*/ {'l','r'},  /* RosenHeuschrecke */
    /*130*/ {'z','e'},  /* Zebu */
    /*131*/ {'n','r'},  /* Reflektierender Nachreiter*/
    /*132*/ {'s','2'},  /* spiralspringer (2,0) */
    /*133*/ {'s','4'},  /* spiralspringer (4,0) */
    /*134*/ {'s','1'},  /* diagonalspiralspringer (1,1) */
    /*135*/ {'s','3'},  /* diagonalspiralspringer (3,3) */
    /*136*/ {'q','n'},   /* quintessence */
    /*137*/ {'d','t'},  /* Doppelturmhuepfer */
    /*138*/ {'d','l'},  /* Doppellaeuferhuepfer */
    /*139*/ {'n','o'},   /* orix (non-stop) */
    /*140*/ {'u','h'},   /* treehopper */
    /*141*/ {'b','h'},   /* leafhopper */
    /*142*/ {'g','u'},   /* greater treehopper */
    /*143*/ {'g','b'},  /* greater leafhopper */
    /*144*/ {'l','k'},   /* kangarou lion */
    /*145*/ {'k','o'},   /* kao */
    /*146*/ {'s','h'},   /* springerhuepfer */
    /*147*/ {'s','c'},   /* schiff */
    /*148*/ {'g','y'},   /* gryphon */
    /*149*/ {'m','s'},   /* mariner Springer */
    /*150*/ {'p','o'},   /* Poseidon */
    /*151*/ {'m','b'},   /* mariner Bauer */
    /*152*/ {'m','c'},   /* marines Schiff */
    /*153*/ {'s','t'},   /* Stachel */
    /*154*/ {'s','e'},   /* Senora */
    /*155*/ {'f','a'},   /* Faro */
    /*156*/ {'l','o'},   /* Loco */
    /*157*/ {'s','a'},   /* Saltador */
    /*158*/ {'a','h'},   /* MaoHopper */
    /*159*/ {'o','h'}    /* MoaHopper */
  },{/* English PieNamString */
  /*  0*/ {'.',' '},  /* empty */
  /*  1*/ {' ',' '},  /* outside board */
  /*  2*/ {'k',' '},  /* king */
  /*  3*/ {'p',' '},  /* pawn */
  /*  4*/ {'q',' '},  /* queen */
  /*  5*/ {'s',' '},  /* knight */
  /*  6*/ {'r',' '},  /* rook */
  /*  7*/ {'b',' '},  /* bishop */
  /*  8*/ {'l','e'},  /* leo */
  /*  9*/ {'m','a'},  /* mao */
  /* 10*/ {'p','a'},  /* pao */
  /* 11*/ {'v','a'},  /* vao */
  /* 12*/ {'r','o'},  /* rose */
  /* 13*/ {'g',' '},  /* grashopper */
  /* 14*/ {'n',' '},  /* nightrider */
  /* 15*/ {'z',' '},  /* zebra */
  /* 16*/ {'c','a'},  /* camel */
  /* 17*/ {'g','i'},  /* giraffe */
  /* 18*/ {'r','f'},  /* root-50-leaper */
  /* 19*/ {'b','u'},  /* root-25-leaper  (bucephale: horse of Alexander the Great) */
  /* 20*/ {'w','e'},  /* wazir */
  /* 21*/ {'a','l'},  /* alfil */
  /* 22*/ {'f','e'},  /* fers */
  /* 23*/ {'d','a'},  /* dabbabba */
  /* 24*/ {'l','i'},  /* lion */
  /* 25*/ {'n','e'},  /* nonstop equihopper */
  /* 26*/ {'l',' '},  /* locust */
  /* 27*/ {'b','p'},  /* berolina pawn */
  /* 28*/ {'a','m'},  /* amazon */
  /* 29*/ {'e','m'},  /* empress */
  /* 30*/ {'p','r'},  /* princess */
  /* 31*/ {'g','n'},  /* gnu */
  /* 32*/ {'a','n'},  /* antelope */
  /* 33*/ {'s','q'},  /* squirrel */
  /* 34*/ {'w','a'},  /* waran */
  /* 35*/ {'d','r'},  /* dragon */
  /* 36*/ {'k','a'},  /* kangaroo */
  /* 37*/ {'s','s'},  /* Spiralspringer */
  /* 38*/ {'u','u'},  /* UbiUbi */
  /* 39*/ {'h','a'},  /* hamster: 180 degree grashopper */
  /* 40*/ {'m',' '},  /* moose: 45 degree grashopper */
  /* 41*/ {'e','a'},  /* eagle: 90 degree grashopper */
  /* 42*/ {'s','w'},  /* sparrow: 135 degree grashopper */
  /* 43*/ {'a','r'},  /* archbishop */
  /* 44*/ {'r','b'},  /* reflecting bishop */
  /* 45*/ {'c',' '},  /* cardinal */
  /* 46*/ {'n','h'},  /* nightrider-hopper */
  /* 47*/ {'d','u'},  /* dummy */
  /* 48*/ {'c','r'},  /* camelrider */
  /* 49*/ {'z','r'},  /* zebrarider */
  /* 50*/ {'g','r'},  /* gnurider */
  /* 51*/ {'c','h'},  /* camelrider hopper */
  /* 52*/ {'z','h'},  /* zebrarider hopper */
  /* 53*/ {'g','h'},  /* gnurider hopper */
  /* 54*/ {'d','s'},  /* DiagonalSpiralSpringer */
  /* 55*/ {'b','k'},  /* bouncy knight */
  /* 56*/ {'e','q'},  /* equihoppper */
  /* 57*/ {'c','t'},  /* CAT  ( cavalier de troie) */
  /* 58*/ {'b','s'},  /* berolina superpawn */
  /* 59*/ {'s','p'},  /* superpawn */
  /* 60*/ {'r','l'},  /* rook-lion */
  /* 61*/ {'b','l'},  /* bishop-lion */
  /* 62*/ {'s','i'},  /* sirene */
  /* 63*/ {'t','r'},  /* triton */
  /* 64*/ {'n','d'},  /* nereide */
  /* 65*/ {'o',' '},  /* orphan */
  /* 66*/ {'e','h'},  /* edgehog */
  /* 67*/ {'m','o'},  /* moa */
  /* 68*/ {'r','r'},  /* rook/bishop-hunter */
  /* 69*/ {'b','r'},  /* bishop/rook-hunter */
  /* 70*/ {'a','o'},  /* maorider */
  /* 71*/ {'o','a'},  /* moarider */
  /* 72*/ {'r','h'},  /* rookhopper */
  /* 73*/ {'b','h'},  /* bishophopper */
  /* 74*/ {'e','k'},  /* erlking */
  /* 75*/ {'b','t'},  /* boyscout */
  /* 76*/ {'g','t'},  /* girlscout */
  /* 77*/ {'s','k'},  /* Skylla */
  /* 78*/ {'c','y'},  /* Charybdis */
  /* 79*/ {'c','g'},  /* contra grashopper */
  /* 80*/ {'r','n'},  /* RoseLion */
  /* 81*/ {'r','p'},  /* RoseHopper */
  /* 82*/ {'o','k'},  /* Okapi */
  /* 83*/ {'3','7'},  /* 3:7-leaper */
  /* 84*/ {'g','2'},  /* grasshopper-2 */
  /* 85*/ {'g','3'},  /* grasshopper-3 */
  /* 86*/ {'1','6'},  /* 1:6-leaper */
  /* 87*/ {'2','4'},  /* 2:4-leaper */
  /* 88*/ {'3','5'},  /* 3:5-leaper */
  /* 89*/ {'d','g'},  /* Double Grasshopper */
  /* 90*/ {'k','h'},  /* Kinghopper */
  /* 91*/ {'o','r'},  /* Orix */
  /* 92*/ {'1','5'},  /* 1:5-leaper */
  /* 93*/ {'2','5'},  /* 2:5-leaper */
  /* 94*/ {'g','l'},  /* Gral */
    /* 95*/ {'l','r'},  /* rook locust */
    /* 96*/ {'l','b'},  /* bishop locust */
    /* 97*/ {'l','n'},  /* nightrider locust */
    /* 98*/ {'w','r'},  /* wazirrider */
    /* 99*/ {'f','r'},  /* fersrider */
  /*100*/ {'b','i'},  /* bison */
  /*101*/ {'e','t'},  /* elephant */
  /*102*/ {'n','a'},  /* Nao */
  /*103*/ {'r','m'},  /* rook moose: 45 degree grashopper */
  /*104*/ {'r','e'},  /* rook eagle: 90 degree grashopper */
  /*105*/ {'r','w'},  /* rook sparrow: 135 degree grashopper */
  /*106*/ {'b','m'},  /* bishop moose: 45 degree grashopper */
  /*107*/ {'b','e'},  /* bishop eagle: 90 degree grashopper */
  /*108*/ {'b','w'},  /* bishop sparrow: 135 degree grashopper */
  /*109*/ {'r','a'},  /* rao: chinese rose */
  /*110*/ {'s','o'},  /* scorpion: erlking + grashopper */
  /*111*/ {'m','g'},  /* marguerite */
  /*112*/ {'3','6'},  /* 3:6 leaper */
    /*113*/ {'n','l'},  /* nightriderlion */
    /*114*/ {'m','l'},  /* maoriderlion */
    /*115*/ {'m','m'},  /* moariderlion */
    /*116*/ {'f',' '},  /* friend */
    /*117*/ {'d','o'},  /* dolphin */
    /*118*/ {'r','t'},  /* rabbit */
    /*119*/ {'b','o'},  /* bob */
    /*120*/ {'q','e'},  /* equi english */
    /*121*/ {'q','f'},  /* equi french */
    /*122*/ {'q','q'},  /* querquisite */
    /*123*/ {'b','1'},  /* bouncer */
    /*124*/ {'b','2'},  /* tour-bouncer */
    /*125*/ {'b','3'},  /* fou-bouncer */
    /*126*/ {'c','p'},  /* chinese pawn */
    /*127*/ {'r','k'},  /* radial knight */
    /*128*/ {'p','p'},  /* protean pawn */
    /*129*/ {'l','s'},  /* Rose Locust */
    /*130*/ {'z','e'},  /* zebu */
    /*131*/ {'b','n'},  /* Bouncy Nightrider */
    /*132*/ {'s','2'},  /* spiralspringer (2,0) */
    /*133*/ {'s','4'},  /* spiralspringer (4,0) */
    /*134*/ {'s','1'},  /* diagonalspiralspringer (1,1) */
    /*135*/ {'s','3'},  /* diagonalspiralspringer (3,3) */
    /*136*/ {'q','n'},  /* quintessence */
    /*137*/ {'d','k'},  /* double rookhopper */
    /*138*/ {'d','b'},  /* double bishopper */
    /*139*/ {'n','o'},   /* orix (non-stop) */
    /*140*/ {'t','h'},   /* treehopper */
    /*141*/ {'l','h'},   /* leafhopper */
    /*142*/ {'g','e'},   /* greater treehopper */
    /*143*/ {'g','f'},   /* greater leafhopper */
    /*144*/ {'k','l'},   /* kangarou lion */
    /*145*/ {'k','o'},   /* kao */
    /*146*/ {'k','p'},   /* knighthopper */
    /*147*/ {'s','h'},   /* ship */
    /*148*/ {'g','y'},   /* gryphon */
    /*149*/ {'m','s'},   /* marine knight */
    /*150*/ {'p','o'},   /* poseidon */
    /*151*/ {'m','p'},   /* marine pawn */
    /*152*/ {'m','h'},   /* marine ship */
    /*153*/ {'s','t'},   /* sting */
    /*154*/ {'s','e'},   /* Senora */
    /*155*/ {'f','a'},   /* Faro */
    /*156*/ {'l','o'},   /* Loco */
    /*157*/ {'s','a'},   /* Saltador */
    /*158*/ {'a','h'},   /* MaoHopper */
    /*159*/ {'o','h'}    /* MoaHopper */
  }
};

/* later set according to language */
PieceChar *PieceTab = PieNamString[German];

char const * const *OptTab;

static char const * const OptString[LanguageCount][OptCount] =
{
  {
  /* Francais French Franzoesisch */
  /* 0*/  "Defense",
  /* 1*/  "Apparent",
  /* 2*/  "ApparentSeul",
  /* 3*/  "Variantes",
  /* 4*/  "Trace",
  /* 5*/  "Enroute",
  /* 6*/  "SansRB",
  /* 7*/  "Duplex",
  /* 8*/  "SansRN",
  /* 9*/  "Menace",
  /*10*/  "SansMenace",
  /*11*/  "Essais",
  /*12*/  "MaxSolutions",
  /*13*/  "MaxCasesFuites",
  /*14*/  "Search",
  /*15*/  "MultiSolutions",
  /*16*/  "NonSymetrique",
  /*17*/  "NonTrivial",
  /*18*/  "RetiensPieceMatante",
  /*19*/  "EnPassant",
  /*20*/  "SansEchiquier",
  /*21*/  "SansVariantesCourtes",
  /*22*/  "DemiDuplex",
  /*23*/  "ApresCle",
  /*24*/  "Intelligent",
  /*25*/  "MaxTemps",
  /*26*/  "SansRoquer",
  /*27*/  "Quodlibet",
  /*28*/  "FinApresSolutionsCourtes",
  /*29*/  "Bip",
  /*30*/  "SansGrille",
  /*31*/  "AjouteGrille",
  /*32*/  "RoquesMutuellementExclusifs",
  /*33*/  "ButEstFin",
  /*34*/  "optionnonpubliee"
  },{
  /* Deutsch German Allemand */
  /* 0*/  "Widerlegung",
  /* 1*/  "Satzspiel",
  /* 2*/  "WeissBeginnt",
  /* 3*/  "Varianten",
  /* 4*/  "Zugnummern",
  /* 5*/  "StartZugnummer",
  /* 6*/  "OhneWk",
  /* 7*/  "Duplex",
  /* 8*/  "OhneSk",
  /* 9*/  "Drohung",
  /*10*/  "OhneDrohung",
  /*11*/  "Verfuehrung",
  /*12*/  "MaxLoesungen",
  /*13*/  "MaxFluchtFelder",
  /*14*/  "Suchen",
  /*15*/  "MehrereLoesungen",
  /*16*/  "KeineSymmetrie",
  /*17*/  "NichtTrivial",
  /*18*/  "HalteMattsetzendenStein",
  /*19*/  "EnPassant",
  /*20*/  "OhneBrett",
  /*21*/  "OhneKurzVarianten",
  /*22*/  "HalbDuplex",
  /*23*/  "NachSchluessel",
  /*24*/  "Intelligent",
  /*25*/  "MaxZeit",
  /*26*/  "KeineRochade",
  /*27*/  "Quodlibet",
  /*28*/  "StopNachKurzloesungen",
  /*29*/  "Pieps",
  /*30*/  "OhneGitter",
  /*31*/  "ZeichneGitter",
  /*32*/  "RochadenGegenseitigAusschliessend",
  /*33*/  "ZielIstEnde",
  /*34*/  "nichtpublizierteoption"
  },{
  /* English Anglais Englisch */
  /* 0*/  "Defence",
  /* 1*/  "SetPlay",
  /* 2*/  "WhiteToPlay",
  /* 3*/  "Variations",
  /* 4*/  "MoveNumbers",
  /* 5*/  "StartMoveNumber",
  /* 6*/  "NoWk",
  /* 7*/  "Duplex",
  /* 8*/  "NoBk",
  /* 9*/  "Threat",
  /*10*/  "NoThreat",
  /*11*/  "Try",
  /*12*/  "MaxSolutions",
  /*13*/  "MaxFlightsquares",
  /*14*/  "Search",
  /*15*/  "MultiSolutions",
  /*16*/  "NoSymmetry",
  /*17*/  "NonTrivial",
  /*18*/  "KeepMatingPiece",
  /*19*/  "EnPassant",
  /*20*/  "NoBoard",
  /*21*/  "NoShortVariations",
  /*22*/  "HalfDuplex",
  /*23*/  "PostKeyPlay",
  /*24*/  "Intelligent",
  /*25*/  "MaxTime",
  /*26*/  "NoCastling",
  /*27*/  "Quodlibet",
  /*28*/  "StopOnShortSolutions",
  /*29*/  "Beep",
  /*30*/  "SuppressGrid",
  /*31*/  "WriteGrid",
  /*32*/  "CastlingMutuallyExclusive",
  /*33*/  "GoalIsEnd",
  /*34*/  "unpublishedoption"
  }
};

char const * const *CondTab;

static char const * const CondString[LanguageCount][CondCount] =
{
  {
    /* French Condition Names */
    /* 0*/  "Circe",
    /* 1*/  "CirceMalefique",
    /* 2*/  "Madrasi",
    /* 3*/  "Volage",
    /* 4*/  "Hypervolage",
    /* 5*/  "Bichromatique",
    /* 6*/  "Monochromatique",
    /* 7*/  "Grille",
    /* 8*/  "Koeko",
    /* 9*/  "NoirBordCoup",
    /*10*/  "BlancBordCoup",
    /*11*/  "Leofamily",
    /*12*/  "Chinoises",
    /*13*/  "Patrouille",
    /*14*/  "CirceEchange",
    /*15*/  "SansPrises",
    /*16*/  "Immun",
    /*17*/  "ImmunMalefique",
    /*18*/  "ContactGrille",
    /*19*/  "Imitator",
    /*20*/  "CavalierMajeur",
    /*21*/  "Haan",
    /*22*/  "CirceCameleon",
    /*23*/  "CirceCouscous",
    /*24*/  "CirceEquipollents",
    /*25*/  "FileCirce",
    /*26*/  "NoirMaximum",
    /*27*/  "NoirMinimum",
    /*28*/  "BlancMaximum",
    /*29*/  "BlancMinimum",
    /*30*/  "CaseMagique",
    /*31*/  "Sentinelles",
    /*32*/  "Tibet",
    /*33*/  "DoubleTibet",
    /*34*/  "CirceDiagramme",
    /*35*/  "Trou",
    /*36*/  "NoirPriseForce",
    /*37*/  "BlancPriseForce",
    /*38*/  "RoisReflecteurs",
    /*39*/  "RoisTransmutes",
    /*40*/  "NoirPoursuite",
    /*41*/  "BlancPoursuite",
    /*42*/  "Duelliste",
    /*43*/  "CirceParrain",
    /*44*/  "SansIProm",
    /*45*/  "CirceSymetrique",
    /*46*/  "EchecsVogtlaender",
    /*47*/  "EchecsEinstein",
    /*48*/  "Bicolores",
    /*49*/  "KoekoNouveaux",
    /*50*/  "CirceClone",
    /*51*/  "AntiCirce",
    /*52*/  "FileCirceMalefique",
    /*53*/  "CirceAntipoden",
    /*54*/  "CirceCloneMalefique",
    /*55*/  "AntiCirceMalefique",
    /*56*/  "AntiCirceDiagramme",
    /*57*/  "AntiFileCirce",
    /*58*/  "AntiCirceSymetrique",
    /*59*/  "AntiFileCirceMalefique",
    /*60*/  "AntiCirceAntipoden",
    /*61*/  "AntiCirceEquipollents",
    /*62*/  "ImmunFile",
    /*63*/  "ImmunDiagramme",
    /*64*/  "ImmunFileMalefique",
    /*65*/  "ImmunSymmetrique",
    /*66*/  "ImmunAntipoden",
    /*67*/  "ImmunEquipollents",
    /*68*/  "ReversEchecsEinstein",
    /*69*/  "SuperCirce",
    /*70*/  "Degradierung",
    /*71*/  "NorskSjakk",
    /*72*/  "EchecsTraitor",
    /*73*/  "EchecsAndernach",
    /*74*/  "BlancCaseForce",
    /*75*/  "BlancCaseForceConsequent",
    /*76*/  "NoirCaseForce",
    /*77*/  "NoirCaseForceConsequent",
    /*78*/  "EchecsCameleon",
    /*79*/  "EchecsFonctionnaire",
    /*80*/  "EchecsGlasgow",
    /*81*/  "EchecsAntiAndernach",
    /*82*/  "FrischAufCirce",
    /*83*/  "CirceMalefiqueVerticale",
    /*84*/  "Isardam",
    /*85*/  "SansEchecs",
    /*86*/  "CirceDiametrale",
    /*87*/  "PromSeul",
    /*88*/  "RankCirce",
    /*89*/  "EchecsExclusif",
    /*90*/  "MarsCirce",
    /*91*/  "MarsCirceMalefique",
    /*92*/  "EchecsPhantom",
    /*93*/  "BlancRoiReflecteur",
    /*94*/  "NoirRoiReflecteur",
    /*95*/  "BlancRoiTransmute",
    /*96*/  "NoirRoiTransmute",
    /*97*/  "EchecsAntiEinstein",
    /*98*/  "CirceCouscousMalefique",
    /*99*/  "NoirCaseRoyal",
    /*100*/ "BlancCaseRoyal",
    /*101*/ "EchecsBrunner",
    /*102*/ "EchecsPlus",
    /*103*/ "CirceAssassin",
    /*104*/ "EchecsPatience",
    /*105*/ "EchecsRepublicains",
    /*106*/ "EchecsExtinction",
    /*107*/ "EchecsCentral",
    /*108*/ "ActuatedRevolvingBoard",
    /*109*/ "EchecsMessigny",
    /*110*/ "Woozles",
    /*111*/ "BiWoozles",
    /*112*/ "Heffalumps",
    /*113*/ "BiHeffalumps",
    /*114*/ "BlancCasePromotion",
    /*115*/ "NoirCasePromotion",
    /*116*/ "SansBlancPromotion",
    /*117*/ "SansNoirPromotion",
    /*118*/ "EchecsEiffel",
    /*119*/ "NoirUltraSchachZwang",
    /*120*/ "BlancUltraSchachZwang",
    /*121*/ "ActuatedRevolvingCentre",
    /*122*/ "ShieldedKings",
    /*123*/ "NONUTILISEACTUELLEMENT",
    /*124*/ "EchecsCameleonLigne",
    /*125*/ "BlancSansPrises",
    /*126*/ "NoirSansPrises",
    /*127*/ "EchecsAvril",
    /*128*/ "EchecsAlphabetiques",
    /*129*/ "CirceTurncoats",
    /*130*/ "CirceDoubleAgents",
    /*131*/ "AMU",
    /*132*/ "SingleBox",
    /*133*/ "MAFF",
    /*134*/ "OWU",
    /*135*/ "BlancRoisOscillant",
    /*136*/ "NoirRoisOscillant",
    /*137*/ "AntiRois",
    /*138*/ "AntiMarsCirce",
    /*139*/ "AntiMarsMalefiqueCirce",
    /*140*/ "AntiMarsAntipodeanCirce",
    /*141*/ "BlancSuperRoiTransmute",
    /*142*/ "NoirSuperRoiTransmute",
    /*143*/ "AntiSuperCirce",
    /*144*/ "UltraPatrouille",
    /*145*/ "RoisEchanges",
    /*146*/ "DynastieRoyale",
    /*147*/ "SAT",
    /*148*/ "StrictSAT",
    /*149*/ "Take&MakeEchecs",
    /*150*/ "NoirSynchronCoup",
    /*151*/ "BlancSynchronCoup",
    /*152*/ "NoirAntiSynchronCoup",
    /*153*/ "BlancAntiSynchronCoup",
    /*154*/ "Masand",
    /*155*/ "BGL" ,
    /*156*/ "NoirEchecs",
    /*157*/ "AnnanEchecs" ,
    /*158*/ "PionNormale",
    /*159*/ "Elliuortap",
    /*160*/ "VaultingKings",
    /*161*/ "BlancVaultingKing",
    /*162*/ "NoirVaultingKing",
    /*163*/ "EchecsProtee",
    /*164*/ "EchecsGeneve",
    /*165*/ "CameleonPoursuite",
    /*166*/ "AntiKoeko",
    /*167*/ "EchecsRoque",
    /*168*/ "QuiPerdGagne",
    /*169*/ "Disparate",
    /*170*/ "EchecsGhost",
    /*171*/ "EchecsHantes",
    /*172*/ "EchecsProvacateurs",
    /*173*/ "CirceCage",
    /*174*/ "Dummy",
    /*175*/ "Football",
    /*176*/ "ContraParrain",
    /*177*/ "RoisKobul",
    /*178*/ "EchangeRoque",
    /*179*/ "BlancEchecsAlphabetiques",
    /*180*/ "NoirEchecsAlphabetiques",
    /*181*/ "CirceTake&Make",
    /*182*/ "Supergardes",
    /*183*/ "TrousDeVer",
    /*184*/ "EchecsMarins",
    /*185*/ "EchecsUltramarins",
    /*186*/ "Retour",
    /*187*/ "FaceAFace",
    /*188*/ "DosADos",
    /*189*/ "JoueAJoue",
    /*190*/ "CameleonSequence",
    /*191*/ "AntiCloneCirce",
    /*192*/ "EchecsSnek",
    /*193*/ "EchecsSnekCirculaires",
    /*194*/ "DernierePrise",
    /*195*/ "EchecsArgentins",
    /*196*/ "PiecesPerdues",
    /*197*/ "ParalysePartielle",
    /*198*/ "CirceSymetriqueVerticale",
    /*199*/ "CirceSymetriqueHorizontale",
    /*200*/ "AntiCirceSymetriqueVerticale",
    /*201*/ "AntiCirceSymetriqueHorizontale",
    /*202*/ "ImmuneSymetriqueVerticale",
    /*203*/ "ImmuneSymetriqueHorizontale"
  },{
    /* German Condition Names */
    /* 0*/  "Circe",
    /* 1*/  "SpiegelCirce",
    /* 2*/  "Madrasi",
    /* 3*/  "Volage",
    /* 4*/  "Hypervolage",
    /* 5*/  "BichromesSchach",
    /* 6*/  "MonochromesSchach",
    /* 7*/  "Gitterschach",
    /* 8*/  "KoeKo",
    /* 9*/  "SchwarzerRandzueger",
    /*10*/  "WeisserRandzueger",
    /*11*/  "Leofamily",
    /*12*/  "ChinesischesSchach",
    /*13*/  "PatrouilleSchach",
    /*14*/  "PlatzwechselCirce",
    /*15*/  "Ohneschlag",
    /*16*/  "Immunschach",
    /*17*/  "SpiegelImmunschach",
    /*18*/  "KontaktGitter",
    /*19*/  "Imitator",
    /*20*/  "CavalierMajeur",
    /*21*/  "HaanerSchach",
    /*22*/  "ChamaeleonCirce",
    /*23*/  "CouscousCirce",
    /*24*/  "EquipollentsCirce",
    /*25*/  "FileCirce",
    /*26*/  "SchwarzerLaengstzueger",
    /*27*/  "SchwarzerKuerzestzueger",
    /*28*/  "WeisserLaengstzueger",
    /*29*/  "WeisserKuerzestzueger",
    /*30*/  "MagischeFelder",
    /*31*/  "Sentinelles",
    /*32*/  "TibetSchach",
    /*33*/  "DoppeltibetSchach",
    /*34*/  "DiagrammCirce",
    /*35*/  "Loch",
    /*36*/  "SchwarzerSchlagzwang",
    /*37*/  "WeisserSchlagzwang",
    /*38*/  "ReflektierendeKoenige",
    /*39*/  "TransmutierendeKoenige",
    /*40*/  "SchwarzerVerfolgungszueger",
    /*41*/  "WeisserVerfolgungszueger",
    /*42*/  "Duellantenschach",
    /*43*/  "CirceParrain",
    /*44*/  "OhneIUW",
    /*45*/  "SymmetrieCirce",
    /*46*/  "VogtlaenderSchach",
    /*47*/  "EinsteinSchach",
    /*48*/  "Bicolores",
    /*49*/  "NeuKoeko",
    /*50*/  "CirceClone",
    /*51*/  "AntiCirce",
    /*52*/  "SpiegelFileCirce",
    /*53*/  "AntipodenCirce",
    /*54*/  "SpiegelCirceclone",
    /*55*/  "AntiSpiegelCirce",
    /*56*/  "AntiDiagrammCirce",
    /*57*/  "AntiFileCirce",
    /*58*/  "AntiSymmetrieCirce",
    /*59*/  "AntiSpiegelFileCirce",
    /*60*/  "AntiAntipodenCirce",
    /*61*/  "AntiEquipollentsCirce",
    /*62*/  "FileImmunSchach",
    /*63*/  "DiagrammImmunSchach",
    /*64*/  "SpiegelFileImmunSchach",
    /*65*/  "SymmetrieImmunSchach",
    /*66*/  "AntipodenImmunSchach",
    /*67*/  "EquipollentsImmunSchach",
    /*68*/  "ReversesEinsteinSchach",
    /*69*/  "SuperCirce",
    /*70*/  "Degradierung",
    /*71*/  "NorskSjakk",
    /*72*/  "TraitorChess",
    /*73*/  "AndernachSchach",
    /*74*/  "WeissesZwangsfeld",
    /*75*/  "WeissesKonsequentesZwangsfeld",
    /*76*/  "SchwarzesZwangsfeld",
    /*77*/  "SchwarzesKonsequentesZwangsfeld",
    /*78*/  "ChamaeleonSchach",
    /*79*/  "BeamtenSchach",
    /*80*/  "GlasgowSchach",
    /*81*/  "AntiAndernachSchach",
    /*82*/  "FrischAufCirce",
    /*83*/  "VertikalesSpiegelCirce",
    /*84*/  "Isardam",
    /*85*/  "OhneSchach",
    /*86*/  "DiametralCirce",
    /*87*/  "UWnur",
    /*88*/  "RankCirce",
    /*89*/  "ExklusivSchach",
    /*90*/  "MarsCirce",
    /*91*/  "MarsSpiegelCirce",
    /*92*/  "PhantomSchach",
    /*93*/  "WeisserReflektierenderKoenig",
    /*94*/  "SchwarzerReflektierenderKoenig",
    /*95*/  "WeisserTransmutierenderKoenig",
    /*96*/  "SchwarzerTransmutierenderKoenig",
    /*97*/  "AntiEinsteinSchach",
    /*98*/  "SpiegelCouscousCirce",
    /*99*/  "SchwarzesKoeniglichesFeld",
    /*100*/ "WeissesKoeniglichesFeld",
    /*101*/ "BrunnerSchach",
    /*102*/ "PlusSchach",
    /*103*/ "AssassinCirce",
    /*104*/ "PatienceSchach",
    /*105*/ "RepublikanerSchach",
    /*106*/ "AusrottungsSchach",
    /*107*/ "ZentralSchach",
    /*108*/ "ActuatedRevolvingBoard",
    /*109*/ "MessignySchach",
    /*110*/ "Woozles",
    /*111*/ "BiWoozles",
    /*112*/ "Heffalumps",
    /*113*/ "BiHeffalumps",
    /*114*/ "UWFeldWeiss",
    /*115*/ "UWFeldSchwarz",
    /*116*/ "OhneWeissUW",
    /*117*/ "OhneSchwarzUW",
    /*118*/ "EiffelSchach",
    /*119*/ "SchwarzerUltraSchachZwang",
    /*120*/ "WeisserUltraSchachZwang",
    /*121*/ "ActuatedRevolvingCentre",
    /*122*/ "SchutzKoenige",
    /*123*/ "ZURZEITUNVERWENDET",
    /*124*/ "LinienChamaeleonSchach",
    /*125*/ "WeisserOhneSchlag",
    /*126*/ "SchwarzerOhneSchlag",
    /*127*/ "Aprilschach",
    /*128*/ "AlphabetischesSchach",
    /*129*/ "TurncoatCirce",
    /*130*/ "DoppelAgentenCirce",
    /*131*/ "AMU",
    /*132*/ "NurPartiesatzSteine",
    /*133*/ "MAFF",
    /*134*/ "OWU",
    /*135*/ "WeisseOszillierendeKoenige",
    /*136*/ "SchwarzeOszillierendeKoenige",
    /*137*/ "AntiKoenige",
    /*138*/ "AntiMarsCirce",
    /*139*/ "AntiMarsMalefiqueCirce",
    /*140*/ "AntiMarsAntipodeanCirce",
    /*141*/ "WeisserSuperTransmutierenderKoenig",
    /*142*/ "SchwarzerSuperTransmutierenderKoenig",
    /*143*/ "AntiSuperCirce",
    /*144*/ "UltraPatrouille",
    /*145*/ "TauschKoenige",
    /*146*/ "KoenigsDynastie",
    /*147*/ "SAT",
    /*148*/ "StrictSAT",
    /*149*/ "Take&MakeSchach",
    /*150*/ "SchwarzerSynchronZueger",
    /*151*/ "WeisserSynchronZueger",
    /*152*/ "SchwarzerAntiSynchronZueger",
    /*153*/ "WeisserAntiSynchronZueger",
    /*154*/ "Masand",
    /*155*/ "BGL",
    /*156*/ "SchwarzSchaecher",
    /*157*/ "Annanschach",
    /*158*/ "NormalBauern",
    /*159*/ "Elliuortap",
    /*160*/ "VaultingKings",
    /*161*/ "WeisserVaultingKing",
    /*162*/ "SchwarzerVaultingKing",
    /*163*/ "ProteischesSchach",
    /*164*/ "GenferSchach",
    /*165*/ "ChamaeleonVerfolgung",
    /*166*/ "AntiKoeko",
    /*167*/ "RochadeSchach",
    /*168*/ "Schlagschach",
    /*169*/ "Disparate",
    /*170*/ "Geisterschach",
    /*171*/ "Spukschach",
    /*172*/ "ProvokationSchach",
    /*173*/ "KaefigCirce",
    /*174*/ "Dummy",
    /*175*/ "Fussball",
    /*176*/ "KontraParrain",
    /*177*/ "KobulKoenige",
    /*178*/ "PlatzwechselRochade",
    /*179*/ "WeissesAlphabetischesSchach",
    /*180*/ "SchwarzesAlphabetischesSchach",
    /*181*/ "CirceTake&Make",
    /*182*/ "SuperDeckungen",
    /*183*/ "Wurmloecher",
    /*184*/ "MarinesSchach",
    /*185*/ "UltramarinesSchach",
    /*186*/ "NachHause",
    /*187*/ "AngesichtZuAngesicht",
    /*188*/ "RueckenAnRuecken",
    /*189*/ "WangeAnWange",
    /*190*/ "Chamaeleonsequenz",
    /*191*/ "AntiCloneCirce",
    /*192*/ "SnekSchach",
    /*193*/ "SnekZirkulaerSchach",
    /*194*/ "LetzterSchlag",
    /*195*/ "ArgentinischesSchach",
    /*196*/ "VerloreneSteine",
    /*197*/ "PartielleParalyse",
    /*198*/ "VertikaleSymmetrieCirce",
    /*199*/ "HorizontaleSymmetrieCirce",
    /*200*/ "VertikaleAntiSymmetrieCirce",
    /*201*/ "HorizontaleAntiSymmetrieCirce",
    /*202*/ "ImmuneVertikaleSymmetrie",
    /*203*/ "ImmuneHorizontaleSymmetrie"
  },{
    /* English Condition Names */
    /* 0*/  "Circe",
    /* 1*/  "MirrorCirce",
    /* 2*/  "Madrasi",
    /* 3*/  "Volage",
    /* 4*/  "Hypervolage",
    /* 5*/  "BichromChess",
    /* 6*/  "MonochromChess",
    /* 7*/  "GridChess",
    /* 8*/  "KoeKo",
    /* 9*/  "BlackEdgeMover",
    /*10*/  "WhiteEdgeMover",
    /*11*/  "Leofamily",
    /*12*/  "ChineseChess",
    /*13*/  "Patrouille",
    /*14*/  "PWC",
    /*15*/  "NoCapture",
    /*16*/  "ImmunChess",
    /*17*/  "MirrorImmunChess",
    /*18*/  "ContactGridChess",
    /*19*/  "Imitator",
    /*20*/  "CavalierMajeur",
    /*21*/  "HaanerChess",
    /*22*/  "ChameleonCirce",
    /*23*/  "CouscousCirce",
    /*24*/  "EquipollentsCirce",
    /*25*/  "FileCirce",
    /*26*/  "BlackMaximummer",
    /*27*/  "BlackMinimummer",
    /*28*/  "WhiteMaximummer",
    /*29*/  "WhiteMinimummer",
    /*30*/  "MagicSquares",
    /*31*/  "Sentinelles",
    /*32*/  "Tibet",
    /*33*/  "DoubleTibet",
    /*34*/  "DiagramCirce",
    /*35*/  "Hole",
    /*36*/  "BlackMustCapture",
    /*37*/  "WhiteMustCapture",
    /*38*/  "ReflectiveKings",
    /*39*/  "TransmutedKings",
    /*40*/  "BlackFollowMyLeader",
    /*41*/  "WhiteFollowMyLeader",
    /*42*/  "DuellistChess",
    /*43*/  "ParrainCirce",
    /*44*/  "NoIProm",
    /*45*/  "SymmetryCirce",
    /*46*/  "VogtlaenderChess",
    /*47*/  "EinsteinChess",
    /*48*/  "Bicolores",
    /*49*/  "NewKoeko",
    /*50*/  "CirceClone",
    /*51*/  "AntiCirce",
    /*52*/  "MirrorFileCirce",
    /*53*/  "AntipodeanCirce",
    /*54*/  "MirrorCirceClone",
    /*55*/  "MirrorAntiCirce",
    /*56*/  "DiagramAntiCirce",
    /*57*/  "FileAntiCirce",
    /*58*/  "SymmetryAntiCirce",
    /*59*/  "MirrorFileAntiCirce",
    /*60*/  "AntipodeanAntiCirce",
    /*61*/  "EquipollentsAntiCirce",
    /*62*/  "FileImmunChess",
    /*63*/  "DiagramImmunChess",
    /*64*/  "MirrorFileImmunChess",
    /*65*/  "SymmetryImmunChess",
    /*66*/  "AntipodeanImmunChess",
    /*67*/  "EquipollentsImmunChess",
    /*68*/  "ReversalEinsteinChess",
    /*69*/  "SuperCirce",
    /*70*/  "RelegationChess",
    /*71*/  "NorskSjakk",
    /*72*/  "TraitorChess",
    /*73*/  "AndernachChess",
    /*74*/  "WhiteForcedSquare",
    /*75*/  "WhiteConsequentForcedSquare",
    /*76*/  "BlackForcedSquare",
    /*77*/  "BlackConsequentForcedSquare",
    /*78*/  "ChameleonChess",
    /*79*/  "FunctionaryChess",
    /*80*/  "GlasgowChess",
    /*81*/  "AntiAndernachChess",
    /*82*/  "FrischAufCirce",
    /*83*/  "VerticalMirrorCirce",
    /*84*/  "Isardam",
    /*85*/  "ChecklessChess",
    /*86*/  "DiametralCirce",
    /*87*/  "PromOnly",
    /*88*/  "RankCirce",
    /*89*/  "ExclusiveChess",
    /*90*/  "MarsCirce",
    /*91*/  "MarsMirrorCirce",
    /*92*/  "PhantomChess",
    /*93*/  "WhiteReflectiveKing",
    /*94*/  "BlackReflectiveKing",
    /*95*/  "WhiteTransmutedKing",
    /*96*/  "BlackTransmutedKing",
    /*97*/  "AntiEinsteinChess",
    /*98*/  "MirrorCouscousCirce",
    /*99*/  "BlackRoyalSquare",
    /*100*/ "WhiteRoyalSquare",
    /*101*/ "BrunnerChess",
    /*102*/ "PlusChess",
    /*103*/ "CirceAssassin",
    /*104*/ "PatienceChess",
    /*105*/ "RepublicanChess",
    /*106*/ "ExtinctionChess",
    /*107*/ "CentralChess",
    /*108*/ "ActuatedRevolvingBoard",
    /*109*/ "MessignyChess",
    /*110*/ "Woozles",
    /*111*/ "BiWoozles",
    /*112*/ "Heffalumps",
    /*113*/ "BiHeffalumps",
    /*114*/ "WhitePromSquares",
    /*115*/ "BlackPromSquares",
    /*116*/ "NoWhitePromotion",
    /*117*/ "NoBlackPromotion",
    /*118*/ "EiffelChess",
    /*119*/ "BlackUltraSchachZwang",
    /*120*/ "WhiteUltraSchachZwang",
    /*121*/ "ActuatedRevolvingCentre",
    /*122*/ "ShieldedKings",
    /*123*/ "CURRENTLYUNUSED",
    /*124*/ "LineChameleonChess",
    /*125*/ "NoWhiteCapture",
    /*126*/ "NoBlackCapture",
    /*127*/ "AprilChess",
    /*128*/ "AlphabeticChess",
    /*129*/ "CirceTurncoats",
    /*130*/ "CirceDoubleAgents",
    /*131*/ "AMU",
    /*132*/ "SingleBox",
    /*133*/ "MAFF",
    /*134*/ "OWU",
    /*135*/ "WhiteOscillatingKings",
    /*136*/ "BlackOscillatingKings",
    /*137*/ "AntiKings",
    /*138*/ "AntiMarsCirce",
    /*139*/ "AntiMarsMirrorCirce",
    /*140*/ "AntiMarsAntipodeanCirce",
    /*141*/ "WhiteSuperTransmutingKing",
    /*142*/ "BlackSuperTransmutingKing",
    /*143*/ "AntiSuperCirce",
    /*144*/ "UltraPatrol",
    /*145*/ "SwappingKings",
    /*146*/ "RoyalDynasty",
    /*147*/ "SAT",
    /*148*/ "StrictSAT",
    /*149*/ "Take&MakeChess",
    /*150*/ "BlackSynchronMover",
    /*151*/ "WhiteSynchronMover",
    /*152*/ "BlackAntiSynchronMover",
    /*153*/ "WhiteAntiSynchronMover",
    /*154*/ "Masand",
    /*155*/ "BGL",
    /*156*/ "BlackChecks",
    /*157*/ "AnnanChess",
    /*158*/ "NormalPawn",
    /*159*/ "Lortap",
    /*160*/ "VaultingKings",
    /*161*/ "WhiteVaultingKing",
    /*162*/ "BlackVaultingKing",
    /*163*/ "ProteanChess",
    /*164*/ "GenevaChess",
    /*165*/ "ChameleonPursuit",
    /*166*/ "AntiKoeko",
    /*167*/ "CastlingChess",
    /*168*/ "LosingChess",
    /*169*/ "Disparate",
    /*170*/ "GhostChess",
    /*171*/ "HauntedChess",
    /*172*/ "ProvocationChess",
    /*173*/ "CageCirce",
    /*174*/ "Dummy",
    /*175*/ "Football",
    /*176*/ "ContraParrain",
    /*177*/ "KobulKings",
    /*178*/ "ExchangeCastling",
    /*179*/ "WhiteAlphabeticChess",
    /*180*/ "BlackAlphabeticChess",
    /*181*/ "CirceTake&Make",
    /*182*/ "SuperGuards",
    /*183*/ "Wormholes",
    /*184*/ "MarineChess",
    /*185*/ "UltramarineChess",
    /*186*/ "BackHome",
    /*187*/ "FaceToFace",
    /*188*/ "BackToBack",
    /*189*/ "CheekToCheek",
    /*190*/ "ChameleonSequence",
    /*191*/ "AntiCloneCirce",
    /*192*/ "SnekChess",
    /*193*/ "SnekCircleChess",
    /*194*/ "LastCapture",
    /*195*/ "ArgentinianChess",
    /*196*/ "LostPieces",
    /*197*/ "PartialParalysis",
    /*198*/ "VerticalSymmetryCirce",
    /*199*/ "HorizontalSymmetryCirce",
    /*200*/ "VerticalSymmetryAntiCirce",
    /*201*/ "HorizontalSymmetryAntiCirce",
    /*202*/ "ImmuneVerticalSymmetry",
    /*203*/ "ImmuneHorizontalSymmetry"
  }
};

char const * const *ColourTab;

static char const * const ColourString[LanguageCount][nr_colours] =
{
  {
     /* French */
    "Blanc",
    "Noir",
    "Neutre"
  },
  {
    /* German */
    "Weiss",
    "Schwarz",
    "Neutral"
  },
  {
    /* English */
    "White",
    "Black",
    "Neutral"
  }
};


char const * const *PieSpTab;

static char const * const PieSpString[LanguageCount][nr_piece_flags-nr_sides] =
{
  {
    /* French */
    "Royale",
    "Kamikaze",
    "Paralysante",
    "Cameleon",
    "Jigger",
    "Volage",
    "Fonctionnaire",
    "DemiNeutre",
    "CouleurEchangeantSautoir",
    "Protee",
    "Magique",
    "Imprenable",
    "Patrouille",
    "Frischauf"
  },
  {
    /* German */
    "Koeniglich",
    "Kamikaze",
    "Paralysierend",
    "Chamaeleon",
    "Jigger",
    "Volage",
    "Beamtet",
    "HalbNeutral",
    "SprungbockFarbeWechselnd",
    "Proteisch",
    "Magisch",
    "Unschlagbar",
    "Patrouille",
    "Frischauf"
  },
  {
    /* English */
    "Royal",
    "Kamikaze",
    "Paralysing",
    "Chameleon",
    "Jigger",
    "Volage",
    "Functionary",
    "HalfNeutral",
    "HurdleColourChanging",
    "Protean",
    "Magic",
    "Uncapturable",
    "Patrol",
    "Frischauf"
  }
};


char const * const *VaultingVariantTypeTab;

static char const * const VaultingVariantTypeString[LanguageCount][1] =
{
  {
    /* French */
    "Transmute"
  },{
    /* German */
    "Transmutierende"
  },{
    /* English */
    "Transmuting"
  }
};

char const * const *ConditionLetteredVariantTypeTab;

static char const * const ConditionLetteredVariantTypeString[LanguageCount][ConditionLetteredVariantTypeCount] =
{
  {
    /* French */
    "TypeA",
    "TypeB",
    "TypeC",
    "TypeD"
  },{
    /* German */
    "TypA",
    "TypB",
    "TypC",
    "TypD"
  },{
    /* English */
    "TypeA",
    "TypeB",
    "TypeC",
    "TypeD"
  }
};

char const * const *ConditionNumberedVariantTypeTab;

static char const * const ConditionNumberedVariantTypeString[LanguageCount][ConditionNumberedVariantTypeCount] =
{
  {
    /* French */
    "Type1",
    "Type2",
    "Type3"
  },{
    /* German */
    "Typ1",
    "Typ2",
    "Typ3"
  },{
    /* English */
    "Type1",
    "Type2",
    "Type3"
  }
};

char const * const *AntiCirceVariantTypeTab;

static char const * const AntiCirceVariantTypeString[LanguageCount][anticirce_type_count] =
{
  {
    /* French */
     "Calvet",
     "Cheylan"
  },{
    /* German */
     "Calvet",
     "Cheylan"
  },{
    /* English */
     "Calvet",
     "Cheylan"
  }
};

char const * const *SentinellesVariantTypeTab;

static char const * const SentinellesVariantTypeString[LanguageCount][SentinellesVariantCount] =
{
  {
    /* French */
     "PionPropre",
     "PionAdvers",
     "PionNeutre",
     "MaximumNoir",
     "MaximumBlanc",
     "MaximumTotal",
     "ParaSentinelles",
     "Berolina"
  },{
    /* German */
     "PionPropre",
     "PionAdvers",
     "PionNeutre",
     "MaximumSchwarz",
     "MaximumWeiss",
     "MaximumTotal",
     "ParaSentinelles",
     "Berolina"
  },{
    /* English */
     "PionPropre",
     "PionAdvers",
     "PionNeutre",
     "MaximumBlack",
     "MaximumWhite",
     "MaximumTotal",
     "ParaSentinelles",
     "Berolina"
  }
};

char const * const *GridVariantTypeTab;

static char const * const GridVariantTypeString[LanguageCount][GridVariantCount] =
{
  {
    /* French */
    "VerticalDecalage",
    "HorizontalDecalage",
    "DiagonalDecalage",
    "Orthogonale",
    "Irreguliere",
    "LigneGrille"
  },{
    /* German */
    "VertikalVerschieben",
    "HorizontalVerschieben",
    "DiagonalVerschieben",
    "Orthogonal",
    "Unregelmaessig",
    "GitterLinie"
  },{
    /* English */
    "VerticalShift",
    "HorizontalShift",
    "DiagonalShift",
    "Orthogonal",
    "Irregular",
    "GridLine"
  }
};

char const * const *KobulVariantTypeTab;

static char const * const KobulVariantTypeString[LanguageCount][KobulVariantCount] =
{
  {
    /* French */
    "Blancs",
    "Noirs"
  },{
    /* German */
    "Weiss",
    "Schwarz"
  },{
    /* English */
    "White",
    "Black"
  }
};

char const * const *KoekoVariantTypeTab;

static char const * const KoekoVariantTypeString[LanguageCount][1] =
{
  {
    /* French */
    "Voisin"
  },{
    /* German */
    "Nachbar"
  },{
    /* English */
    "Neighbour"
  }
};


char const * const *CirceVariantTypeTab;

static char const * const CirceVariantTypeString[LanguageCount][CirceVariantCount] =
{
  {
    /* French */
    "RexInclusif",
    "RexExclusif",
    "Malefique",
    "Assassin",
    "Diametral",
    "MalefiqueVerticale",
    "Clone",
    "Chameleon",
    "Turncoats",
    "Couscous",
    "CoupDernier",
    "Equipollents",
    "Parrain",
    "ContraParrain",
    "Cage",
    "Rangee",
    "Colonne",
    "Symetrie",
    "VerticaleSymetrique",
    "HorizontaleSymetrique",
    "Diagramme",
    "Echange",
    "Antipodes",
    "Take&Make",
    "Super",
    "Avril",
    "Frischauf",
    "Calvet",
    "Cheylan",
    "Stricte",
    "Relache",
    "Volcanique",
    "Parachute",
    "Einstein",
    "ReversEinstein"
  },{
    /* German */
    "RexInklusive",
    "RexExklusive",
    "Spiegel",
    "Assassin",
    "Diametral",
    "VertikalSpiegel",
    "Clone",
    "Chamaeleon",
    "Turncoat",
    "Couscous",
    "LetzterZug",
    "Equipollents",
    "Parrain",
    "ContraParrain",
    "Kaefig",
    "Reihe",
    "File",
    "Symmetrie",
    "VertikaleSymmetrie",
    "HorizontaleSymmetrie",
    "Diagramm",
    "Platzwechsel",
    "Antipoden",
    "Take&Make",
    "Super",
    "April",
    "Frischauf",
    "Calvet",
    "Cheylan",
    "Strikt",
    "Locker",
    "Vulkan",
    "Fallschirm",
    "Einstein",
    "ReversesEinstein"
  },{
    /* English */
    "RexInclusive",
    "RexExclusive",
    "Mirror",
    "Assassin",
    "Diametral",
    "VerticalMirror",
    "Clone",
    "Chameleon",
    "Turncoats",
    "Couscous",
    "LastMove",
    "Equipollents",
    "Parrain",
    "ContraParrain",
    "Cage",
    "Rank",
    "File",
    "Symmetry",
    "VerticalSymmetry",
    "HorizontalSymmetry",
    "Diagram",
    "PWC",
    "Antipodes",
    "Take&Make",
    "Super",
    "April",
    "Frischauf",
    "Calvet",
    "Cheylan",
    "Strict",
    "Relaxed",
    "Volcanic",
    "Parachute",
    "Einstein",
    "ReversalEinstein"
  }
};

char const * const *ExtraCondTab;

static char const * const ExtraCondString[LanguageCount][ExtraCondCount] =
{
  {
  /* French */
  /* 0*/  "Maximum",
  /* 1*/  "UltraSchachZwang"
  },{
  /* German */
  /* 0*/  "Laengstzueger",
  /* 1*/  "UltraSchachZwang"
  },{
  /* English */
  /* 0*/  "Maximummer",
  /* 1*/  "UltraSchachZwang"
  }
};

char const * const *TwinningTab;

static char const * const TwinningString[LanguageCount][TwinningCount] =
{
  { /* francais */
    /* 0*/  "deplacement",
    /* 1*/  "echange",
    /* 2*/  "enonce",
    /* 3*/  "senonce",
    /* 4*/  "ajoute",
    /* 5*/  "ote",
    /* 6*/  "enplus",
    /* 7*/  "rotation",
    /* 8*/  "condition",
    /* 9*/  "CouleurEchange",
    /*10*/  "miroir",
    /*11*/  "translation",
    /*12*/  "remplace"
  },
  { /* German */
    /* 0*/  "versetze",
    /* 1*/  "tausche",
    /* 2*/  "Forderung",
    /* 3*/  "SForderung",
    /* 4*/  "hinzufuegen",
    /* 5*/  "entferne",
    /* 6*/  "ferner",
    /* 7*/  "Drehung",
    /* 8*/  "Bedingung",
    /* 9*/  "Farbwechsel",
    /*10*/  "Spiegelung",
    /*11*/  "Verschiebung",
    /*12*/  "ersetze"
  },
  { /* English */
    /* 0*/  "move",
    /* 1*/  "exchange",
    /* 2*/  "stipulation",
    /* 3*/  "sstipulation",
    /* 4*/  "add",
    /* 5*/  "remove",
    /* 6*/  "continued",
    /* 7*/  "rotate",
    /* 8*/  "condition",
    /* 9*/  "PolishType",
    /*10*/  "mirror",
    /*11*/  "shift",
    /*12*/  "substitute"
  }
};

char const * const *TwinningMirrorTab;

static char const * const TwinningMirrorString[LanguageCount][TwinningMirrorCount] =
{
  { /* francais */
    "a1<-->h1",
    "a1<-->a8",
    "a1<-->h8",
    "a8<-->h1"
  },
  { /* German */
    "a1<-->h1",
    "a1<-->a8",
    "a1<-->h8",
    "a8<-->h1"
  },
  { /* English */
    "a1<-->h1",
    "a1<-->a8",
    "a1<-->h8",
    "a8<-->h1"
  }
};

char const * const *mummer_strictness_tab;

static char const * const mummer_strictness_string[LanguageCount][nr_mummer_strictness] =
{
  {
  /* French */
    "",
    "",
    "exact",
    "ultra"
  },{
  /* German */
      "",
      "",
      "exakt",
      "Ultra"
  },{
  /* English */
      "",
      "",
      "exact",
      "ultra"
  }
};

void output_plaintext_select_language(Language lang)
{
  ProblemTokenTab = &ProblemTokenString[lang][0];
  EndTwinTokenTab = &EndTwinTokenString[lang][0];
  InitialTwinTokenTab = &InitialTwinTokenString[lang][0];
  OptTab= &OptString[lang][0];
  CondTab= &CondString[lang][0];
  TwinningTab= &TwinningString[lang][0];
  TwinningMirrorTab= &TwinningMirrorString[lang][0];
  VaultingVariantTypeTab = &VaultingVariantTypeString[lang][0];
  ConditionLetteredVariantTypeTab = &ConditionLetteredVariantTypeString[lang][0];
  ConditionNumberedVariantTypeTab = &ConditionNumberedVariantTypeString[lang][0];
  AntiCirceVariantTypeTab = &AntiCirceVariantTypeString[lang][0];
  SentinellesVariantTypeTab = &SentinellesVariantTypeString[lang][0];
  GridVariantTypeTab = &GridVariantTypeString[lang][0];
  KobulVariantTypeTab = &KobulVariantTypeString[lang][0];
  KoekoVariantTypeTab = &KoekoVariantTypeString[lang][0];
  CirceVariantTypeTab = &CirceVariantTypeString[lang][0];
  ExtraCondTab= &ExtraCondString[lang][0];
  mummer_strictness_tab = &mummer_strictness_string[lang][0];
  PieceTab= PieNamString[lang];
  PieSpTab= PieSpString[lang];
  ColourTab= ColourString[lang];
}

static int comparePieceNames(void const * param1, void const * param2)
{
  PieceChar const *name1 = (PieceChar const *)param1;
  PieceChar const *name2 = (PieceChar const *)param2;
  int result = (*name1)[0]-(*name2)[0];
  if (result==0)
    result = (*name1)[1]-(*name2)[1];
  return result;
}

static void enforce_piecename_uniqueness_one_language(Language language)
{
  piece_walk_type name_index;
  PieTable piece_names_sorted;
  unsigned int nr_names = sizeof piece_names_sorted / sizeof piece_names_sorted[0];

  memcpy(piece_names_sorted, PieNamString[language], sizeof piece_names_sorted);
  qsort(piece_names_sorted,
        nr_names,
        sizeof piece_names_sorted[0],
        &comparePieceNames);

  /* hunter names are initialised to two blanks; qsort moves them to the
   * beginning */
  for (name_index = max_nr_hunter_walks; name_index<nr_names-1; ++name_index)
  {
    assert(piece_names_sorted[name_index][0]!=piece_names_sorted[name_index+1][0]
           || piece_names_sorted[name_index][1]!=piece_names_sorted[name_index+1][1]);
  }
}

/* Make sure that we don't define ambiguous piece name shortcuts */
void enforce_piecename_uniqueness(void)
{
  Language language;

  for (language = 0; language!=LanguageCount; ++language)
    enforce_piecename_uniqueness_one_language(language);
}
