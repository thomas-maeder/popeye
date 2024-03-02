
           Petko A. Petkov
       ANDA Fairy Planet, 2020

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   G   .   .   8
|                                   |
7   .  -P  -P   .   S   .   .   .   7
|                                   |
6   .   .   P  -K   .   .   S  -B   6
|                                   |
5   .   .  -P   G  -P   P   .   .   5
|                                   |
4   S   .  -P   .  -P   .   .   .   4
|                                   |
3   .   .   K   .   .   .   .   .   3
|                                   |
2   .   .   .   Q   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #1                          9 + 8
              Bul f8 d5

   1.bGd5-b3[bPc4->f4] # !

   1.bGd5-f3[bPe4->g2] # !

   1.bGd5*b7[wPc6->c8=S] # !

   1.bGd5-d1[wQd2->d7] # !

   1.bGd5-b5[bPc5->a5] # !

   1.bGd5-d7[bKd6->d1] # !

  add_to_move_generation_stack:         408
                     play_move:         550
 is_white_king_square_attacked:          30
 is_black_king_square_attacked:         303

solution finished.



           Petko A. Petkov
       ANDA Fairy Planet 2020

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   Q   .   .   8
|                                   |
7  -B  -P   .  -P  =G  -P   .  -P   7
|                                   |
6   .  -P   .   .   .   .   .  -R   6
|                                   |
5   .   .   .   .  -K   .   .   S   5
|                                   |
4   .   .   .   .   .   .   .   S   4
|                                   |
3   .   B   .   .   .   .   .   .   3
|                                   |
2   .   P   P   R   .   .   .   .   2
|                                   |
1   R   .   .  -B   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                     9 + 9 + 1n
               Bul e7

   1.Ra1-a2 ! threat:
          2.nbGe7-e4[bKe5->a1] #
      1...Ke5-e4
          2.nbGe7-e3[bKe4->a8] #
      1...nbGe7-e4[bKe5->e3]
          2.nbGe4-e2[bKe3->a3] #
      1...nbGe7-c7[bPd7->g7]
          2.nbGc7-f4[bKe5->h8] #
      1...nbGe7-g7[bPf7->c7]
          2.nbGg7-d4[bKe5->b8] #

   1.Rd2-d5 + ?
      1...Ke5-e6
          2.nbGe7-e5[bKe6->c4] #
      1...nbGe7-e4[bKe5->e3]
          2.nbGe4-e2[bKe3->a3] #
    but
      1...Ke5-e4 !

   1.Qf8*e7 + ?
    but
      1...Rh6-e6 !

  add_to_move_generation_stack:       26271
                     play_move:       24044
 is_white_king_square_attacked:        1676
 is_black_king_square_attacked:       15635

solution finished.



           Petko A. Petkov
       ANDA Fairy Planet 2020

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7  -P   .   .   .   .   .   .   .   7
|                                   |
6   .   .  -P   .  -P   G  -P   .   6
|                                   |
5  -P   .   K   P   .   .   .   .   5
|                                   |
4   .   .   .   P   .   .   .   .   4
|                                   |
3  -P   .   .   .   .   .   .   .   3
|                                   |
2   P   .   Q  -P   .   .   .   .   2
|                                   |
1  -K   B   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                          7 + 8
               Bul f6

   1.bGf6-c3[wPd4->b2] ! zugzwang.
      1...a3*b2
          2.Qc2-d1 #
      1...a5-a4
          2.Kc5-b4 #
      1...c6*d5
          2.Kc5-b5 #
      1...e6*d5
          2.Kc5-d4 #
      1...g6-g5
          2.bGc3-e1[bPd2->h6] #
      1...a7-a6
          2.Kc5-b6 #

  add_to_move_generation_stack:        3257
                     play_move:        2658
 is_white_king_square_attacked:         303
 is_black_king_square_attacked:        1824

solution finished.



           Petko A. Petkov
       ANDA Fairy Planet 2020

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .  -P   .   .   .   .   .   .   7
|                                   |
6   .   .   .  -P   .   B   .   .   6
|                                   |
5   .  -P   .   .   .   .   .   .   5
|                                   |
4   .  -P   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   K   .  -G   3
|                                   |
2   P   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                          3 + 5
              Royal h3
               Bul h3

   1.Kf3-g2 + ?
    but
      1...rbGh3-f1[wKg2->a8] !

   1.Kf3-g4 + ?
    but
      1...rbGh3-f5[wKg4->a4] + !

   1.Kf3-f2 ? zugzwang.
      1...d6-d5
          2.Kf2-g3 #
      1...b7-b6
          2.Kf2-g2 #
    but
      1...b4-b3 !

   1.Kf3-g3 + ?
    but
      1...rbGh3-f3[wKg3->c7] !

   1.Bf6-b2 ! zugzwang.
      1...rbGh3-e3[wKf3->a8]
          2.Bb2-c1 #
      1...b4-b3
          2.Kf3-g4 #
      1...d6-d5
          2.Kf3-g3 #
      1...b7-b6
          2.Kf3-g2 #

   1.Bf6-c3 ? zugzwang.
      1...b4-b3
          2.Kf3-g4 #
      1...b4*c3
          2.Kf3-g4 #
      1...d6-d5
          2.Kf3-g3 #
      1...b7-b6
          2.Kf3-g2 #
    but
      1...rbGh3-e3[wKf3->a8] !

   1.Bf6-d4 ? zugzwang.
      1...d6-d5
          2.Kf3-g3 #
      1...b7-b6
          2.Kf3-g2 #
    but
      1...b4-b3 !

   1.Bf6-d8 ? zugzwang.
      1...rbGh3-e3[wKf3->a8]
          2.Bd8-b6 #
      1...d6-d5
          2.Kf3-g3 #
      1...b7-b6
          2.Kf3-g2 #
    but
      1...b4-b3 !

  add_to_move_generation_stack:        2114
                     play_move:        2034
 is_white_king_square_attacked:         192
 is_black_king_square_attacked:        1627

solution finished.



           Petko A. Petkov
       ANDA Fairy Planet 2020

+---a---b---c---d---e---f---g---h---+
|                                   |
8   Q   .   .   .   .   .   .   .   8
|                                   |
7   P   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .  -P   P   .   6
|                                   |
5   .   .   B  LI   .   .   .   .   5
|                                   |
4   .   .  -P   .  -K   .   .   .   4
|                                   |
3   .   .  -P   .   .   .  -P   .   3
|                                   |
2   .   .   .   .   .   .   S   .   2
|                                   |
1   .   .   .   .   .   .   K   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                          7 + 5
               Bul d5

   1.bLId5-b5[wBc5->a5] + ?
      1...Ke4-f5
          2.bLIb5-g5[bKf5->f8] #
      1...Ke4-e5
          2.bLIb5-g5[bKe5->h8] #
    but
      1...Ke4-d4 !

   1.Qa8-c6 ! threat:
          2.bLId5-f3[bKe4->a8] #
      1...c3-c2
          2.bLId5-f3[bKe4->a8] #
      1...Ke4-d3
          2.bLId5-d2[bKd3->a6] #
      1...Ke4-f3
          2.bLId5-b5[wBc5->c8] #
      1...Ke4-f5
          2.bLId5-g5[bKf5->f8] #
      1...Ke4-e5
          2.bLId5-f5[bKe5->a5] #

  add_to_move_generation_stack:       20651
                     play_move:       13367
 is_white_king_square_attacked:        1853
 is_black_king_square_attacked:        9795

solution finished.



           Petko A. Petkov
       ANDA Fairy Planet 2020

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   Q   .   .   B   .   .  -B   7
|                                   |
6   .   .  NA   P  -P   .  -P   .   6
|                                   |
5   P   .   P   .  -K   .  -P   .   5
|                                   |
4   .  NA   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   P   .   3
|                                   |
2   .   .   .   .  -P   .   .   .   2
|                                   |
1   .   B   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                         10 + 6
              Bul c6 b4

   1.a5-a6 ! zugzwang.
      1...Ke5-d4
          2.bNAc6*e2[bKd4->b8] #
      1...Ke5-d5
          2.bNAb4*h7[bKd5->c7] #
      1...g5-g4
          2.bNAc6*g4[bKe5->a7] #
      1...Bh7-g8
          2.bNAc6*g8[wBe7->c3] #

  add_to_move_generation_stack:        5226
                     play_move:        3932
 is_white_king_square_attacked:         380
 is_black_king_square_attacked:        3084

solution finished.



           Petko A. Petkov
       ANDA Fairy Planet 2020

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .  -R   .   .   .  -K   8
|                                   |
7   .   .   .   .  =S   .   .  -P   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .  -R   .   .   . =CA   4
|                                   |
3   .   .   .   .   .   .   .   K   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   . =LE   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                    1 + 4 + 3n
               Bul h1

  1.Rd4-a4 nSe7-d5   2.Ra4-a8 nbLEh1*a8[nSd5->f7] #
  1.Rd8-a8 nCAh4-g1   2.Ra8-a1 nbLEh1*a1[nCAg1->g5] #
  add_to_move_generation_stack:     4980209
                     play_move:     1635173
 is_white_king_square_attacked:      123510
 is_black_king_square_attacked:     1284433

solution finished.


