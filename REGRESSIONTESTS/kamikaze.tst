
a target square is considered to be reached if the last move goes to
the target square and is there at the end of the move

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .  -S   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   B   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  zh71                        1 + 1
             Kamikaze d3

a) 

  add_to_move_generation_stack:          11
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

b) Circe  

  add_to_move_generation_stack:          11
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

c) Circe  bSh7-->f1  zf11  

   1.kBd3*f1[wkBf1->f1][+bSg8] z !

  add_to_move_generation_stack:          11
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .  -P   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   R   .   .   .   .   .   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  <>3                         2 + 1
             Kamikaze b1
                Circe

   1.kRb1-b2 !
      1...b5-b4
          2.Rh1-b1
              2...b4-b3
                  3.kRb2*b3[wkRb3->h1][+bPb7] <>

  add_to_move_generation_stack:        7145
                     play_move:        7116
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



exchange of pieces ending on the capture and rebirth on the square
originally occupied by the other exchanger

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   K   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .  -P   .   4
|                                   |
3   .   .   P   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   S   .   .   R   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  <>4                         4 + 1
           Kamikaze g4 d1
                Circe

   1.kSd1-f2 !
      1...kPg4-g3
          2.kSf2-h3
              2...kPg3-g2
                  3.Rg1-d1
                      3...kPg2-g1=kQ
                          4.kSh3*g1[wkSg1->g1][+bkQd8] <>
                      3...kPg2-g1=kS
                          4.kSh3*g1[wkSg1->g1][+bkSb8] <>
                      3...kPg2-g1=kR
                          4.kSh3*g1[wkSg1->g1][+bkRh8] <>
                      3...kPg2-g1=kB
                          4.kSh3*g1[wkSg1->g1][+bkBf8] <>

  add_to_move_generation_stack:       47249
                     play_move:       46676
 is_white_king_square_attacked:       11824
 is_black_king_square_attacked:           0

solution finished.


