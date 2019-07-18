Popeye Linux-4.15.0-54-generic-x86_64-64Bit v4.82 (1024 MB)

castling only with real king and rook

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .  -K   .   .   .  -R   8
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
2   .   .   .   .   .   .   K   .   2
|                                   |
1   S   .   .   .   S   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  001                         3 + 2
           PointReflection

a) 

  add_to_move_generation_stack:           0
                     play_move:           0
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           1

b) -wSe1  wKg2-->e1  

  add_to_move_generation_stack:           0
                     play_move:           0
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           1

+c) wRa1  

   1.0-0-0 + !

  add_to_move_generation_stack:          16
                     play_move:           1
 is_white_king_square_attacked:           3
 is_black_king_square_attacked:           2

solution finished. Time = 0.063 s



en passant capture only by real pawn

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .  -S   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   S   .   .   .   .   5
|                                   |
4   .   .   .   .  -P   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   P   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hep1                        2 + 2
           PointReflection

a) 
both sides need a king

  add_to_move_generation_stack:          10
                     play_move:          10
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

b) bPe7  
both sides need a king

  1.e7-e5 Sd5*e6 ep.
  add_to_move_generation_stack:          14
                     play_move:          11
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

+c) wPd5  
both sides need a king

  1.e7-e5 d5*e6 ep.
  add_to_move_generation_stack:           7
                     play_move:           4
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished. Time = 1.051 s


