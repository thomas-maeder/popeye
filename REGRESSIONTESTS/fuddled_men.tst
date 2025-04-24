
no check by rook that has just castled

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
3   .   .   .   .  -P   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   R   .   .   .   K   .   .  -K   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1                         2 + 2
             FuddledMen

  1.Kh1-g1 Ke1-e2 #
  add_to_move_generation_stack:          77
                     play_move:          71
 is_white_king_square_attacked:          16
 is_black_king_square_attacked:          71

solution finished.



no check by pawn that just been involved in a Messigny exchange

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
4   .   .   .  -K   .   .   .   .   4
|                                   |
3   K   .  -P   .   .   .   .   .   3
|                                   |
2   .   P   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  +1                          2 + 2
            MessignyChess
             FuddledMen

  1  (Pb2<->Pc3 )
  2  (b2-b4 )
  3  (b2-b3 )
  4  (b2*c3 )
  5  (Ka3<->Kd4 + )
   1.Ka3<->Kd4 + !

  6  (Ka3-b4 )
  7  (Ka3-a2 )
  8  (Ka3-a4 )
  9  (Ka3-b3 )
  add_to_move_generation_stack:           9
                     play_move:           9
 is_white_king_square_attacked:           1
 is_black_king_square_attacked:          19

solution finished.



no check by rook that just been involved in a ExchangeCastling exchange

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   R   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .  -K   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  +1                          2 + 1
          ExchangeCastling

a) 

   1.Ra5<->Ka1 + !

   1.Ra5-h5 + !

  add_to_move_generation_stack:          17
                     play_move:          17
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:          18

b) ExchangeCastling
   FuddledMen  

  add_to_move_generation_stack:          17
                     play_move:          17
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:          18

solution finished.


