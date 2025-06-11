
Alice Chess move generation and execution

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .  -P   .   5
|                                   |
4   .   .   .   .   .   P   .   B   4
|                                   |
3   .   .   .   .   .   .  -P   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   K   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          3 + 2
           AliceA h4 g3 d1
            AliceB g5 f4
             AliceChess

   1.f4-f5A !

   1.f4*g5A !

   1.Kd1-e2B !

   1.Kd1-c2B !

   1.Kd1-c1B !

   1.Kd1-d2B !

   1.Kd1-e1B !

   1.Bh4*g3B !

   1.Bh4-d8B !

   1.Bh4-e7B !

   1.Bh4-f6B !

  add_to_move_generation_stack:          12
                     play_move:          11
 is_white_king_square_attacked:          16
 is_black_king_square_attacked:           0

solution finished.



  Alice check only from same board

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -K   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   P   .   P   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  +1                          2 + 1
            AliceA d5 c3
              AliceB e3
             AliceChess

   1.e3-e4A + !

  add_to_move_generation_stack:           2
                     play_move:           2
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           3

solution finished.



 special check rule after king move

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
3  -P   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 1
            AliceA a3 a1
               AliceB
             AliceChess

   1.Ka1-a2B !

   1.Ka1-b1B !

  add_to_move_generation_stack:           3
                     play_move:           3
 is_white_king_square_attacked:           6
 is_black_king_square_attacked:           0

solution finished.



no special check rule after non-king move

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   P   .   .   .   .   .   .   6
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
  +1                          1 + 1
            AliceA a8 b6
               AliceB
             AliceChess

a) 

  add_to_move_generation_stack:           1
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           2

b) wPb6  

   1.b6-b7A + !

  add_to_move_generation_stack:           1
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           2

solution finished.



  castling transfers both partners

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
1   R   .   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  001                         2 + 0
            AliceA a1 e1
               AliceB
             AliceChess

   1.0-0-0BB !

  add_to_move_generation_stack:          16
                     play_move:          16
 is_white_king_square_attacked:           4
 is_black_king_square_attacked:           0

solution finished.



no castling if intermediate square is occupied on other board

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
1   R   .   .  -S   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  001                         2 + 1
            AliceA a1 e1
              AliceB d1
             AliceChess

  add_to_move_generation_stack:          16
                     play_move:          13
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:           0

solution finished.



en passant capture only of pawns on same board

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .  -P   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   P   .   P   .   .   5
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
  hep1                        2 + 1
            AliceA e7 d5
              AliceB f5
             AliceChess

  1.e7-e5B f5*e6 ep.A
  add_to_move_generation_stack:           6
                     play_move:           5
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.


