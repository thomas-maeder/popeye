
White at the move, but Black in check

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .   .   .   .   .   8
|                                   |
7   .   P   .   .   .   .   .   .   7
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
1   .   .   .   .   .   .   .   K   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                       2 + 1

the side to play can capture the king  add_to_move_generation_stack:           0
                     play_move:           0
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           1

solution finished.



h#1 with set play, but black king is in check

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .  -K   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   K   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   S   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   B   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1                         3 + 1

the side to play can capture the king
  1.Kb8-a8 Sc4-b6 #
  add_to_move_generation_stack:          21
                     play_move:          18
 is_white_king_square_attacked:           5
 is_black_king_square_attacked:          19

solution finished.



       hep1 (help-en passant)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .  -K   8
|                                   |
7   .   .   .   .   .   .  -P   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   P   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   K   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hep1                        2 + 2

  1.g7-g5 h5*g6 ep.
  add_to_move_generation_stack:           9
                     play_move:           5
 is_white_king_square_attacked:           3
 is_black_king_square_attacked:           5

solution finished.



               h001.5

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .  -K   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .  -B   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   P   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   R   .   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h001.5                      3 + 2

  1...h3-h4   2.Bh6-g7 0-0-0
  add_to_move_generation_stack:         384
                     play_move:         192
 is_white_king_square_attacked:          51
 is_black_king_square_attacked:         168

solution finished.



                h##!2

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .  -Q   8
|                                   |
7   .   .   .   .   .   .  -R   B   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   K   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1  -K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h##!2                       2 + 3

  1.Rg7-g4 Bh7-b1   2.Qh8-c3 + Ka3-a2 ##!
  add_to_move_generation_stack:       15132
                     play_move:       12053
 is_white_king_square_attacked:        9827
 is_black_king_square_attacked:        5808

solution finished.



                h##2

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
4   .   .   .   .   K   .  -K   S   4
|                                   |
3   .   .   .   .   .   .   P   .   3
|                                   |
2   .   .   .   .  -P   .  -P  -Q   2
|                                   |
1   .   .   .   .   .   .   .   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h##2                        4 + 5

  1.Kg4-h3 Ke4-f3   2.g5-g4 + Kf3*g2 ##
  add_to_move_generation_stack:       99152
                     play_move:       37763
 is_white_king_square_attacked:       31333
 is_black_king_square_attacked:        6584

solution finished.



    not 1.Qb2+ because of exact-

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   Q   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3  -K   .  -P   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   B   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  exact-hr#2                  3 + 2

  1.Qb5-e2 Ka3-b3   2.Qe2-b2 + c3*b2 #
  add_to_move_generation_stack:       11919
                     play_move:       10485
 is_white_king_square_attacked:        9129
 is_black_king_square_attacked:        2754

solution finished.



hr#1.5 - don't write long solutions because Black must mate immediately

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .  -P   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   P   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hr#1.5                      2 + 4

  1...b3-b2 #
  add_to_move_generation_stack:          10
                     play_move:          10
 is_white_king_square_attacked:          11
 is_black_king_square_attacked:           2

solution finished.



hr#2 - don't write long solutions because Black must mate immediately

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .  -P   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   P   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hr#2                        2 + 4

  1.f3-f4 b3-b2 #
  add_to_move_generation_stack:          18
                     play_move:          18
 is_white_king_square_attacked:          18
 is_black_king_square_attacked:           4

solution finished.



stop solving when the goal is reached

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5  -P   .   .   .   .   .   .   .   5
|                                   |
4  DU  DU   .   .   .   .   .   .   4
|                                   |
3   .  DU  DU   .   .   .   .   P   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hsx2                        5 + 1

  1.h3-h4 a5*b4 x
  add_to_move_generation_stack:           5
                     play_move:           5
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



 no solution because White must mate

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .  -K   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   K   .   .   .   .   Q   .   .   6
|                                   |
5   .   .   .   .   .   .   .  -R   5
|                                   |
4   .   .   .   .   .   .  -R   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hr#1                        2 + 3

  1.Qf6-d8 #
  add_to_move_generation_stack:          76
                     play_move:          63
 is_white_king_square_attacked:           8
 is_black_king_square_attacked:          64

solution finished.


PostKeyPlay not applicable - ignored

ignore option postkeyplay in help play

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
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                         0 + 0

a) 

  add_to_move_generation_stack:           0
                     play_move:           0
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
PostKeyPlay not applicable - ignored

b) hs#2  

  add_to_move_generation_stack:           0
                     play_move:           0
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.


