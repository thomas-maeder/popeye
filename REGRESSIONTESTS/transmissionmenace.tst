
basic move generation: move like observer, avoid duplicate moves

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
4   .   .   P   .   .   .   .  -R   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 1
         TransmissionMenace

   1.c4-c1 !

   1.c4-c2 !

   1.c4-c3 !

   1.c4-a4 !

   1.c4-b4 !

   1.c4-c8=Q !

   1.c4-c8=S !

   1.c4-c8=R !

   1.c4-c8=B !

   1.c4-c7 !

   1.c4-c6 !

   1.c4*h4 !

   1.c4-g4 !

   1.c4-f4 !

   1.c4-e4 !

   1.c4-d4 !

   1.c4-c5 !

  add_to_move_generation_stack:          15
                     play_move:          17
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



           test for check

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .  -K   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   P   .   .   .   .   .   4
|                                   |
3   .  -S   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  +1                          1 + 2
         TransmissionMenace

   1.c4-c5 + !

  add_to_move_generation_stack:          11
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.


