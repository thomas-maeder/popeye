

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
  add_to_move_generation_stack:        7013
                     play_move:       18091
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        5569
                  TI decisions:        9488

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
                  TI decisions:      187425

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
                     play_move:       13807
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:        4924
                  TI decisions:        7850

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
                     play_move:        5968
 is_white_king_square_attacked:         252
 is_black_king_square_attacked:        1533
                  TI decisions:        1039

b)   

  1...0-0   2.h5-h4 Rf1-f3 #
  add_to_move_generation_stack:        3107
                     play_move:        5398
 is_white_king_square_attacked:         380
 is_black_king_square_attacked:        1393
                  TI decisions:         922

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
                  TI decisions:       72585

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
                  TI decisions:      694850

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
                  TI decisions:       16606

solution finished.



the mate wasn't recognized for a long time because of a silly bug

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
4   .   .   .   .   Q   .  -Q   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  2 + 1 + 3 TI

 16  (Qg4-f4 )
 16:4  (0-0 )
 16:4:10  (Qf4-g5 )
 16:4:10:7  (Rf1-e1 )
 16:4:10:8  (Rf1-f8 )
 16:4:10:9  (Rf1-f7 )
 16:4:10:10  (Rf1-f6 )
 16:4:10:11  (Rf1-f5 )
 16:4:10:12  (Rf1-f4 )
 16:4:10:13  (Rf1-f3 )
  1.Qg4-f4 0-0   2.Qf4-g5 Rf1-f3[g3=bK] #
  add_to_move_generation_stack:         230
                     play_move:         223
 is_white_king_square_attacked:           8
 is_black_king_square_attacked:          88
                  TI decisions:         106

solution finished.


