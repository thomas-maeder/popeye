
king is transfered when castling, rook isn't

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
1   .   .   .   .   K   .   .   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  001                         2 + 0
        Wormholes f1 g1 d4 h5

   1.0-0[wKg1->d4] !

   1.0-0[wKg1->h5] !

  add_to_move_generation_stack:          15
                     play_move:           2
 is_white_king_square_attacked:           4
 is_black_king_square_attacked:           0

solution finished.



pawns are promoted before and after transfer

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   P   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   P   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          2 + 0
         Wormholes d1 a4 f8

   1.a3-a4[wPa4->d1] !

   1.a3-a4[wPa4->f8=Q] !

   1.a3-a4[wPa4->f8=S] !

   1.a3-a4[wPa4->f8=R] !

   1.a3-a4[wPa4->f8=B] !

   1.f7-f8=Q[wQf8->d1] !

   1.f7-f8=Q[wQf8->a4] !

   1.f7-f8=S[wSf8->d1] !

   1.f7-f8=S[wSf8->a4] !

   1.f7-f8=R[wRf8->d1] !

   1.f7-f8=R[wRf8->a4] !

   1.f7-f8=B[wBf8->d1] !

   1.f7-f8=B[wBf8->a4] !

  add_to_move_generation_stack:           2
                     play_move:          13
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



neutral pawns are not promoted when transfered to the "wrong" promotion row

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
3   .   .   .  =P   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                     0 + 0 + 1n
           Wormholes f1 d4

   1.nPd3-d4[nPd4->f1] !

  add_to_move_generation_stack:           1
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



pawns in wormholes on 1st rank can do single step

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -P   .   .   .   .  -P   .   .   8
|                                   |
7   .   .   .   .   P   .   .   .   7
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
1   .   .   .   P   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hzd21                       2 + 2
         Wormholes d1 d4 f8

  1.f8-f7 d1-d2 z
  1.f8*e7 d1-d2 z
  add_to_move_generation_stack:           5
                     play_move:           8
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



 castling after transfer to wormhole

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   K   .   .   .   7
|                                   |
6   .   .   .   .   R   .   .   .   6
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
  ser-003                     2 + 0
         Wormholes e1 h1 f8

  1.Re6-e1[wRe1->h1]   2.Ke7-f8[wKf8->e1]   3.0-0
  add_to_move_generation_stack:         477
                     play_move:         492
 is_white_king_square_attacked:         494
 is_black_king_square_attacked:           0

solution finished.



at least 1 wormhole must be empty for check on a wormhole ...

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .  -K   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   P   .   .   .   .   5
|                                   |
4   .   .   .   P   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ser-+2                      2 + 1
           Wormholes d4 e7

  1.d5-d6   2.d4-d5 +
  add_to_move_generation_stack:           5
                     play_move:           4
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           7

solution finished.



unless the checking piece occupies a wormhole itself

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .  -K   .   .   .   8
|                                   |
7   .   .   .  -P   .   .   .   .   7
|                                   |
6   .   .   P   .   .   .   .   .   6
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
  +1                          1 + 2
                Circe
         Wormholes d7 f7 e8

   1.c6*d7[wPd7->f7][+bPd7] + !

  add_to_move_generation_stack:           2
                     play_move:           2
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           3

solution finished.



paralysis on wormhole even if all wormholes are blocked

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K  -S   .   .   .   .   .   .   8
|                                   |
7   .   .   .   S   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   K   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   Q   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #1                          3 + 2
               Madrasi
           Wormholes h4 b8

   1.Qf1-a6 # !

  add_to_move_generation_stack:          36
                     play_move:          35
 is_white_king_square_attacked:           5
 is_black_king_square_attacked:          36

solution finished.



support on wormhole even if all wormholes are blocked

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   S   .   .   .   .   7
|                                   |
6   Q   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   K   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #1                          3 + 1
             Patrouille
           Wormholes h4 a6

   1.Sd7-b8 # !

  add_to_move_generation_stack:          41
                     play_move:          37
 is_white_king_square_attacked:           3
 is_black_king_square_attacked:          38

solution finished.



the length of a move is determined by the wormhole a pices moves on

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
1   B   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 0
           WhiteMaximummer
           Wormholes b1 h8

   1.Ba1-h8[wBh8->b1] !

  add_to_move_generation_stack:           7
                     play_move:           8
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



en passant capture of pawn transfered to wormhole

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .  -P   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   P   .   .   .  -K   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   B   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h+1                         2 + 2
           BlackMaximummer
           Wormholes f3 e5

  1.e7-e5[bPe5->f3] d5*e6 ep. +
  add_to_move_generation_stack:          14
                     play_move:          10
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:          10

solution finished.



       ep capture to wormhole

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .  -P   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   P   .   .   .   .   5
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
  hep1                        1 + 1
           Wormholes h4 e6

  1  (e7-e5 )
  1.e7-e5 d5*e6 ep.[wPe6->h4]
  2  (e7-e6[bPe6->h4] )
  add_to_move_generation_stack:           4
                     play_move:           3
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



1.e2-e4[wBe4->e2] would be an illegal null move; 1.e2*f3[wBf3->e2] isn't

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
3   .   .   .   .   .  -P   .   .   3
|                                   |
2   .   .   .   .   P   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 1
         Wormholes e2 f3 e4

   1.e2-e3 !

   1.e2*f3[wPf3->e2] !

   1.e2*f3[wPf3->e4] !

  add_to_move_generation_stack:           3
                     play_move:           3
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



promotions are not illegal null moves

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   P   .   .   .   7
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
  ~1                          1 + 0
           Wormholes e7 e8

   1.e7-e8=Q[wQe8->e7] !

   1.e7-e8=S[wSe8->e7] !

   1.e7-e8=R[wRe8->e7] !

   1.e7-e8=B[wBe8->e7] !

  add_to_move_generation_stack:           1
                     play_move:           4
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



  4.63 optimised this solution away

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
4   P   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ze71                        1 + 0
           Wormholes a5 e7

   1.a4-a5[wPa5->e7] z !

  add_to_move_generation_stack:           1
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.


