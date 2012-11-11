
White king is not in check on b7 even if nBc8*bKb7 is not the longest
              white move

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .  =B   .   .   .   .   .   8
|                                   |
7   .  -P   .   .   .   .   .   .   7
|                                   |
6   K   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #1                     1 + 2 + 1n
           WhiteMaximummer
             BrunnerChess

   1.Ka6*b7 # !


              empile:          38
           play_move:          36
        orig_rbechec:          16
        orig_rnechec:          20
solution finished. 



White king is not in check on d5 even if c6-d5 is shorter than the
         longest move of wBc6

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .  -P   .   .   7
|                                   |
6   .   .   B   .   K   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .  -B   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  zd51                        2 + 2
           WhiteMaximummer
               Isardam

   1.Ke6-d5 z !


              empile:          26
           play_move:          23
        orig_rbechec:           7
        orig_rnechec:           0
solution finished. 



a8 is no cage for bBb2 even if Ba8-b7 is shorter than knight moves.
So the only cage is a1, but that is self-check.

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .  -S   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .  -K   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .  -B   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 3
           BlackMaximummer
           WhiteMaximummer
              CageCirce

   1.Ka1-b1 !

   1.Ka1-a2 !


              empile:         476
           play_move:          72
        orig_rbechec:           4
        orig_rnechec:          63
solution finished. 



White Maximummer doesn't cause the test for the legality of 1.Ke1-f1
(necessary precondition for playing 0-0) to fail

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   P   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   K   .   .   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  001                         3 + 0
           WhiteMaximummer

   1.0-0 !


              empile:          11
           play_move:           3
        orig_rbechec:           5
        orig_rnechec:           0
solution finished. 



don't ignore the longest move just because of self-check if it reaches
               the goal

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .  -R   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6  -P   .  -P   .   .   .   .   .   6
|                                   |
5  -P  -K   P   .   .   .   .   .   5
|                                   |
4   .  -B   .   P   .   .   .   .   4
|                                   |
3   .   P   .   .   .   .   .   .   3
|                                   |
2   .   K   P   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .  -R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h##1                        5 + 7
           WhiteMaximummer

  1.Rh8-h2 c2-c4 ## 

              empile:        1053
           play_move:         585
        orig_rbechec:         659
        orig_rnechec:          97
solution finished. 




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   P   8
|                                   |
7   B  -P   .   R   .   .   .   .   7
|                                   |
6   R   S   B   .   .   .   .   .   6
|                                   |
5   S   .   .   Q   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h~1                         8 + 1
           BlackMaximummer
           WhiteMaximummer
          SingleBox    Type3

  1.b7*a6 [h8=R]Rh8-h1 
  1.b7*a6 [h8=R]Rh8-a8 
  1.b7*c6 [h8=B]Bh8-a1 

              empile:         103
           play_move:          21
        orig_rbechec:           0
        orig_rnechec:           0
solution finished. 


