

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
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .  -R   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   .   .   S   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 2
           Make&TakeChess

   1.Sc1*c3 !

   1.Sc1-e2 !

   1.Sc1-d3 !

   1.Sc1-b3 !

  add_to_move_generation_stack:          40
                     play_move:           4
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



     black is in check from se8!

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -R   .   .   .   S   .   .   .   8
|                                   |
7  -K   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
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
  zg71                        1 + 2
           Make&TakeChess

the side to play can capture the king  add_to_move_generation_stack:          40
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



         en passant capture

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .  -P   P   7
|                                   |
6   .   .   .   .   .   P   .   .   6
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
  hx1                         2 + 1
           BlackMaximummer
           Make&TakeChess

  1.g7-g5 f6*g6 ep. x
  1.g7-g5 h7*g6 ep. x
  add_to_move_generation_stack:          14
                     play_move:           5
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



Ke2 would be self-check because of castling

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
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   K  -S   .   .  -R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ze21                        1 + 2
           Make&TakeChess

  add_to_move_generation_stack:         185
                     play_move:           2
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



 no make&take of pawn via base line

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
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .  -P   P   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
           Make&TakeChess

a) 

   1.h3*g3 x !

  add_to_move_generation_stack:           5
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

b) shift a8 ==> a7  

  add_to_move_generation_stack:           3
                     play_move:           0
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



          castling as make

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -R   .   .   .   Q   .   .  -R   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .  -K   .   .   .   6
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
  ze61                        1 + 3
         Circe RexInclusive
           Make&TakeChess

  1  (Qe8*e6[+bKe8] + )
   1.Qe8*e6[+bKe8] + z !

  2  (Qe8*e6[bRh8-f8][+bKe8] + )
   1.Qe8*e6[bRh8-f8][+bKe8] + z !

  3  (Qe8*e6[bRa8-d8][+bKe8] + )
   1.Qe8*e6[bRa8-d8][+bKe8] + z !

  add_to_move_generation_stack:        1822
                     play_move:          25
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.


