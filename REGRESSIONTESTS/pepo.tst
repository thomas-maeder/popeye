
double check is mate, single checks are not

+---a---b---c---d---e---f---g---h---+
|                                   |
8   R   B   .   .   .   .   .  -K   8
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
  #1                          2 + 1
                Pepo

  1  (Bb8-a7 )
  2  (Bb8-h2 )
  3  (Bb8-g3 )
  4  (Bb8-f4 )
  5  (Bb8-e5 + )
   1.Bb8-e5 # !

  6  (Bb8-d6 )
  7  (Bb8-c7 )
  add_to_move_generation_stack:           7
                     play_move:           7
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:          15

solution finished.



           Petko A. Petkov
         KobulChess.com 2019

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   B   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   R   .   P   .   .   5
|                                   |
4   .   .  =P   .  -K   .   .   .   4
|                                   |
3   .   .  -P   .   .   R  -P   .   3
|                                   |
2   .   .   P   .   .   .   P   K   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                     7 + 3 + 1n
                Pepo

   1.Rf3-f1 ? zugzwang.
      1...Ke4*d5
          2.Bh7-g8 #
          2.Rf1-d1 #
      1...Ke4-d4
          2.Rf1-d1 #
          2.Rf1-f4 #
      1...Ke4-e5
          2.Rf1-e1 #
      1...Ke4-f4
          2.Rd5-d4 #
    but
      1...Ke4-e3 !

   1.Rf3*c3 ? zugzwang.
      1...Ke4*d5
          2.Bh7-g8 #
      1...Ke4-e3
          2.Rd5-e5 #
      1...Ke4-d4
          2.Rc3*c4 #
      1...Ke4-e5
          2.Rc3-e3 #
    but
      1...Ke4-f4 !

   1.Rf3*g3 ? zugzwang.
      1...Ke4*d5
          2.Bh7-g8 #
      1...Ke4-e3
          2.Rd5-e5 #
      1...Ke4-d4
          2.Rg3-g4 #
      1...Ke4-e5
          2.Rg3-e3 #
    but
      1...Ke4-f4 !

   1.Rd5-d1 ? zugzwang.
      1...Ke4*f3
          2.Rd1-f1 #
      1...Ke4-e3
          2.Rd1-e1 #
      1...Ke4-d4
          2.Rf3-f4 #
      1...Ke4-f4
          2.Rd1-d4 #
    but
      1...Ke4-e5 !

   1.Bh7-g6 ! zugzwang.
      1...Ke4*f3
          2.Bg6-h5 #
      1...Ke4*d5
          2.Bg6-f7 #
      1...Ke4-e3
          2.Rd5-e5 #
      1...Ke4-d4
          2.Rf3-f4 #
      1...Ke4-e5
          2.Rf3-e3 #
      1...Ke4-f4
          2.Rd5-d4 #

   1.Bh7-g8 ? zugzwang.
      1...Ke4-e3
          2.Rd5-e5 #
      1...Ke4-d4
          2.Rf3-f4 #
      1...Ke4-e5
          2.Rf3-e3 #
      1...Ke4-f4
          2.Rd5-d4 #
    but
      1...Ke4*f3 !

  add_to_move_generation_stack:         597
                     play_move:         536
 is_white_king_square_attacked:         150
 is_black_king_square_attacked:         402

solution finished.


