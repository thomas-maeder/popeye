

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
  add_to_move_generation_stack:        6997
                     play_move:       17789
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        5461
                  TI decisions:        8566

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

  add_to_move_generation_stack:        8154
                     play_move:       22164
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        6885
                  TI decisions:      173466

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
  add_to_move_generation_stack:        5612
                     play_move:       13666
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        4877
                  TI decisions:        7057

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

  add_to_move_generation_stack:        3001
                     play_move:        5950
 is_white_king_square_attacked:         252
 is_black_king_square_attacked:        1527
                  TI decisions:        1021

b)   

  1...0-0   2.h5-h4 Rf1-f3 #
  add_to_move_generation_stack:        3107
                     play_move:        5380
 is_white_king_square_attacked:         380
 is_black_king_square_attacked:        1387
                  TI decisions:         904

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
  add_to_move_generation_stack:        8412
                     play_move:       12928
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        7735
                  TI decisions:       18023

solution finished.



capture by invisible pawn results in promotion

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .  -B   .   S   .   .   .   .   8
|                                   |
7  -P   .   .   .   .   .   .  -R   7
|                                   |
6  -B  -K   .   K   .   .   .   .   6
|                                   |
5  -P   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .  -R   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  2 + 7 + 1 TI

  1.Rb1-b5 TI~*b8   2.Rh7-c7 TI~*c7[c7=wB] #
  1.Rb1-b5 TI~*b8   2.Rh7-d7 TI~*d7[d7=wS] #
  add_to_move_generation_stack:      264773
                     play_move:      576910
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:      146488
                  TI decisions:      688312

solution finished.



random move by invisible pawn results in promotion

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   B   5
|                                   |
4   .   .   .   .   .   S   .   .   4
|                                   |
3   .   .   .   .   P   K   .   .   3
|                                   |
2   .   .   .   .   P   B   .   .   2
|                                   |
1   .   .   .   .   .  -K   .  -B   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  6 + 2 + 1 TI

  1.TI~-~ Kf3-g3   2.TI~*e2[e2=bS] Bh5*e2 #
  add_to_move_generation_stack:        3911
                     play_move:       11966
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        2185
                  TI decisions:       16577

solution finished.


