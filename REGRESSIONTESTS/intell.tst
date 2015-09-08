
     double check with promotee

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   P   .   .  -B   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .  -P   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   R   .   4
|                                   |
3   .   .   .  -K   .   .   .   .   3
|                                   |
2   .   .   .  -S   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   K   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2.5                       3 + 4

  1...e7-e8=B   2.Kd3-e2 Be8-h5   3.Bh7-d3 Rg4-e4 #

  add_to_move_generation_stack:       34585
                     play_move:       35413
 is_white_king_square_attacked:        1367
 is_black_king_square_attacked:        2430
solution finished.



    double check by pawn capture

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .  -P   .   .  -S   .   .   7
|                                   |
6   .   .  -Q  -K   .   K   .   .   6
|                                   |
5   .   .  -P   .   .   .   .   .   5
|                                   |
4   .   .   .  -P   .   .   .   .   4
|                                   |
3   .   .   .   .   P   .   .   .   3
|                                   |
2   .   .   .   R   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                       3 + 6

  1...e3*d4   2.Sf7-e5 d4*e5 #

  add_to_move_generation_stack:         103
                     play_move:         103
 is_white_king_square_attacked:           4
 is_black_king_square_attacked:          33
solution finished.



double check by en passant capture from above

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   R   .   .   .   .   8
|                                   |
7   .   .   .   .  -P   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   B   6
|                                   |
5   .   .  -R   P   .   .   .   .   5
|                                   |
4   .   .  -P  -K   .   .   .   .   4
|                                   |
3   .   .   .   .   .   K   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                       4 + 4

  1...Bh6-g7 +   2.e7-e5 d5*e6 ep. #

  add_to_move_generation_stack:         278
                     play_move:         269
 is_white_king_square_attacked:          13
 is_black_king_square_attacked:          42
solution finished.



double check by en passant capture from below

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   K  -P   .   .   .   .   .   7
|                                   |
6   .   .   .  -K  -P   .   .   .   6
|                                   |
5   .   .   .   P  -P   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   B   .   R   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                       4 + 4

  1...Bb2-a3 +   2.c7-c5 d5*c6 ep. #

  add_to_move_generation_stack:         720
                     play_move:         647
 is_white_king_square_attacked:          40
 is_black_king_square_attacked:         121
solution finished.



interception of a defending pawn by a piece that doesn't guard

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   S   .   .   .   .   .   8
|                                   |
7  -P  -P   .   .   .   .   .   .   7
|                                   |
6  -K   .   .   .   .   .   .   .   6
|                                   |
5  -P   .   P   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   B   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ser-#2                      3 + 4

  1.Sc8-b6   2.Bh3-f1 #

  add_to_move_generation_stack:         110
                     play_move:          99
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:          63
solution finished.



guard with check -> selfpin with check -> selfpin with guard by s

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   K   .   .   5
|                                   |
4   .   .   .   .   S   .   .   .   4
|                                   |
3   .   .   Q  -B  -K   .   .   .   3
|                                   |
2   .   .   .   .  -P   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#0.5                       4 + 3

  1...Rh1-h3 #

  add_to_move_generation_stack:         133
                     play_move:         117
 is_white_king_square_attacked:          14
 is_black_king_square_attacked:         102
solution finished.



guard with check -> selfpin with check -> selfpin with guard by b

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   S   6
|                                   |
5   .   .   .   .   .   K   .   .   5
|                                   |
4   .   .   .   .   B   .   .   .   4
|                                   |
3   .   .   Q  -B  -K   .   .   .   3
|                                   |
2   .   .   .   .  -P   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#0.5                       4 + 3

  1...Sh6-g4 #

  add_to_move_generation_stack:          25
                     play_move:          23
 is_white_king_square_attacked:           3
 is_black_king_square_attacked:          21
solution finished.



in mate stipulations, it makes sense to pin a queen or rook by a
           guarding queen

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   S   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .  -P   .   .   .   2
|                                   |
1   .   .   Q  -Q  -K  -B   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#0.5                       2 + 4

  1...Sc5-d3 #

  add_to_move_generation_stack:          24
                     play_move:          21
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:          17
solution finished.


