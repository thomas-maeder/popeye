

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .  -S   .   .   .   6
|                                   |
5   .   .   .   .   .   .   P   .   5
|                                   |
4   .   .   .   P   P   P   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .  -P   P   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          5 + 2
             DangerCirce

   1.g2-g4 !

   1.g2-g3 !

   1.d4-d5 !

   1.d4-d2 !

   1.e4-e5 !

   1.f4-f5 !

   1.f4*f2 !

   1.g5-g6 !

  add_to_move_generation_stack:           8
                     play_move:           8
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



disable optimisations in Danger Circe

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -R  -S  -B  -Q  -K  -B  -S  -R   8
|                                   |
7  -P  -P  -P   .  -P  -P  -P  -P   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -P   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   P   P   P   P   P   P   P   P   2
|                                   |
1   R   S   B   Q   K   B   S   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  dia1.5                    16 + 16
             DangerCirce

  1.Sb1-c3 d7-d5   2.Sc3-b1 dia
  1.Sb1-a3 d7-d5   2.Sa3-b1 dia
  1.Sg1-h3 d7-d5   2.Sh3-g1 dia
  1.Sg1-f3 d7-d5   2.Sf3-g1 dia
  1.c2-c4 d7-d5   2.c4-c2 dia
  1.e2-e4 d7-d5   2.e4-e2 dia
  1.g2-g4 d7-d5   2.g4-g2 dia
  1.h2-h3 d7-d5   2.h3-h2 dia
  add_to_move_generation_stack:        3442
                     play_move:        1337
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.


