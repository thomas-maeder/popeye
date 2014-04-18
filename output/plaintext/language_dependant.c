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
    /* 0*/  "RexInclusif",
    /* 1*/  "Circe",
    /* 2*/  "CirceMalefique",
    /* 3*/  "Madrasi",
    /* 4*/  "Volage",
    /* 5*/  "Hypervolage",
    /* 6*/  "Bichromatique",
    /* 7*/  "Monochromatique",
    /* 8*/  "Grille",
    /* 9*/  "Koeko",
    /*10*/  "NoirBordCoup",
    /*11*/  "BlancBordCoup",
    /*12*/  "Leofamily",
    /*13*/  "Chinoises",
    /*14*/  "Patrouille",
    /*15*/  "CirceEchange",
    /*16*/  "SansPrises",
    /*17*/  "Immun",
    /*18*/  "ImmunMalefique",
    /*19*/  "ContactGrille",
    /*20*/  "Imitator",
    /*21*/  "CavalierMajeur",
    /*22*/  "Haan",
    /*23*/  "CirceCameleon",
    /*24*/  "CirceCouscous",
    /*25*/  "CirceEquipollents",
    /*26*/  "FileCirce",
    /*27*/  "NoirMaximum",
    /*28*/  "NoirMinimum",
    /*29*/  "BlancMaximum",
    /*30*/  "BlancMinimum",
    /*31*/  "CaseMagique",
    /*32*/  "Sentinelles",
    /*33*/  "Tibet",
    /*34*/  "DoubleTibet",
    /*35*/  "CirceDiagramme",
    /*36*/  "Trou",
    /*37*/  "NoirPriseForce",
    /*38*/  "BlancPriseForce",
    /*39*/  "RoisReflecteurs",
    /*40*/  "RoisTransmutes",
    /*41*/  "NoirPoursuite",
    /*42*/  "BlancPoursuite",
    /*43*/  "Duelliste",
    /*44*/  "CirceParrain",
    /*45*/  "SansIProm",
    /*46*/  "CirceSymetrique",
    /*47*/  "EchecsVogtlaender",
    /*48*/  "EchecsEinstein",
    /*49*/  "Bicolores",
    /*50*/  "KoekoNouveaux",
    /*51*/  "CirceClone",
    /*52*/  "AntiCirce",
    /*53*/  "FileCirceMalefique",
    /*54*/  "CirceAntipoden",
    /*55*/  "CirceCloneMalefique",
    /*56*/  "AntiCirceMalefique",
    /*57*/  "AntiCirceDiagramme",
    /*58*/  "AntiFileCirce",
    /*59*/  "AntiCirceSymetrique",
    /*60*/  "AntiFileCirceMalefique",
    /*61*/  "AntiCirceAntipoden",
    /*62*/  "AntiCirceEquipollents",
    /*63*/  "ImmunFile",
    /*64*/  "ImmunDiagramme",
    /*65*/  "ImmunFileMalefique",
    /*66*/  "ImmunSymmetrique",
    /*67*/  "ImmunAntipoden",
    /*68*/  "ImmunEquipollents",
    /*69*/  "ReversEchecsEinstein",
    /*70*/  "SuperCirce",
    /*71*/  "Degradierung",
    /*72*/  "NorskSjakk",
    /*73*/  "EchecsTraitor",
    /*74*/  "EchecsAndernach",
    /*75*/  "BlancCaseForce",
    /*76*/  "BlancCaseForceConsequent",
    /*77*/  "NoirCaseForce",
    /*78*/  "NoirCaseForceConsequent",
    /*79*/  "EchecsCameleon",
    /*80*/  "EchecsFonctionnaire",
    /*81*/  "EchecsGlasgow",
    /*82*/  "EchecsAntiAndernach",
    /*83*/  "FrischAufCirce",
    /*84*/  "CirceMalefiqueVerticale",
    /*85*/  "Isardam",
    /*86*/  "SansEchecs",
    /*87*/  "CirceDiametrale",
    /*88*/  "PromSeul",
    /*89*/  "RankCirce",
    /*90*/  "EchecsExclusif",
    /*91*/  "MarsCirce",
    /*92*/  "MarsCirceMalefique",
    /*93*/  "EchecsPhantom",
    /*94*/  "BlancRoiReflecteur",
    /*95*/  "NoirRoiReflecteur",
    /*96*/  "BlancRoiTransmute",
    /*97*/  "NoirRoiTransmute",
    /*98*/  "EchecsAntiEinstein",
    /*99*/  "CirceCouscousMalefique",
    /*100*/ "NoirCaseRoyal",
    /*101*/ "BlancCaseRoyal",
    /*102*/ "EchecsBrunner",
    /*103*/ "EchecsPlus",
    /*104*/ "CirceAssassin",
    /*105*/ "EchecsPatience",
    /*106*/ "EchecsRepublicains",
    /*107*/ "EchecsExtinction",
    /*108*/ "EchecsCentral",
    /*109*/ "ActuatedRevolvingBoard",
    /*110*/ "EchecsMessigny",
    /*111*/ "Woozles",
    /*112*/ "BiWoozles",
    /*113*/ "Heffalumps",
    /*114*/ "BiHeffalumps",
    /*115*/ "RexExclusif",
    /*116*/ "BlancCasePromotion",
    /*117*/ "NoirCasePromotion",
    /*118*/ "SansBlancPromotion",
    /*119*/ "SansNoirPromotion",
    /*120*/ "EchecsEiffel",
    /*121*/ "NoirUltraSchachZwang",
    /*122*/ "BlancUltraSchachZwang",
    /*123*/ "ActuatedRevolvingCentre",
    /*124*/ "ShieldedKings",
    /*125*/ "NONUTILISEACTUELLEMENT",
    /*126*/ "EchecsCameleonLigne",
    /*127*/ "BlancSansPrises",
    /*128*/ "NoirSansPrises",
    /*129*/ "EchecsAvril",
    /*130*/ "EchecsAlphabetiques",
    /*131*/ "CirceTurncoats",
    /*132*/ "CirceDoubleAgents",
    /*133*/ "AMU",
    /*134*/ "SingleBox",
    /*135*/ "MAFF",
    /*136*/ "OWU",
    /*137*/ "BlancRoisOscillant",
    /*138*/ "NoirRoisOscillant",
    /*139*/ "AntiRois",
    /*140*/ "AntiMarsCirce",
    /*141*/ "AntiMarsMalefiqueCirce",
    /*142*/ "AntiMarsAntipodeanCirce",
    /*143*/ "BlancSuperRoiTransmute",
    /*144*/ "NoirSuperRoiTransmute",
    /*145*/ "AntiSuperCirce",
    /*146*/ "UltraPatrouille",
    /*147*/ "RoisEchanges",
    /*148*/ "DynastieRoyale",
    /*149*/ "SAT",
    /*150*/ "StrictSAT",
    /*151*/ "Take&MakeEchecs",
    /*152*/ "NoirSynchronCoup",
    /*153*/ "BlancSynchronCoup",
    /*154*/ "NoirAntiSynchronCoup",
    /*155*/ "BlancAntiSynchronCoup",
    /*156*/ "Masand",
    /*157*/ "BGL" ,
    /*158*/ "NoirEchecs",
    /*159*/ "AnnanEchecs" ,
    /*160*/ "PionNormale",
    /*161*/ "Elliuortap",
    /*162*/ "VaultingKings",
    /*163*/ "BlancVaultingKing",
    /*164*/ "NoirVaultingKing",
    /*165*/ "EchecsProtee",
    /*166*/ "EchecsGeneve",
    /*167*/ "CameleonPoursuite",
    /*168*/ "AntiKoeko",
    /*169*/ "EchecsRoque",
    /*170*/ "QuiPerdGagne",
    /*171*/ "Disparate",
    /*172*/ "EchecsGhost",
    /*173*/ "EchecsHantes",
    /*174*/ "EchecsProvacateurs",
    /*175*/ "CirceCage",
    /*176*/ "Dummy",
    /*177*/ "Football",
    /*178*/ "ContraParrain",
    /*179*/ "RoisKobul",
    /*180*/ "EchangeRoque",
    /*181*/ "BlancEchecsAlphabetiques",
    /*182*/ "NoirEchecsAlphabetiques",
    /*183*/ "CirceTake&Make",
    /*184*/ "Supergardes",
    /*185*/ "TrousDeVer",
    /*186*/ "EchecsMarins",
    /*187*/ "EchecsUltramarins",
    /*188*/ "Retour",
    /*189*/ "FaceAFace",
    /*190*/ "DosADos",
    /*191*/ "JoueAJoue",
    /*192*/ "CameleonSequence",
    /*193*/ "AntiCloneCirce",
    /*194*/ "DernierePrise"
  },{
    /* German Condition Names */
    /* 0*/  "RexInklusive",
    /* 1*/  "Circe",
    /* 2*/  "SpiegelCirce",
    /* 3*/  "Madrasi",
    /* 4*/  "Volage",
    /* 5*/  "Hypervolage",
    /* 6*/  "BichromesSchach",
    /* 7*/  "MonochromesSchach",
    /* 8*/  "Gitterschach",
    /* 9*/  "KoeKo",
    /*10*/  "SchwarzerRandzueger",
    /*11*/  "WeisserRandzueger",
    /*12*/  "Leofamily",
    /*13*/  "ChinesischesSchach",
    /*14*/  "PatrouilleSchach",
    /*15*/  "PlatzwechselCirce",
    /*16*/  "Ohneschlag",
    /*17*/  "Immunschach",
    /*18*/  "SpiegelImmunschach",
    /*19*/  "KontaktGitter",
    /*20*/  "Imitator",
    /*21*/  "CavalierMajeur",
    /*22*/  "HaanerSchach",
    /*23*/  "ChamaeleonCirce",
    /*24*/  "CouscousCirce",
    /*25*/  "EquipollentsCirce",
    /*26*/  "FileCirce",
    /*27*/  "SchwarzerLaengstzueger",
    /*28*/  "SchwarzerKuerzestzueger",
    /*29*/  "WeisserLaengstzueger",
    /*30*/  "WeisserKuerzestzueger",
    /*31*/  "MagischeFelder",
    /*32*/  "Sentinelles",
    /*33*/  "TibetSchach",
    /*34*/  "DoppeltibetSchach",
    /*35*/  "DiagrammCirce",
    /*36*/  "Loch",
    /*37*/  "SchwarzerSchlagzwang",
    /*38*/  "WeisserSchlagzwang",
    /*39*/  "ReflektierendeKoenige",
    /*40*/  "TransmutierendeKoenige",
    /*41*/  "SchwarzerVerfolgungszueger",
    /*42*/  "WeisserVerfolgungszueger",
    /*43*/  "Duellantenschach",
    /*44*/  "CirceParrain",
    /*45*/  "OhneIUW",
    /*46*/  "SymmetrieCirce",
    /*47*/  "VogtlaenderSchach",
    /*48*/  "EinsteinSchach",
    /*49*/  "Bicolores",
    /*50*/  "NeuKoeko",
    /*51*/  "CirceClone",
    /*52*/  "AntiCirce",
    /*53*/  "SpiegelFileCirce",
    /*54*/  "AntipodenCirce",
    /*55*/  "SpiegelCirceclone",
    /*56*/  "AntiSpiegelCirce",
    /*57*/  "AntiDiagrammCirce",
    /*58*/  "AntiFileCirce",
    /*59*/  "AntiSymmetrieCirce",
    /*60*/  "AntiSpiegelFileCirce",
    /*61*/  "AntiAntipodenCirce",
    /*62*/  "AntiEquipollentsCirce",
    /*63*/  "FileImmunSchach",
    /*64*/  "DiagrammImmunSchach",
    /*65*/  "SpiegelFileImmunSchach",
    /*66*/  "SymmetrieImmunSchach",
    /*67*/  "AntipodenImmunSchach",
    /*68*/  "EquipollentsImmunSchach",
    /*69*/  "ReversesEinsteinSchach",
    /*70*/  "SuperCirce",
    /*71*/  "Degradierung",
    /*72*/  "NorskSjakk",
    /*73*/  "TraitorChess",
    /*74*/  "AndernachSchach",
    /*75*/  "WeissesZwangsfeld",
    /*76*/  "WeissesKonsequentesZwangsfeld",
    /*77*/  "SchwarzesZwangsfeld",
    /*78*/  "SchwarzesKonsequentesZwangsfeld",
    /*79*/  "ChamaeleonSchach",
    /*80*/  "BeamtenSchach",
    /*81*/  "GlasgowSchach",
    /*82*/  "AntiAndernachSchach",
    /*83*/  "FrischAufCirce",
    /*84*/  "VertikalesSpiegelCirce",
    /*85*/  "Isardam",
    /*86*/  "OhneSchach",
    /*87*/  "DiametralCirce",
    /*88*/  "UWnur",
    /*89*/  "RankCirce",
    /*90*/  "ExklusivSchach",
    /*91*/  "MarsCirce",
    /*92*/  "MarsSpiegelCirce",
    /*93*/  "PhantomSchach",
    /*94*/  "WeisserReflektierenderKoenig",
    /*95*/  "SchwarzerReflektierenderKoenig",
    /*96*/  "WeisserTransmutierenderKoenig",
    /*97*/  "SchwarzerTransmutierenderKoenig",
    /*98*/  "AntiEinsteinSchach",
    /*99*/  "SpiegelCouscousCirce",
    /*100*/ "SchwarzesKoeniglichesFeld",
    /*101*/ "WeissesKoeniglichesFeld",
    /*102*/ "BrunnerSchach",
    /*103*/ "PlusSchach",
    /*104*/ "AssassinCirce",
    /*105*/ "PatienceSchach",
    /*106*/ "RepublikanerSchach",
    /*107*/ "AusrottungsSchach",
    /*108*/ "ZentralSchach",
    /*109*/ "ActuatedRevolvingBoard",
    /*110*/ "MessignySchach",
    /*111*/ "Woozles",
    /*112*/ "BiWoozles",
    /*113*/ "Heffalumps",
    /*114*/ "BiHeffalumps",
    /*115*/ "RexExklusive",
    /*116*/ "UWFeldWeiss",
    /*117*/ "UWFeldSchwarz",
    /*118*/ "OhneWeissUW",
    /*119*/ "OhneSchwarzUW",
    /*120*/ "EiffelSchach",
    /*121*/ "SchwarzerUltraSchachZwang",
    /*122*/ "WeisserUltraSchachZwang",
    /*123*/ "ActuatedRevolvingCentre",
    /*124*/ "SchutzKoenige",
    /*125*/ "ZURZEITUNVERWENDET",
    /*126*/ "LinienChamaeleonSchach",
    /*127*/ "WeisserOhneSchlag",
    /*128*/ "SchwarzerOhneSchlag",
    /*129*/ "Aprilschach",
    /*130*/ "AlphabetischesSchach",
    /*131*/ "TurncoatCirce",
    /*132*/ "DoppelAgentenCirce",
    /*133*/ "AMU",
    /*134*/ "NurPartiesatzSteine",
    /*135*/ "MAFF",
    /*136*/ "OWU",
    /*137*/ "WeisseOszillierendeKoenige",
    /*138*/ "SchwarzeOszillierendeKoenige",
    /*139*/ "AntiKoenige",
    /*140*/ "AntiMarsCirce",
    /*141*/ "AntiMarsMalefiqueCirce",
    /*142*/ "AntiMarsAntipodeanCirce",
    /*143*/ "WeisserSuperTransmutierenderKoenig",
    /*144*/ "SchwarzerSuperTransmutierenderKoenig",
    /*145*/ "AntiSuperCirce",
    /*146*/ "UltraPatrouille",
    /*147*/ "TauschKoenige",
    /*148*/ "KoenigsDynastie",
    /*149*/ "SAT",
    /*150*/ "StrictSAT",
    /*151*/ "Take&MakeSchach",
    /*152*/ "SchwarzerSynchronZueger",
    /*153*/ "WeisserSynchronZueger",
    /*154*/ "SchwarzerAntiSynchronZueger",
    /*155*/ "WeisserAntiSynchronZueger",
    /*156*/ "Masand",
    /*157*/ "BGL",
    /*158*/ "SchwarzSchaecher",
    /*159*/ "Annanschach",
    /*160*/ "NormalBauern",
    /*161*/ "Elliuortap",
    /*162*/ "VaultingKings",
    /*163*/ "WeisserVaultingKing",
    /*164*/ "SchwarzerVaultingKing",
    /*165*/ "ProteischesSchach",
    /*166*/ "GenferSchach",
    /*167*/ "ChamaeleonVerfolgung",
    /*168*/ "AntiKoeko",
    /*169*/ "RochadeSchach",
    /*170*/ "Schlagschach",
    /*171*/ "Disparate",
    /*172*/ "Geisterschach",
    /*173*/ "Spukschach",
    /*174*/ "ProvokationSchach",
    /*175*/ "KaefigCirce",
    /*176*/ "Dummy",
    /*177*/ "Fussball",
    /*178*/ "KontraParrain",
    /*179*/ "KobulKoenige",
    /*180*/ "PlatzwechselRochade",
    /*181*/ "WeissesAlphabetischesSchach",
    /*182*/ "SchwarzesAlphabetischesSchach",
    /*183*/ "CirceTake&Make",
    /*184*/ "SuperDeckungen",
    /*185*/ "Wurmloecher",
    /*186*/ "MarinesSchach",
    /*187*/ "UltramarinesSchach",
    /*188*/ "NachHause",
    /*189*/ "AngesichtZuAngesicht",
    /*190*/ "RueckenAnRuecken",
    /*191*/ "WangeAnWange",
    /*192*/ "Chamaeleonsequenz",
    /*193*/ "AntiCloneCirce",
    /*194*/ "LetzterSchlag"
  },{
    /* English Condition Names */
    /* 0*/  "RexInclusive",
    /* 1*/  "Circe",
    /* 2*/  "MirrorCirce",
    /* 3*/  "Madrasi",
    /* 4*/  "Volage",
    /* 5*/  "Hypervolage",
    /* 6*/  "BichromChess",
    /* 7*/  "MonochromChess",
    /* 8*/  "GridChess",
    /* 9*/  "KoeKo",
    /*10*/  "BlackEdgeMover",
    /*11*/  "WhiteEdgeMover",
    /*12*/  "Leofamily",
    /*13*/  "ChineseChess",
    /*14*/  "Patrouille",
    /*15*/  "PWC",
    /*16*/  "NoCapture",
    /*17*/  "ImmunChess",
    /*18*/  "MirrorImmunChess",
    /*19*/  "ContactGridChess",
    /*20*/  "Imitator",
    /*21*/  "CavalierMajeur",
    /*22*/  "HaanerChess",
    /*23*/  "ChameleonCirce",
    /*24*/  "CouscousCirce",
    /*25*/  "EquipollentsCirce",
    /*26*/  "FileCirce",
    /*27*/  "BlackMaximummer",
    /*28*/  "BlackMinimummer",
    /*29*/  "WhiteMaximummer",
    /*30*/  "WhiteMinimummer",
    /*31*/  "MagicSquares",
    /*32*/  "Sentinelles",
    /*33*/  "Tibet",
    /*34*/  "DoubleTibet",
    /*35*/  "DiagramCirce",
    /*36*/  "Hole",
    /*37*/  "BlackMustCapture",
    /*38*/  "WhiteMustCapture",
    /*39*/  "ReflectiveKings",
    /*40*/  "TransmutedKings",
    /*41*/  "BlackFollowMyLeader",
    /*42*/  "WhiteFollowMyLeader",
    /*43*/  "DuellistChess",
    /*44*/  "ParrainCirce",
    /*45*/  "NoIProm",
    /*46*/  "SymmetryCirce",
    /*47*/  "VogtlaenderChess",
    /*48*/  "EinsteinChess",
    /*49*/  "Bicolores",
    /*50*/  "NewKoeko",
    /*51*/  "CirceClone",
    /*52*/  "AntiCirce",
    /*53*/  "MirrorFileCirce",
    /*54*/  "AntipodeanCirce",
    /*55*/  "MirrorCirceClone",
    /*56*/  "MirrorAntiCirce",
    /*57*/  "DiagramAntiCirce",
    /*58*/  "FileAntiCirce",
    /*59*/  "SymmetryAntiCirce",
    /*60*/  "MirrorFileAntiCirce",
    /*61*/  "AntipodeanAntiCirce",
    /*62*/  "EquipollentsAntiCirce",
    /*63*/  "FileImmunChess",
    /*64*/  "DiagramImmunChess",
    /*65*/  "MirrorFileImmunChess",
    /*66*/  "SymmetryImmunChess",
    /*67*/  "AntipodeanImmunChess",
    /*68*/  "EquipollentsImmunChess",
    /*69*/  "ReversalEinsteinChess",
    /*70*/  "SuperCirce",
    /*71*/  "RelegationChess",
    /*72*/  "NorskSjakk",
    /*73*/  "TraitorChess",
    /*74*/  "AndernachChess",
    /*75*/  "WhiteForcedSquare",
    /*76*/  "WhiteConsequentForcedSquare",
    /*77*/  "BlackForcedSquare",
    /*78*/  "BlackConsequentForcedSquare",
    /*79*/  "ChameleonChess",
    /*80*/  "FunctionaryChess",
    /*81*/  "GlasgowChess",
    /*82*/  "AntiAndernachChess",
    /*83*/  "FrischAufCirce",
    /*84*/  "VerticalMirrorCirce",
    /*85*/  "Isardam",
    /*86*/  "ChecklessChess",
    /*87*/  "DiametralCirce",
    /*88*/  "PromOnly",
    /*89*/  "RankCirce",
    /*90*/  "ExclusiveChess",
    /*91*/  "MarsCirce",
    /*92*/  "MarsMirrorCirce",
    /*93*/  "PhantomChess",
    /*94*/  "WhiteReflectiveKing",
    /*95*/  "BlackReflectiveKing",
    /*96*/  "WhiteTransmutedKing",
    /*97*/  "BlackTransmutedKing",
    /*98*/  "AntiEinsteinChess",
    /*99*/  "MirrorCouscousCirce",
    /*100*/ "BlackRoyalSquare",
    /*101*/ "WhiteRoyalSquare",
    /*102*/ "BrunnerChess",
    /*103*/ "PlusChess",
    /*104*/ "CirceAssassin",
    /*105*/ "PatienceChess",
    /*106*/ "RepublicanChess",
    /*107*/ "ExtinctionChess",
    /*108*/ "CentralChess",
    /*109*/ "ActuatedRevolvingBoard",
    /*110*/ "MessignyChess",
    /*111*/ "Woozles",
    /*112*/ "BiWoozles",
    /*113*/ "Heffalumps",
    /*114*/ "BiHeffalumps",
    /*115*/ "RexExclusive",
    /*116*/ "WhitePromSquares",
    /*117*/ "BlackPromSquares",
    /*118*/ "NoWhitePromotion",
    /*119*/ "NoBlackPromotion",
    /*120*/ "EiffelChess",
    /*121*/ "BlackUltraSchachZwang",
    /*122*/ "WhiteUltraSchachZwang",
    /*123*/ "ActuatedRevolvingCentre",
    /*124*/ "ShieldedKings",
    /*125*/ "CURRENTLYUNUSED",
    /*126*/ "LineChameleonChess",
    /*127*/ "NoWhiteCapture",
    /*128*/ "NoBlackCapture",
    /*129*/ "AprilChess",
    /*130*/ "AlphabeticChess",
    /*131*/ "CirceTurncoats",
    /*132*/ "CirceDoubleAgents",
    /*133*/ "AMU",
    /*134*/ "SingleBox",
    /*135*/ "MAFF",
    /*136*/ "OWU",
    /*137*/ "WhiteOscillatingKings",
    /*138*/ "BlackOscillatingKings",
    /*139*/ "AntiKings",
    /*140*/ "AntiMarsCirce",
    /*141*/ "AntiMarsMirrorCirce",
    /*142*/ "AntiMarsAntipodeanCirce",
    /*143*/ "WhiteSuperTransmutingKing",
    /*144*/ "BlackSuperTransmutingKing",
    /*145*/ "AntiSuperCirce",
    /*146*/ "UltraPatrol",
    /*147*/ "SwappingKings",
    /*148*/ "RoyalDynasty",
    /*149*/ "SAT",
    /*150*/ "StrictSAT",
    /*151*/ "Take&MakeChess",
    /*152*/ "BlackSynchronMover",
    /*153*/ "WhiteSynchronMover",
    /*154*/ "BlackAntiSynchronMover",
    /*155*/ "WhiteAntiSynchronMover",
    /*156*/ "Masand",
    /*157*/ "BGL",
    /*158*/ "BlackChecks",
    /*159*/ "AnnanChess",
    /*160*/ "NormalPawn",
    /*161*/ "Lortap",
    /*162*/ "VaultingKings",
    /*163*/ "WhiteVaultingKing",
    /*164*/ "BlackVaultingKing",
    /*165*/ "ProteanChess",
    /*166*/ "GenevaChess",
    /*167*/ "ChameleonPursuit",
    /*168*/ "AntiKoeko",
    /*169*/ "CastlingChess",
    /*170*/ "LosingChess",
    /*171*/ "Disparate",
    /*172*/ "GhostChess",
    /*173*/ "HauntedChess",
    /*174*/ "ProvocationChess",
    /*175*/ "CageCirce",
    /*176*/ "Dummy",
    /*177*/ "Football",
    /*178*/ "ContraParrain",
    /*179*/ "KobulKings",
    /*180*/ "ExchangeCastling",
    /*181*/ "WhiteAlphabeticChess",
    /*182*/ "BlackAlphabeticChess",
    /*183*/ "CirceTake&Make",
    /*184*/ "SuperGuards",
    /*185*/ "Wormholes",
    /*186*/ "MarineChess",
    /*187*/ "UltramarineChess",
    /*188*/ "BackHome",
    /*189*/ "FaceToFace",
    /*190*/ "BackToBack",
    /*191*/ "CheekToCheek",
    /*192*/ "ChameleonSequence",
    /*193*/ "AntiCloneCirce",
    /*194*/ "LastCapture"
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
    "April",
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
    /*11*/  "a1<-->h1",
    /*12*/  "a1<-->a8",
    /*13*/  "a1<-->h8",
    /*14*/  "a8<-->h1",
    /*15*/  "translation",
    /*16*/  "remplace",
    /*17*/  "forsyth"
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
    /*11*/  "a1<-->h1",
    /*12*/  "a1<-->a8",
    /*13*/  "a1<-->h8",
    /*14*/  "a8<-->h1",
    /*15*/  "Verschiebung",
    /*16*/  "ersetze",
    /*17*/  "forsyth"
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
    /*11*/  "a1<-->h1",
    /*12*/  "a1<-->a8",
    /*13*/  "a1<-->h8",
    /*14*/  "a8<-->h1",
    /*15*/  "shift",
    /*16*/  "substitute",
    /*17*/  "forsyth"
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

char const * const *MessageTabs[] =
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
    /*15*/  "Cle en passant pas supportee avec trous de ver",
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
    /*26*/  "noir",
    /*27*/  "blanc",
    /*28*/  "certaines pieces feeriques incompatibles avec heffalumps",
    /*29*/  "enonce avec trops des coups",
    /*30*/  "pas des pieces royales en circe echange ou rex inclusiv.",
    /*31*/  "message inconnue %s",
    /*32*/  "element d'offenser: %s",
    /*33*/  "erreur d'entree:",
    /*34*/  "piece inconnue ou il manque une piece - ignoree",
    /*35*/  "specification de piece pas unique",
    /*36*/  "Echecs Annan et conditions qui changent la coleur de pieces interdit",
    /*37*/  "condition inconnue",
    /*38*/  "option pas unique - ignoree",
    /*39*/  "numero incorrecte",
    /*40*/  "option inconnue - ignoree",
    /*41*/  "specification de commande pas unique - ignoree",
    /*42*/  "commande non connue",
    /*43*/  "pas de specification de l'enonce",
    /*44*/  "Ne peut pas ouvrir fichier de sortie",
    /*45*/  "Ne peut pas ouvrir fichier d'entree",
    /*46*/  "Un trop grand nombre ont emboite des niveaux d'entree",
    /*47*/  "Aucune couleur n'a indique - toute l'entree de cahier des charges ignoree",
    /*48*/  "enonce inconnue",
    /*49*/  "condition pas unique - ignoree",
    /*50*/  "fin de fichier avant fin de probleme",
    /*51*/  "Debordement de ligne d'entree",
    /*52*/  "pas debut probleme",
    /*53*/  "Erreur interne de popeye dans le commutateur de tk",
    /*54*/  "solution terminee.",
    /*55*/  "blocus.",
    /*56*/  "menace:",
    /*57*/  "mais",
    /*58*/  "Temps = ",
    /*59*/  "on mate en un coup par\n",
    /*60*/  "\n",
    /*61*/  "pas de pieces ou conditions trop feeriques avec imitator",
    /*62*/  "trop d'imitators",
    /*63*/  "roi kamikaze seulement en circe rex inclusiv",
    /*64*/  "Gain_de_Piece et Circe Parrain interdit",
    /*65*/  "probleme ignoree",
    /*66*/  "anticirce incompatible avec certains genres feeriques",
    /*67*/  "echecs einstein incompatible avec des pieces feeriques",
    /*68*/  "supercirce incompatible avec certains genres feeriques",
    /*69*/  "entree dans 'hashtable': %s, hit-rate:",
    /*70*/  "On ne peut pas melanger EchecsChameleon et des pieces chameleon",
    /*71*/  "Calculation de",
    /*72*/  "-coups.",
    /*73*/  "enonce inconnue.\n",
    /*74*/  "Multiple buts pas acceptables si l'un d'eux est dia ou a=>b\n",
    /*75*/  "plus de 2 cases pour cle en passant interdit.\n",
    /*76*/  "Termination par signal %d apres %s\n",
    /*77*/  "pieces royales transmutees/reflectees interdites.\n",
    /*78*/  "Rotation/Miroir impossible.",
    /*79*/  "\nUne piece est deplacee hors de l'echiquier.",
    /*80*/  "Enplus doit immediatement suivre jumeau.\n",
    /*81*/  "Refute.",
    /*82*/  "FrischAufCirce: piece n'est pas une piece promue.\n",
    /*83*/  "certaines conditions feeriques et plus courte jeux justificative interdit.\n",
    /*84*/  "Isardam et Madrasi/Eiffel interdit",
    /*85*/  "on ne peut pas determiner si ce coup est legal.\n",
    /*86*/  "deja occupee - piece ancienne va dans l'au-dela.\n",
    /*87*/  "MarsCirce, EchecsPlus ou EchecsPhantom incompatible avec certains genres feeriques et avec des pieces feeriques.\n",
    /*88*/  "positions possibles dans",
    /*89*/  "Melange interdit",
    /*90*/  "On ne peut pas melanger EchecsVogtlander avec Isardam ou EchecsBrunner\n",
    /*91*/  "CirceAssassin est incompatible avec certains genres feeriques et avec des pieces feeriques.\n",
    /*92*/  "enonce doit etre mat avec condition EchecsRepublicain\n",
    /*93*/  ", increment HashRateLevel a %s\n",
    /*94*/  ", decroissance HashRateLevel a %s\n",
    /*95*/  "mode intelligent seulement avec h#/=, ser-#/= et ser-h#/=, facultativement avec certaines conditions, et sans pieces feeriques.\n",
    /*96*/  "la case est vide - on ne peut pas oter ou deplacer une piece.\n",
    /*97*/  "L'option  MaxTemps  n'est pas soutenu.\n",
    /*98*/  "L'option  FinApresSolutionsCourtes  n'est pas soutenu au jeu direct.\n",
    /*99*/  "Les pieces feeriques ne sont pas permises dans SingleBox",
    /*100*/ "erreur dans FichierLaTeX : piece feerique inconnue\n",
    /*101*/ "Plus de %s types de chasseurs differents\n",
    /*102*/ "Cases et/ou pieces royales incompatibles\n",
    /*103*/ "QuiPerdGagne incompatible avec les enonces echec ou mat\n",
    /*104*/ "Take&MakeEchecs ne sont pas compatibles avec certains genres feeriques\n",
    /*105*/ "Pieces magiques ne sont pas compatibles avec certaines pieces feeriques\n",
    /*106*/ "Trop de calculations magiques necessaires\n",
    /*107*/ "EchecsRepublicains incompatible avec des conditions qui dependent du fait si un coup donne echec.\n",
    /*108*/ "Popeye ne peut pas decider qui a le trait.\n",
    /*109*/ "Option \"menace\" pas applicable ou longueur maximale de la menace trop grande\n",
    /*110*/ "EchecsGhost et EchecsHantes incompatibles avec Circe, Anticirce, Kamikaze, Echecs de Haan\n",
    /*111*/ "ApparentSeul pas applicable - ignore\n",
    /*112*/ "Apparent pas applicable - ignore\n",
    /*113*/ "ApresCle pas applicable - ignore\n",
    /*114*/ "Quodlibet pas applicable - ignore\n",
    /*115*/ "ButEstFin pas applicable - ignore\n",
    /*116*/ "ButEstFin et Quodlibet incompatibles\n",
    /*117*/ "Il ne peut pas etre decide si une condition s'applique aux Blancs ou aux Noirs\n",
    /*118*/ "ImmunCage seulement avec CirceCage\n",
    /*119*/ "On ne peut pas decider si ce coup est refute\n"
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
    /*15*/  "En-Passant-Schluessel mit Wurmloechern nicht unterstuetzt",
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
    /*26*/  "schwarz",
    /*27*/  "weiss",
    /*28*/  "Manche Maerchenfiguren mit Heffalumps unvereinbar",
    /*29*/  "Zuviele Zuege in der Forderung",
    /*30*/  "Keine koeniglichen Figuren im PlatzwechselCirce oder RexInklusive\n",
    /*31*/  "Undefinierter Fehler %s",
    /*32*/  "Stoerende Eingabe: %s",
    /*33*/  "Eingabe-Fehler:",
    /*34*/  "Falsche oder fehlende Steinbezeichnung - wird ignoriert",
    /*35*/  "Steinbezeichnung nicht eindeutig",
    /*36*/  "AnnanSchach und Steine umfaerbende Bedingungen nicht erlaubt",
    /*37*/  "Bedingung nicht erkannt",
    /*38*/  "Option nicht eindeutig - ignoriert",
    /*39*/  "Keine oder falsche Zahlenangabe",
    /*40*/  "Option nicht erkannt - Einlesen von Optionen beendet",
    /*41*/  "Kommando nicht eindeutig - wird ignoriert",
    /*42*/  "Kommando unbekannt",
    /*43*/  "Keine Forderung angegeben",
    /*44*/  "Datei kann nicht zum Schreiben geoeffnet werden",
    /*45*/  "Datei kann nicht zum Lesen geoeffnet werden",
    /*46*/  "Zu viele geschachtelte Eingabedateien",
    /*47*/  "Keine Farbe angegeben - alle Spezifikationen ignoriert",
    /*48*/  "Forderung nicht bekannt",
    /*49*/  "Bedingung nicht eindeutig - ignoriert",
    /*50*/  "Ende der Eingabe vor EndeProblem",
    /*51*/  "Eingabezeilen Ueberlauf",
    /*52*/  "Kein AnfangProblem",
    /*53*/  "Interner Popeye Fehler in tk switch",
    /*54*/  "Loesung beendet.",
    /*55*/  "Zugzwang.",
    /*56*/  "Drohung:",
    /*57*/  "Aber",
    /*58*/  "Zeit = ",
    /*59*/  "Matt in einem Zug durch\n",
    /*60*/  "\n",
    /*61*/  "Zuviel Maerchenschach fuer Imitator",
    /*62*/  "Zuviele Imitatoren",
    /*63*/  "Kamikaze-Koenige nur in Circe RexInklusive",
    /*64*/  "Steingewinn Aufgaben mit Circe Parrain sind Frevel",
    /*65*/  "Problem uebersprungen",
    /*66*/  "AntiCirce mit einigen anderen Bedingungen nicht erlaubt",
    /*67*/  "EinsteinSchach nicht mit Maerchenfiguren erlaubt",
    /*68*/  "SuperCirce unvertraeglich mit einigen Maerchenbedingungen",
    /*69*/  "Stellungen: %s, Trefferrate:",
    /*70*/  "Chamaeleonschach und Chamaeleonsteine nicht gleichzeitig erlaubt",
    /*71*/  "Pruefe als",
    /*72*/  "-Zueger.",
    /*73*/  "Forderung wird nicht unterstuetzt.\n",
    /*74*/  "Mehrere Ziele nicht erlaubt, wenn eines davon dia oder a=>b ist\n",
    /*75*/  "Es koennen maximal zwei Felder fuer ep-Schluessel eingegeben werden.\n",
    /*76*/  "Abbruch durch Signal %d nach %s\n",
    /*77*/  "transmutierende/reflektierende koenigliche Steine nicht erlaubt.\n",
    /*78*/  "Drehung/Spiegelung nicht erlaubt: ",
    /*79*/  "\nEin Stein wird ueber den Brettrand verschoben.",
    /*80*/  "Ferner muss als erstes nach Zwilling angegeben werden.\n",
    /*81*/  "Widerlegt.",
    /*82*/  "FrischAufCirce: Stein ist keine Umwandlungsfigur, gg. Steine zuerst eingeben.\n",
    /*83*/  "Manche Maerchenbedingungen in Beweispartien nicht erlaubt.\n",
    /*84*/  "Isardam und Madrasi/Eiffelschach nicht erlaubt",
    /*85*/  "kann nicht entscheiden, ob dieser Zug legal ist.\n",
    /*86*/  "ist bereits besetzt - alter Stein geht in die Unterwelt.\n",
    /*87*/  "MarsCirce, PlusSchach oder PhantomSchach unvereinbar mit verschiedenen Maerchenbedingungen und -steinen.\n",
    /*88*/  "moegliche Stellungen in",
    /*89*/  "Unsinnige Kombination",
    /*90*/  "VogtlanderSchach nicht erlaubt in Verbindung mit Isardam oder BrunnerSchach\n",
    /*91*/  "Circe Assassin unvereinbar mit verschiedenen Maerchenbedingungen und -steinen\n",
    /*92*/  "Nur Mattforderung im RepublikanerSchach erlaubt\n",
    /*93*/  ", erhoehe HashRateLevel auf %s\n",
    /*94*/  ", vermindere HashRateLevel auf %s\n",
    /*95*/  "Intelligent Modus nur im h#/=, ser-#/= und ser-h#/=, wenigen Maerchenbedingungen und keinen Maerchensteinen.\n",
    /*96*/  "Feld leer - kann keine Figur entfernen/versetzen.\n",
    /*97*/  "Option  MaxZeit  nicht unterstuetzt.\n",
    /*98*/  "Option  StopNachKurzloesungen  nicht unterstuetzt im direkten Spiel.\n",
    /*99*/  "Maerchensteine nicht erlaubt in Single box",
    /*100*/ "LaTeXdatei nicht uebersetzbar: Maerchenstein nicht spezifiziert.\n",
    /*101*/ "Mehr als %s verschiedene Jaegertypen\n",
    /*102*/ "Unvereinbare koenigliche Felder und/oder Steine\n",
    /*103*/ "Schlagschach unvereinbar mit Schach- oder Mattforderungen\n",
    /*104*/ "Take&MakeSchach unvereinbar mit verschiedenen Maerchenbedingungen\n",
    /*105*/ "Magische Steine unvereinbar mit verschiedenen Maerchenfiguren\n",
    /*106*/ "Zu viele magische Berechnungen benoetigt\n",
    /*107*/ "Republikanerschach nicht mit Bedingungen, die davon abhangen, ob gewisse Zuege Schach bieten.\n",
    /*108*/ "Es ist nicht feststellbar, wer am Zug ist\n",
    /*109*/ "Option \"Drohung\" nicht anwendbar order maximale Laenge der Drohung zu gross\n",
    /*110*/ "Geisterschach und Spukschach unvereinbar mit Circe, Anticirce, Kamikaze, Haaner Schach\n",
    /*111*/ "WeissBeginnt nicht anwendbar - ignoriert\n",
    /*112*/ "Satzspiel nicht anwendbar - ignoriert\n",
    /*113*/ "NachSchluessel nicht anwendbar - ignoriert\n",
    /*114*/ "Quodlibet nicht anwendbar - ignoriert\n",
    /*115*/ "ZielIstEnde nicht anwendbar - ignoriert\n",
    /*116*/ "ZielIstEnde und Quodlibet nicht kompatibel\n",
    /*117*/ "Es ist nicht entscheidbar, ob Bedingung fuer Weiss oder Schwarz gilt\n",
    /*118*/ "KaefigImmunSchach nur mit KaefigCirce\n",
    /*119*/ "Kann nicht entscheiden, ob dieser Zug widerlegt wird\n"
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
    /*15*/  "En passant key not supported with Wormholes",
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
    /*26*/  "black",
    /*27*/  "white",
    /*28*/  "some fairy-pieces not compatible with heffalumps",
    /*29*/  "stipulation with too many moves",
    /*30*/  "no royal pieces with PWC or rex inclusive",
    /*31*/  "undefined Message %s",
    /*32*/  "offending item: %s",
    /*33*/  "input-error:",
    /*34*/  "wrong or missing PieceName - ignored",
    /*35*/  "PieceSpecification not uniq",
    /*36*/  "Annan Chess and conditions that recolour pieces not allowed",
    /*37*/  "could not recognize condition",
    /*38*/  "option not uniq - ignored",
    /*39*/  "wrong integer",
    /*40*/  "could not recognize option - parsing of options aborted",
    /*41*/  "command not uniq specified - ignored",
    /*42*/  "command not known",
    /*43*/  "no stipulation specified",
    /*44*/  "cannot open output-file",
    /*45*/  "cannot open input-file",
    /*46*/  "too many nested input levels",
    /*47*/  "no color specified - all specification input ignored",
    /*48*/  "cannot recognize the stipulation",
    /*49*/  "condition not uniq - ignored",
    /*50*/  "end of file before end of problem",
    /*51*/  "input line overflow",
    /*52*/  "no begin of problem",
    /*53*/  "internal popeye error in tk switch",
    /*54*/  "solution finished.",
    /*55*/  "zugzwang.",
    /*56*/  "threat:",
    /*57*/  "but",
    /*58*/  "Time = ",
    /*59*/  "mate in one move with\n",
    /*60*/  "\n",
    /*61*/  "conditions too fairy for imitator",
    /*62*/  "too many imitators",
    /*63*/  "kamikaze king only in Circe rex inclusiv",
    /*64*/  "Win_a_Piece and Circe Parrain not allowed",
    /*65*/  "problem ignored",
    /*66*/  "anticirce incompatible with some fairy conditions",
    /*67*/  "einsteinchess not allowed with fairy pieces",
    /*68*/  "supercirce incompatible with some fairy conditions",
    /*69*/  "positions: %s, hitrate:",
    /*70*/  "ChameleonChess and ChameleonPieces not allowed together",
    /*71*/  "Checking in",
    /*72*/  "_moves.",
    /*73*/  "Stipulation not supported.\n",
    /*74*/  "Multiple goals not allowed if one of them is dia or a=>b\n",
    /*75*/  "Not more than 2 squares can be putin for en passant keys.\n",
    /*76*/  "Abort by signal %d after %s\n",
    /*77*/  "Transmuting/reflecting royal pieces not supported.\n",
    /*78*/  "This rotation/mirroring is not supported: ",
    /*79*/  "\nA piece will be moved outside.",
    /*80*/  "Continued must be given immediately after twin.\n",
    /*81*/  "refutes.",
    /*82*/  "FrischAufCirce: piece is not a promoted piece. Enter the pieces first.\n",
    /*83*/  "Some fairy conditions in proof games not allowed.\n",
    /*84*/  "Isardam and Madrasi/Eiffel not allowed",
    /*85*/  "cannot decide if this move is legal or not.\n",
    /*86*/  "already occupied - old piece goes to the underworld.\n",
    /*87*/  "MarsCirce, PlusChess or PhantomChess is incompatible with several fairy conditions and fairy pieces.\n",
    /*88*/  "potential positions in",
    /*89*/  "nonsense combination",
    /*90*/  "VogtlanderChess not allowed in combination with Isardam or BrunnerChess\n",
    /*91*/  "Circe Assassin is incompatible with several fairy conditions and fairy pieces\n",
    /*92*/  "Must stipulate mate with Republican Chess\n",
    /*93*/  ", increment HashRateLevel to %s\n",
    /*94*/  ", decrement HashRateLevel to %s\n",
    /*95*/  "Intelligent Mode only with h#/=, ser-#/= and ser-h#/=, with a limited set of fairy conditions and without fairy pieces.\n",
    /*96*/  "square is empty - cannot (re)move any piece.\n",
    /*97*/  "Option  MaxTime  not supported.\n",
    /*98*/  "Option  StopOnShortSolutions  not supported with direct play.\n",
    /*99*/  "Fairy pieces not allowed in Single box",
    /*100*/ "LaTeXFile will not compile: undefined fairy piece\n",
    /*101*/ "Limit of %s different hunter types exceeded\n",
    /*102*/ "Incompatible royal pieces and/or squares\n",
    /*103*/ "LosingChess incompatible with check or mate stipulations\n",
    /*104*/ "Take&MakeChess is incompatible with some fairy conditions\n",
    /*105*/ "Magic pieces incompatible with some fairy pieces\n",
    /*106*/ "Too many magic calculations needed\n",
    /*107*/ "RepublicanChess incompatible with conditions that depend on whether a certain move delivers check.\n",
    /*108*/ "Can't decide which side is at the move\n",
    /*109*/ "Option \"Threat\" not applicable or indicated maximum threat length too big\n",
    /*110*/ "GhostChess and HauntedChess incompatible with Circe, Anticirce, Kamikaze, Haaner Chess\n",
    /*111*/ "WhiteToPlay not applicable - ignored\n",
    /*112*/ "SetPlay not applicable - ignored\n",
    /*113*/ "PostKeyPlay not applicable - ignored\n",
    /*114*/ "Quodlibet not applicable - ignored\n",
    /*115*/ "GoalIsEnd not applicable - ignored\n",
    /*116*/ "GoalIsEnd and Quodlibet are incompatible\n",
    /*117*/ "Can't decide whether condition applies to White or Black\n",
    /*118*/ "CageImmunChess only with CageCirce\n",
    /*119*/ "Can't decide whether this move is refuted\n"
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
