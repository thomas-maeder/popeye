#include "output/plaintext/language_dependant.h"

#include "debugging/assert.h"
#include <stdlib.h>
#include <string.h>

PieTable PieNamString[LanguageCount] =
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
    /*153*/ {'e','p'}    /* epine */
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
    /*153*/ {'s','t'}    /* Stachel */
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
    /*153*/ {'s','t'}    /* sting */
  }
};

/* later set according to language */
PieceChar *PieceTab = PieNamString[German];

char const * const *OptTab;

char const * const OptString[LanguageCount][OptCount] =
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

char const * const CondString[LanguageCount][CondCount] =
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
    /*192*/ "DernierePrise"
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
    /*192*/ "LetzterSchlag"
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
    /*192*/ "LastCapture"
  }
};

char const * const *ColourTab;

char const * const ColourString[LanguageCount][nr_colours] =
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

char const * const PieSpString[LanguageCount][nr_piece_flags-nr_sides] =
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

char const * const VaultingVariantTypeString[LanguageCount][1] =
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

char const * const ConditionLetteredVariantTypeString[LanguageCount][ConditionLetteredVariantTypeCount] =
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

char const * const ConditionNumberedVariantTypeString[LanguageCount][ConditionNumberedVariantTypeCount] =
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

char const * const AntiCirceVariantTypeString[LanguageCount][anticirce_type_count] =
{
  {
    /* French */
     "Cheylan",
     "Calvet"
  },{
    /* German */
     "Cheylan",
     "Calvet"
  },{
    /* English */
     "Cheylan",
     "Calvet"
  }
};

char const * const *SentinellesVariantTypeTab;

char const * const SentinellesVariantTypeString[LanguageCount][SentinellesVariantCount] =
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

char const * const GridVariantTypeString[LanguageCount][GridVariantCount] =
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

char const * const KobulVariantTypeString[LanguageCount][KobulVariantCount] =
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

char const * const KoekoVariantTypeString[LanguageCount][1] =
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

char const * const CirceVariantTypeString[LanguageCount][CirceVariantCount] =
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

char const * const ExtraCondString[LanguageCount][ExtraCondCount] =
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

char const * const TwinningString[LanguageCount][TwinningCount] =
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

char const * const TwinningMirrorString[LanguageCount][TwinningMirrorCount] =
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

char const * const mummer_strictness_string[LanguageCount][nr_mummer_strictness] =
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

char const *MessageTabs[LanguageCount][MsgCount] =
{
  /*0*/ {
    /* 0*/  "erreur fatale:\n",
    /* 1*/  "il faut un roi de chaque couleur",
    /* 2*/  "Trop peu memoires centrales disponible",
    /* 3*/  "solution interrompue.",
    /* 4*/  "trop de solutions",
    /* 5*/  "en EchecsHaan les pieces kamikazees sont interdites",
    /* 6*/  "jeu d'essai pas applicable",
    /* 7*/  "trop d'echecs feeriques pour les pieces neutres",
    /* 8*/  "leofamily avec pieces orthodoxes ?",
    /* 9*/  "en circe les pieces impuissantes sont interdites",
    /*10*/  "CouleurEchangeantSautoir seulement avec quelques sauteurs",
    /*11*/  "cavalier + cavalier majeur interdit",
    /*12*/  "renaicirce hors echiquier ?",
    /*13*/  "jeu apparent avec roi en echec ?",
    /*14*/  "pieces kamikazees + neutres ou pieces kamikazees + circe + pieces feeriques interdites.",
    /*15*/  "zeroposition, mais pas de jumeau",
    /*16*/  "on ne peut pas melanger minimum/maximum/duelliste/alphabetique et priseforce pour le meme camp",
    /*17*/  "\nle camp au trait peut prendre le roi",
    /*18*/  "on ne peut pas melanger monochromatique et bichromatique",
    /*19*/  "charactere inconnue: `%s'",
    /*20*/  "plus d'un roi interdit",
    /*21*/  "liste des cases mal terminee - ignoree",
    /*22*/  "liste des cases inconnue ou il mangue une liste des cases - veuillez donner une",
    /*23*/  "numenroute hors limites: %s",
    /*24*/  "madrasi, eiffel, isardam, woozles, pieces paralysantes incompatibles avec certains genres feeriques",
    /*25*/  "en rexinclusive, on ne peut pas melanger circe et immunschach",
    /*26*/  "certaines pieces feeriques incompatibles avec heffalumps",
    /*27*/  "enonce avec trops des coups",
    /*28*/  "pas des pieces royales en circe echange ou rex inclusiv.",
    /*29*/  "message inconnue %s",
    /*30*/  "element d'offenser: %s",
    /*31*/  "erreur d'entree:",
    /*32*/  "piece inconnue ou il manque une piece - ignoree",
    /*33*/  "specification de piece pas unique",
    /*34*/  "Echecs Annan et conditions qui changent la coleur de pieces interdit",
    /*35*/  "condition inconnue",
    /*36*/  "option pas unique - ignoree",
    /*37*/  "numero incorrecte",
    /*38*/  "option inconnue - ignoree",
    /*39*/  "specification de commande pas unique - ignoree",
    /*40*/  "commande non connue",
    /*41*/  "pas de specification de l'enonce",
    /*42*/  "Ne peut pas ouvrir fichier de sortie",
    /*43*/  "Ne peut pas ouvrir fichier d'entree",
    /*44*/  "Un trop grand nombre ont emboite des niveaux d'entree",
    /*45*/  "Aucune couleur n'a indique - toute l'entree de cahier des charges ignoree",
    /*46*/  "enonce inconnue",
    /*47*/  "condition pas unique - ignoree",
    /*48*/  "fin de fichier avant fin de probleme",
    /*49*/  "Debordement de ligne d'entree",
    /*50*/  "pas debut probleme",
    /*51*/  "Erreur interne de popeye dans le commutateur de tk",
    /*52*/  "solution terminee.",
    /*53*/  " blocus.",
    /*54*/  " menace:",
    /*55*/  "mais",
    /*56*/  "Temps = ",
    /*57*/  "on mate en un coup par\n",
    /*58*/  "\n",
    /*59*/  "pas de pieces ou conditions trop feeriques avec imitator",
    /*60*/  "trop d'imitators",
    /*61*/  "roi kamikaze seulement en circe rex inclusiv",
    /*62*/  "Gain_de_Piece et Circe Parrain interdit",
    /*63*/  "probleme ignoree",
    /*64*/  "anticirce incompatible avec certains genres feeriques",
    /*65*/  "echecs einstein incompatible avec des pieces feeriques",
    /*66*/  "supercirce incompatible avec certains genres feeriques",
    /*67*/  "entree dans 'hashtable': %s, hit-rate:",
    /*68*/  "On ne peut pas melanger EchecsChameleon et des pieces chameleon",
    /*69*/  "Calculation de",
    /*70*/  "-coups.",
    /*71*/  "enonce inconnue.\n",
    /*72*/  "Multiple buts pas acceptables si l'un d'eux est dia ou a=>b\n",
    /*73*/  "plus de 2 cases pour cle en passant interdit.\n",
    /*74*/  "Termination par signal %d apres ",
    /*75*/  "pieces royales transmutees/reflectees interdites.\n",
    /*76*/  "Rotation/Miroir impossible.",
    /*77*/  "\nUne piece est deplacee hors de l'echiquier.",
    /*78*/  "Enplus doit immediatement suivre jumeau.\n",
    /*79*/  "Refute.",
    /*80*/  "FrischAufCirce: piece n'est pas une piece promue.\n",
    /*81*/  "certaines conditions feeriques et plus courte jeux justificative interdit.\n",
    /*82*/  "Isardam et Madrasi/Eiffel interdit",
    /*83*/  " on ne peut pas determiner si ce coup est legal.\n",
    /*84*/  " deja occupee - piece ancienne va dans l'au-dela.\n",
    /*85*/  "MarsCirce, EchecsPlus ou EchecsPhantom incompatible avec certains genres feeriques et avec des pieces feeriques.\n",
    /*86*/  "%lu positions possibles dans %u+%u",
    /*87*/  "Melange interdit",
    /*88*/  "On ne peut pas melanger EchecsVogtlander avec Isardam ou EchecsBrunner\n",
    /*89*/  "CirceAssassin est incompatible avec certains genres feeriques et avec des pieces feeriques.\n",
    /*90*/  "enonce doit etre mat avec condition EchecsRepublicain\n",
    /*91*/  ", increment HashRateLevel a %s\n",
    /*92*/  ", decroissance HashRateLevel a %s\n",
    /*93*/  "mode intelligent seulement avec h#/=, ser-#/= et ser-h#/=, facultativement avec certaines conditions, et sans pieces feeriques.\n",
    /*94*/  ": la case est vide - on ne peut pas oter ou deplacer une piece.\n",
    /*95*/  "L'option  MaxTemps  n'est pas soutenu.\n",
    /*96*/  "L'option  FinApresSolutionsCourtes  n'est pas soutenu au jeu direct.\n",
    /*97*/  "Les pieces feeriques ne sont pas permises dans SingleBox",
    /*98*/  "erreur dans FichierLaTeX : piece feerique inconnue\n",
    /*99*/  "Plus de %s types de chasseurs differents\n",
    /*100*/ "Cases et/ou pieces royales incompatibles\n",
    /*101*/ "QuiPerdGagne incompatible avec les enonces echec ou mat\n",
    /*102*/ "Take&MakeEchecs ne sont pas compatibles avec certains genres feeriques\n",
    /*103*/ "Pieces magiques ne sont pas compatibles avec certaines pieces feeriques\n",
    /*104*/ "Trop de calculations magiques necessaires\n",
    /*105*/ "EchecsRepublicains incompatible avec des conditions qui dependent du fait si un coup donne echec.\n",
    /*106*/ "Popeye ne peut pas decider qui a le trait.\n",
    /*107*/ "Option \"menace\" pas applicable ou longueur maximale de la menace trop grande\n",
    /*108*/ "EchecsGhost et EchecsHantes incompatibles avec Circe, Anticirce, Kamikaze, Echecs de Haan\n",
    /*109*/ "ApparentSeul pas applicable - ignore\n",
    /*110*/ "Apparent pas applicable - ignore\n",
    /*111*/ "ApresCle pas applicable - ignore\n",
    /*112*/ "Quodlibet pas applicable - ignore\n",
    /*113*/ "ButEstFin pas applicable - ignore\n",
    /*114*/ "ButEstFin et Quodlibet incompatibles\n",
    /*115*/ "Il ne peut pas etre decide si une condition s'applique aux Blancs ou aux Noirs\n",
    /*116*/ "ImmunCage seulement avec CirceCage\n",
    /*117*/ "On ne peut pas decider si ce coup est refute\n",
    /*118*/ "Capture de roi detectee - elements feeriques probablement incompatibles\n"
  },
  /*1*/ {
    /* 0*/  "Fataler Fehler:\n",
    /* 1*/  "Es fehlt ein weisser oder schwarzer Koenig\n",
    /* 2*/  "Zu wenig Hauptspeicher verfuegbar\n",
    /* 3*/  "Bearbeitung abgebrochen.",
    /* 4*/  "Zuviele Loesungen",
    /* 5*/  "Im HaanerSchach sind Kamikaze-Steine nicht erlaubt",
    /* 6*/  "Verfuehrung nicht anwendbar\n",
    /* 7*/  "Zu viel Maerchenschach fuer neutrale Steine",
    /* 8*/  "Leofamily mit orthodoxen Figuren nicht erlaubt",
    /* 9*/  "Dummy-Figuren bei Circe nicht erlaubt.",
    /*10*/  "SprungbockFarbeWechselnd nur bei einigen Huepfern erlaubt",
    /*11*/  "Springer und Cavalier Majeur nicht erlaubt",
    /*12*/  "Circe-Wiedergeburt ausserhalb des Brettes ?",
    /*13*/  "Satzspiel mit Koenig im Schach ?",
    /*14*/  "Kamikaze + Neutrale Steine oder Kamikaze + Circe + Maerchenfiguren nicht gleichzeitig erlaubt",
    /*15*/  "Null-Stellung, aber kein Zwilling",
    /*16*/  "Laengst-/Kuerzestzueger/Schlagzwang/Duellantenschach/AlphabetischesSchach gleichzeitig fuer die gleiche Partei ?",
    /*17*/  "\nDie am Zug befindliche Partei kann den Koenig schlagen",
    /*18*/  "Monochromes und Bichromes Schach gleichzeitig ergibt keinen Sinn",
    /*19*/  "Unzulaessiges Zeichen: `%s'",
    /*20*/  "Nur ein Koenig erlaubt",
    /*21*/  "Falsch beendete Feldliste - ignoriert",
    /*22*/  "Falsche oder fehlende Feldliste - bitte eine eingeben",
    /*23*/  "StartZugnummer ausserhalb der Grenzen: %s",
    /*24*/  "Eiffelschach, Madrasi, Isardam, Woozles oder paralysierende Steine unvereinbar mit manchen Maerchenbedingungen",
    /*25*/  "Circe und Immunschach und RexInklusive nicht erlaubt",
    /*26*/  "Manche Maerchenfiguren mit Heffalumps unvereinbar",
    /*27*/  "Zuviele Zuege in der Forderung",
    /*28*/  "Keine koeniglichen Figuren im PlatzwechselCirce oder RexInklusive\n",
    /*29*/  "Undefinierter Fehler %s",
    /*30*/  "Stoerende Eingabe: %s",
    /*31*/  "Eingabe-Fehler:",
    /*32*/  "Falsche oder fehlende Steinbezeichnung - wird ignoriert",
    /*33*/  "Steinbezeichnung nicht eindeutig",
    /*34*/  "AnnanSchach und Steine umfaerbende Bedingungen nicht erlaubt",
    /*35*/  "Bedingung nicht erkannt",
    /*36*/  "Option nicht eindeutig - ignoriert",
    /*37*/  "Keine oder falsche Zahlenangabe",
    /*38*/  "Option nicht erkannt - Einlesen von Optionen beendet",
    /*39*/  "Kommando nicht eindeutig - wird ignoriert",
    /*40*/  "Kommando unbekannt",
    /*41*/  "Keine Forderung angegeben",
    /*42*/  "Datei kann nicht zum Schreiben geoeffnet werden",
    /*43*/  "Datei kann nicht zum Lesen geoeffnet werden",
    /*44*/  "Zu viele geschachtelte Eingabedateien",
    /*45*/  "Keine Farbe angegeben - alle Spezifikationen ignoriert",
    /*46*/  "Forderung nicht bekannt",
    /*47*/  "Bedingung nicht eindeutig - ignoriert",
    /*48*/  "Ende der Eingabe vor EndeProblem",
    /*49*/  "Eingabezeilen Ueberlauf",
    /*50*/  "Kein AnfangProblem",
    /*51*/  "Interner Popeye Fehler in tk switch",
    /*52*/  "Loesung beendet.",
    /*53*/  " Zugzwang.",
    /*54*/  " Drohung:",
    /*55*/  "Aber",
    /*56*/  "Zeit = ",
    /*57*/  "Matt in einem Zug durch\n",
    /*58*/  "\n",
    /*59*/  "Zuviel Maerchenschach fuer Imitator",
    /*60*/  "Zuviele Imitatoren",
    /*61*/  "Kamikaze-Koenige nur in Circe RexInklusive",
    /*62*/  "Steingewinn Aufgaben mit Circe Parrain sind Frevel",
    /*63*/  "Problem uebersprungen",
    /*64*/  "AntiCirce mit einigen anderen Bedingungen nicht erlaubt",
    /*65*/  "EinsteinSchach nicht mit Maerchenfiguren erlaubt",
    /*66*/  "SuperCirce unvertraeglich mit einigen Maerchenbedingungen",
    /*67*/  "Stellungen: %s, Trefferrate:",
    /*68*/  "Chamaeleonschach und Chamaeleonsteine nicht gleichzeitig erlaubt",
    /*69*/  "Pruefe als",
    /*70*/  "-Zueger.",
    /*71*/  "Forderung wird nicht unterstuetzt.\n",
    /*72*/  "Mehrere Ziele nicht erlaubt, wenn eines davon dia oder a=>b ist\n",
    /*73*/  "Es koennen maximal zwei Felder fuer ep-Schluessel eingegeben werden.\n",
    /*74*/  "Abbruch durch Signal %d nach ",
    /*75*/  "transmutierende/reflektierende koenigliche Steine nicht erlaubt.\n",
    /*76*/  "Drehung/Spiegelung nicht erlaubt: ",
    /*77*/  "\nEin Stein wird ueber den Brettrand verschoben.",
    /*78*/  "Ferner muss als erstes nach Zwilling angegeben werden.\n",
    /*79*/  "Widerlegt.",
    /*80*/  "FrischAufCirce: Stein ist keine Umwandlungsfigur, gg. Steine zuerst eingeben.\n",
    /*81*/  "Manche Maerchenbedingungen in Beweispartien nicht erlaubt.\n",
    /*82*/  "Isardam und Madrasi/Eiffelschach nicht erlaubt",
    /*83*/  " kann nicht entscheiden, ob dieser Zug legal ist.\n",
    /*84*/  " ist bereits besetzt - alter Stein geht in die Unterwelt.\n",
    /*85*/  "MarsCirce, PlusSchach oder PhantomSchach unvereinbar mit verschiedenen Maerchenbedingungen und -steinen.\n",
    /*86*/  "%lu moegliche Stellungen in %u+%u",
    /*87*/  "Unsinnige Kombination",
    /*88*/  "VogtlanderSchach nicht erlaubt in Verbindung mit Isardam oder BrunnerSchach\n",
    /*89*/  "Circe Assassin unvereinbar mit verschiedenen Maerchenbedingungen und -steinen\n",
    /*90*/  "Nur Mattforderung im RepublikanerSchach erlaubt\n",
    /*91*/  ", erhoehe HashRateLevel auf %s\n",
    /*92*/  ", vermindere HashRateLevel auf %s\n",
    /*93*/  "Intelligent Modus nur im h#/=, ser-#/= und ser-h#/=, wenigen Maerchenbedingungen und keinen Maerchensteinen.\n",
    /*94*/  ": Feld leer - kann keine Figur entfernen/versetzen.\n",
    /*95*/  "Option  MaxZeit  nicht unterstuetzt.\n",
    /*96*/  "Option  StopNachKurzloesungen  nicht unterstuetzt im direkten Spiel.\n",
    /*97*/  "Maerchensteine nicht erlaubt in Single box",
    /*98*/  "LaTeXdatei nicht uebersetzbar: Maerchenstein nicht spezifiziert.\n",
    /*99*/  "Mehr als %s verschiedene Jaegertypen\n",
    /*100*/ "Unvereinbare koenigliche Felder und/oder Steine\n",
    /*101*/ "Schlagschach unvereinbar mit Schach- oder Mattforderungen\n",
    /*102*/ "Take&MakeSchach unvereinbar mit verschiedenen Maerchenbedingungen\n",
    /*103*/ "Magische Steine unvereinbar mit verschiedenen Maerchenfiguren\n",
    /*104*/ "Zu viele magische Berechnungen benoetigt\n",
    /*105*/ "Republikanerschach nicht mit Bedingungen, die davon abhangen, ob gewisse Zuege Schach bieten.\n",
    /*106*/ "Es ist nicht feststellbar, wer am Zug ist\n",
    /*107*/ "Option \"Drohung\" nicht anwendbar order maximale Laenge der Drohung zu gross\n",
    /*108*/ "Geisterschach und Spukschach unvereinbar mit Circe, Anticirce, Kamikaze, Haaner Schach\n",
    /*109*/ "WeissBeginnt nicht anwendbar - ignoriert\n",
    /*110*/ "Satzspiel nicht anwendbar - ignoriert\n",
    /*111*/ "NachSchluessel nicht anwendbar - ignoriert\n",
    /*112*/ "Quodlibet nicht anwendbar - ignoriert\n",
    /*113*/ "ZielIstEnde nicht anwendbar - ignoriert\n",
    /*114*/ "ZielIstEnde und Quodlibet nicht kompatibel\n",
    /*115*/ "Es ist nicht entscheidbar, ob Bedingung fuer Weiss oder Schwarz gilt\n",
    /*116*/ "KaefigImmunSchach nur mit KaefigCirce\n",
    /*117*/ "Kann nicht entscheiden, ob dieser Zug widerlegt wird\n",
    /*118*/ "Schlag eines Koenigs detektiert - Maerchenelemente scheinen unvereinbar\n"
  },
  /*2*/ {
    /* 0*/  "fatal error:\n",
    /* 1*/  "both sides need a king",
    /* 2*/  "too less main memory available",
    /* 3*/  "solving interrupted.",
    /* 4*/  "too many solutions",
    /* 5*/  "haaner chess not allowed with kamikaze-pieces",
    /* 6*/  "try play not applicable\n",
    /* 7*/  "too much fairy chess for neutral pieces",
    /* 8*/  "leofamily with orthodox pieces ?",
    /* 9*/  "dummy-pieces not allowed with circe.",
    /*10*/  "HurdleColourChanging only allowed with certain hoppers",
    /*11*/  "knight + cavalier majeur not allowed",
    /*12*/  "circe rebirth outside the board ?",
    /*13*/  "set play with checked king ?",
    /*14*/  "kamikaze pieces + neutral pieces or kamikaze pieces + circe + fairy pieces not allowed",
    /*15*/  "Zero-Position without twin",
    /*16*/  "maxi-/minimummer/mustcapture/duellistchess/alphabeticchess at the same time for the same side not allowed",
    /*17*/  "\nthe side to play can capture the king",
    /*18*/  "monochrom + bichrom chess gives no sense",
    /*19*/  "wrong character: `%s'",
    /*20*/  "only one king allowed",
    /*21*/  "wrong terminated SquareList - ignored",
    /*22*/  "wrong or missing SquareList - please give one",
    /*23*/  "startmovenumber exceeds the limit: %s",
    /*24*/  "eiffel, madrasi, isardam, woozles or paralysing pieces incompatible with some fairy conditions",
    /*25*/  "circe and immunschach and rex inclusiv not allowed",
    /*26*/  "some fairy-pieces not compatible with heffalumps",
    /*27*/  "stipulation with too many moves",
    /*28*/  "no royal pieces with PWC or rex inclusive",
    /*29*/  "undefined Message %s",
    /*30*/  "offending item: %s",
    /*31*/  "input-error:",
    /*32*/  "wrong or missing PieceName - ignored",
    /*33*/  "PieceSpecification not uniq",
    /*34*/  "Annan Chess and conditions that recolour pieces not allowed",
    /*35*/  "could not recognize condition",
    /*36*/  "option not uniq - ignored",
    /*37*/  "wrong integer",
    /*38*/  "could not recognize option - parsing of options aborted",
    /*39*/  "command not uniq specified - ignored",
    /*40*/  "command not known",
    /*41*/  "no stipulation specified",
    /*42*/  "cannot open output-file",
    /*43*/  "cannot open input-file",
    /*44*/  "too many nested input levels",
    /*45*/  "no color specified - all specification input ignored",
    /*46*/  "cannot recognize the stipulation",
    /*47*/  "condition not uniq - ignored",
    /*48*/  "end of file before end of problem",
    /*49*/  "input line overflow",
    /*50*/  "no begin of problem",
    /*51*/  "internal popeye error in tk switch",
    /*52*/  "solution finished.",
    /*53*/  " zugzwang.",
    /*54*/  " threat:",
    /*55*/  "but",
    /*56*/  "Time = ",
    /*57*/  "mate in one move with\n",
    /*58*/  "\n",
    /*59*/  "conditions too fairy for imitator",
    /*60*/  "too many imitators",
    /*61*/  "kamikaze king only in Circe rex inclusiv",
    /*62*/  "Win_a_Piece and Circe Parrain not allowed",
    /*63*/  "problem ignored",
    /*64*/  "anticirce incompatible with some fairy conditions",
    /*65*/  "einsteinchess not allowed with fairy pieces",
    /*66*/  "supercirce incompatible with some fairy conditions",
    /*67*/  "positions: %s, hitrate:",
    /*68*/  "ChameleonChess and ChameleonPieces not allowed together",
    /*69*/  "Checking in",
    /*70*/  "_moves.",
    /*71*/  "Stipulation not supported.\n",
    /*72*/  "Multiple goals not allowed if one of them is dia or a=>b\n",
    /*73*/  "Not more than 2 squares can be putin for en passant keys.\n",
    /*74*/  "Abort by signal %d after ",
    /*75*/  "Transmuting/reflecting royal pieces not supported.\n",
    /*76*/  "This rotation/mirroring is not supported: ",
    /*77*/  "\nA piece will be moved outside.",
    /*78*/  "Continued must be given immediately after twin.\n",
    /*79*/  "refutes.",
    /*80*/  "FrischAufCirce: piece is not a promoted piece. Enter the pieces first.\n",
    /*81*/  "Some fairy conditions in proof games not allowed.\n",
    /*82*/  "Isardam and Madrasi/Eiffel not allowed",
    /*83*/  " cannot decide if this move is legal or not.\n",
    /*84*/  " already occupied - old piece goes to the underworld.\n",
    /*85*/  "MarsCirce, PlusChess or PhantomChess is incompatible with several fairy conditions and fairy pieces.\n",
    /*86*/  "%lu potential positions in %u+%u",
    /*87*/  "nonsense combination",
    /*88*/  "VogtlanderChess not allowed in combination with Isardam or BrunnerChess\n",
    /*89*/  "Circe Assassin is incompatible with several fairy conditions and fairy pieces\n",
    /*90*/  "Must stipulate mate with Republican Chess\n",
    /*91*/  ", increment HashRateLevel to %s\n",
    /*92*/  ", decrement HashRateLevel to %s\n",
    /*93*/  "Intelligent Mode only with h#/=, ser-#/= and ser-h#/=, with a limited set of fairy conditions and without fairy pieces.\n",
    /*94*/  ": square is empty - cannot (re)move any piece.\n",
    /*95*/  "Option  MaxTime  not supported.\n",
    /*96*/  "Option  StopOnShortSolutions  not supported with direct play.\n",
    /*97*/  "Fairy pieces not allowed in Single box",
    /*98*/  "LaTeXFile will not compile: undefined fairy piece\n",
    /*99*/  "Limit of %s different hunter types exceeded\n",
    /*100*/ "Incompatible royal pieces and/or squares\n",
    /*101*/ "LosingChess incompatible with check or mate stipulations\n",
    /*102*/ "Take&MakeChess is incompatible with some fairy conditions\n",
    /*103*/ "Magic pieces incompatible with some fairy pieces\n",
    /*104*/ "Too many magic calculations needed\n",
    /*105*/ "RepublicanChess incompatible with conditions that depend on whether a certain move delivers check.\n",
    /*106*/ "Can't decide which side is at the move\n",
    /*107*/ "Option \"Threat\" not applicable or indicated maximum threat length too big\n",
    /*108*/ "GhostChess and HauntedChess incompatible with Circe, Anticirce, Kamikaze, Haaner Chess\n",
    /*109*/ "WhiteToPlay not applicable - ignored\n",
    /*110*/ "SetPlay not applicable - ignored\n",
    /*111*/ "PostKeyPlay not applicable - ignored\n",
    /*112*/ "Quodlibet not applicable - ignored\n",
    /*113*/ "GoalIsEnd not applicable - ignored\n",
    /*114*/ "GoalIsEnd and Quodlibet are incompatible\n",
    /*115*/ "Can't decide whether condition applies to White or Black\n",
    /*116*/ "CageImmunChess only with CageCirce\n",
    /*117*/ "Can't decide whether this move is refuted\n",
    /*118*/ "King capture detected - fairy elements seem incompatible"
  }
};

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
