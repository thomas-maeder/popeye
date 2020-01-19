

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
2   .   K   .  -S  -P   .   .   .   2
|                                   |
1   .   R  -S  -K   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#0.5                       2 + 4

  1...Rb1*c1 #
  add_to_move_generation_stack:          11
                     play_move:          14
 is_white_king_square_attacked:           1
 is_black_king_square_attacked:          15

solution finished.




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6  -K   .   .   .   .   .   .   .   6
|                                   |
5   P   .   .   .   .   .   .   .   5
|                                   |
4   .   .   K   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   R   R   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                4 + 1 + 1 TI

  1...Rb1-b8   2.TI~-~ [+bTIb6]a5*b6 #
  1...Rb1-b5   2.TI~*a5 Ra1*a5 #
  1...Rb1-b4   2.Ka6*a5 Ra1-a4 #
  add_to_move_generation_stack:        7017
                     play_move:       18095
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        5569
                  TI decisions:        8820

solution finished.



1...Rh1-c1   2..~-~ Bh8-c3 # is not a solution because a bTI can have done d4-b2

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   B   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   K   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1  -K   .   .   .   .   .   .   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                3 + 2 + 2 TI

  add_to_move_generation_stack:        8117
                     play_move:       22062
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        6819
                  TI decisions:      177352

solution finished.



1...Rb1-b8   2..~-~ [+bTIb6]a5*b6 # is not a solution because a bTI can not have moved to b6

+---a---b---c---d---e---f---g---h---+
|                                   |
8       .       .   .   .   .   .   8
|                                   |
7   .   .       .   .   .   .   .   7
|                                   |
6  -K   .   .       .   .   .   .   6
|                                   |
5   P   .   K       .   .   .   .   5
|                                   |
4   .   .       .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   R   R   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                4 + 1 + 1 TI

  1...Ra1-a3   2.Ka6*a5 TI~-~ #
  1...Rb1-b7   2.Ka6*a5 Rb7-a7 #
  1...Rb1-b5   2.TI~*a5 Ra1*a5 #
  1...Rb1-b4   2.Ka6*a5 Ra1-a4 #
  1...Kc5-c6   2.TI~-~ [+bTIb6]a5*b6 #
  add_to_move_generation_stack:        5616
                     play_move:       13811
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        4924
                  TI decisions:        7337

solution finished.



no castling if the invisible rook would have delivered check

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .  -P   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   S   .   .  -K   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                2 + 2 + 1 TI

a) 

  add_to_move_generation_stack:        2966
                     play_move:        5947
 is_white_king_square_attacked:         377
 is_black_king_square_attacked:        1533
                  TI decisions:        1033

b)   

  1...[+wRh1]0-0[f1=wR]   2.h5-h4 Rf1-f3 #
  add_to_move_generation_stack:        3076
                     play_move:        5381
 is_white_king_square_attacked:         488
 is_black_king_square_attacked:        1393
                  TI decisions:         916

solution finished.



this doesn't work if we overdo it with optimisations

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .  -R  -K   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .  -P   .   .   .   K   6
|                                   |
5   .   .   .   .   .   .   .  -B   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .  -R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                1 + 5 + 2 TI

  1...TI~*d6   2.Bh5-f7 TI~*f7[f7=wS] #
  add_to_move_generation_stack:        7114
                     play_move:       11468
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        6383
                  TI decisions:       11634

solution finished.


