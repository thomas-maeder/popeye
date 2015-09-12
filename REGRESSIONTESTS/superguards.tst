
a) capturing the rook is illegal and capturing the bishop is not check

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
3  -R   .  -K   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   R   .  -B   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  +1                          1 + 3
             SuperGuards

a) 

  1  (Ra1-a2 )
  2  (Ra1*c1 )
  3  (Ra1-b1 )
  add_to_move_generation_stack:           4
                     play_move:           3
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           7

b) bSc1  

  4  (Ra1*a3 + )
   1.Ra1*a3 + !

  5  (Ra1-a2 )
  6  (Ra1*c1 )
  7  (Ra1-b1 )
  add_to_move_generation_stack:           4
                     play_move:           4
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           9

solution finished.


