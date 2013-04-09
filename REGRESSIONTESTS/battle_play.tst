
         #1 - Black in check

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .   .   .   .   .   8
|                                   |
7   .   .   S   .   .   .   .   .   7
|                                   |
6   K   .   .   .   .   .   .   .   6
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

the side to play can capture the king
                        empile:           0
                     play_move:           0
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           1
solution finished. 



                  #1

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .   S   .   .   .   8
|                                   |
7   P   .   .   .   .   .   .   .   7
|                                   |
6   K   .   .   .   .   .   .   .   6
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
  #1                          3 + 1

   1.Se8-c7 # !


                        empile:           4
                     play_move:           4
 is_white_king_square_attacked:           1
 is_black_king_square_attacked:           5
solution finished. 



        option try in 1movers

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   B   .   .   .   .   .   .   .   6
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
  zc81                        1 + 0

   1.Ba6-c8 z !


                        empile:           7
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



                 #1.5

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .  -S   .   .   .   8
|                                   |
7   P   .   .   .   .   .   .   .   7
|                                   |
6   .   K   .   .   .   .   .   .   6
|                                   |
5   .   .   .   S   .   .   .   .   5
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
  #2                          3 + 2

 zugzwang.
      1...Se8-c7
          2.Sd5*c7 #
      1...Se8-d6
          2.Sd5-c7 #
      1...Se8-f6
          2.Sd5-c7 #
      1...Se8-g7
          2.Sd5-c7 #

                        empile:          51
                     play_move:          48
 is_white_king_square_attacked:          10
 is_black_king_square_attacked:          50
solution finished. 



           #1.5 incomplete

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .  -S   .   .   .   8
|                                   |
7   P   .   .   .   .   .   .   .   7
|                                   |
6   .   K   .   .   .   .   .   .   6
|                                   |
5   .   .   .   S   .   .   .  -P   5
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
  #2                          3 + 3

 zugzwang.
      1...h5-h4
        refutes.
      1...Se8-c7
          2.Sd5*c7 #
      1...Se8-d6
          2.Sd5-c7 #
      1...Se8-f6
          2.Sd5-c7 #
      1...Se8-g7
          2.Sd5-c7 #

                        empile:          64
                     play_move:          58
 is_white_king_square_attacked:          12
 is_black_king_square_attacked:          60
solution finished. 



        #2 with short solution

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .   S   .   .   .   8
|                                   |
7   P   .   .   .   .   .   .   .   7
|                                   |
6   K   .   .   .   .   .   .  -P   6
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
  #2                          3 + 2

   1.Se8-c7 # !

   1.Ka6-b6 ! threat:
          2.Se8-c7 #


                        empile:          63
                     play_move:          46
 is_white_king_square_attacked:          13
 is_black_king_square_attacked:          49
solution finished. 



##!2 with tries (1.Ra6/7?) that only fail because the prerequisite
  (White has to be mate) is not met

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .  -B   8
|                                   |
7   .   .   .   .   .   .   P   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   R   .   .   .   .   .   .   .   5
|                                   |
4  -S  -P   .   .   .   .   .   .   4
|                                   |
3  -K   P   .   .   .   .   .   .   3
|                                   |
2   P   .   .   .   .   .   .   .   2
|                                   |
1   K   B   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ##!2                        6 + 4

   1.Ra5-a8 ! zugzwang.
      1...Bh8*g7 +
          2.Ra8*a4 ##!


                        empile:         751
                     play_move:         514
 is_white_king_square_attacked:         437
 is_black_king_square_attacked:         106
solution finished. 



       ##!2 with short solution

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   R   K   .   .   .   .   .   8
|                                   |
7  -K   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .  -Q  -R   .   .   .   .   5
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
  ##!2                        2 + 3

   1.Kc8-b7 ##! !


                        empile:         246
                     play_move:         237
 is_white_king_square_attacked:         212
 is_black_king_square_attacked:          45
solution finished. 



       #=2 with short solution

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
3   K   .   R   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1  -K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #=2                         2 + 1

   1.Rc3-c1 #= !

   1.Rc3-b3 #= !


                        empile:        1561
                     play_move:         728
 is_white_king_square_attacked:         295
 is_black_king_square_attacked:         448
solution finished. 



          #2 with variation

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .   .   .   .   .   8
|                                   |
7   P   .  -P   .   .   .   .   .   7
|                                   |
6   .   K   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   S   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                          3 + 2

   1.Kb6-a6 ! zugzwang.
      1...c7-c5
          2.Sa4-b6 #
      1...c7-c6
          2.Sa4-b6 #


                        empile:          67
                     play_move:          53
 is_white_king_square_attacked:          17
 is_black_king_square_attacked:          56
solution finished. 



           #2 with setplay

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .  -K   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   K   .   R   .   .   .   .   .   6
|                                   |
5   .   .  -P   .   .   .   .   .   5
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
  #2                          2 + 2

      1...Kb8-a8
          2.Rc6-c8 #

   1.Rc6*c5 ! zugzwang.
      1...Kb8-a8
          2.Rc5-c8 #


                        empile:         268
                     play_move:         131
 is_white_king_square_attacked:          46
 is_black_king_square_attacked:         133
solution finished. 



   #2 with setplay, White in check

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .  -K   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   K   .   R   .   .   .   .   .   6
|                                   |
5   .   .  -S   .   .   .   .   .   5
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
  #2                          2 + 2

the side to play can capture the king
   1.Rc6*c5 ! zugzwang.
      1...Kb8-a8
          2.Rc5-c8 #


                        empile:         108
                     play_move:          48
 is_white_king_square_attacked:          25
 is_black_king_square_attacked:          51
solution finished. 



                 #2.5

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .  -S   .   .   .   8
|                                   |
7   P   .   .   .   .   .   .   .   7
|                                   |
6   K   .  -P   .   S   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   P   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #3                          4 + 3

 zugzwang.
      1...c6-c5
          2.c3-c4 zugzwang.
              2...Se8-c7 +
                  3.Se6*c7 #
              2...Se8-d6
                  3.Se6-c7 #
              2...Se8-f6
                  3.Se6-c7 #
              2...Se8-g7
                  3.Se6-c7 #
      1...Se8-c7 +
          2.Se6*c7 #
      1...Se8-d6
          2.Se6-c7 #
      1...Se8-f6
          2.Se6-c7 #
      1...Se8-g7
          2.Se6-c7 #

                        empile:         408
                     play_move:         242
 is_white_king_square_attacked:          59
 is_black_king_square_attacked:         220
solution finished. 



        #3 with short setplay

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -K   .   .   .  -S   .   .   .   8
|                                   |
7   P   .   .   .   .   .   .   .   7
|                                   |
6   K   .  -P   .   S   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .  -P   .   .   .   .   .   3
|                                   |
2   .   P   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #3                          4 + 4

      1...Se8-c7 +
          2.Se6*c7 #
      1...Se8-d6
          2.Se6-c7 #
      1...Se8-f6
          2.Se6-c7 #
      1...Se8-g7
          2.Se6-c7 #

   1.b2*c3 ! zugzwang.
      1...c6-c5
          2.c3-c4 zugzwang.
              2...Se8-c7 +
                  3.Se6*c7 #
              2...Se8-d6
                  3.Se6-c7 #
              2...Se8-f6
                  3.Se6-c7 #
              2...Se8-g7
                  3.Se6-c7 #
      1...Se8-c7 +
          2.Se6*c7 #
      1...Se8-d6
          2.Se6-c7 #
      1...Se8-f6
          2.Se6-c7 #
      1...Se8-g7
          2.Se6-c7 #


                        empile:        3605
                     play_move:        2482
 is_white_king_square_attacked:        1420
 is_black_king_square_attacked:        1174
solution finished. 



                s#0.5

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
4   .   .   .   .   .   .   .   R   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#1                         2 + 3

      1...b3-b2 #

                        empile:          21
                     play_move:          21
 is_white_king_square_attacked:          22
 is_black_king_square_attacked:           5
solution finished. 



                 s#1

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
3  -K  -P   .   .   .   .   .   R   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#1                         2 + 3

   1.Rh3-h4 !
      1...b3-b2 #


                        empile:         106
                     play_move:          70
 is_white_king_square_attacked:          70
 is_black_king_square_attacked:          22
solution finished. 



s#1 with incomplete set play (not given)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .  -P   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   R   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#1                         2 + 4


   1.Rc3-c4 !
      1...b3-b2 #


                        empile:         104
                     play_move:          68
 is_white_king_square_attacked:          66
 is_black_king_square_attacked:          26
solution finished. 



      s#1 with complete set play

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .  -P   .   .   .   .   .   5
|                                   |
4   .   .   R   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   P   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#1                         3 + 4

      1...b3-b2 #

   1.h3-h4 !
      1...b3-b2 #


                        empile:         160
                     play_move:         119
 is_white_king_square_attacked:         120
 is_black_king_square_attacked:          34
solution finished. 



            s#1 with tries

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
4   .   .   R   .   P   .   .   .   4
|                                   |
3  -K  -P   .   B   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#1                         4 + 2

   1.e4-e5 ?
    but
      1...b3-b2 + !

   1.Rc4-a4 + ?
    but
      1...Ka3*a4 !

   1.Rc4-d4 ?
    but
      1...b3-b2 + !

   1.Bd3-b1 !
      1...b3-b2 #

   1.Bd3-f1 ?
    but
      1...b3-b2 + !

   1.Bd3-e2 ?
    but
      1...b3-b2 + !

   1.Ka1-b1 ?
    but
      1...b3-b2 !


                        empile:         233
                     play_move:         190
 is_white_king_square_attacked:         166
 is_black_king_square_attacked:         118
solution finished. 



                s##!1

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .  -R  -K -DU   .   .   .   .   8
|                                   |
7   K   .   . -DU   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   Q   .   .   .   .   .   .   .   5
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
  s##!1                       2 + 4

   1.Qa5-c5 + !
      1...Kc8-b7 ##!


                        empile:         561
                     play_move:         542
 is_white_king_square_attacked:         105
 is_black_king_square_attacked:         505
solution finished. 



        sep1 (self-en passant)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   B   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .  -P   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   P   .  -P   2
|                                   |
1   .   .   .   .   .   K   .  -K   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  sep1                        3 + 3

   1.f2-f4 !
      1...e4*f3 ep.


                        empile:          18
                     play_move:          18
 is_white_king_square_attacked:          11
 is_black_king_square_attacked:          10
solution finished. 



                s#1.5

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
4   .  -P   .   .   .   .   .   .   4
|                                   |
3  -K   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   Q   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#2                         2 + 3

 zugzwang.
      1...b4-b3
          2.Qd1-g4
              2...b3-b2 #

                        empile:         255
                     play_move:         165
 is_white_king_square_attacked:         167
 is_black_king_square_attacked:          68
solution finished. 



                 s#2

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
4   .  -P   .   .   .   .   Q   .   4
|                                   |
3  -K   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#2                         2 + 3

   1.Qg4-d1 ! zugzwang.
      1...b4-b3
          2.Qd1-g4
              2...b3-b2 #


                        empile:        3013
                     play_move:        1440
 is_white_king_square_attacked:        1441
 is_black_king_square_attacked:         770
solution finished. 



       s#2 with short solution

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .  -P   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   R   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#2                         2 + 4

   1.Rc3-c4 !
      1...b3-b2 #


                        empile:         753
                     play_move:         340
 is_white_king_square_attacked:         340
 is_black_king_square_attacked:         160
solution finished. 



       s#=2 with short solution

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
3  -P  -K  -P   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   R   2
|                                   |
1   K  CA   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#=2                        3 + 3

   1.Rh2-b2 + !
      1...a3*b2 #=
      1...c3*b2 #=


                        empile:        2835
                     play_move:        1193
 is_white_king_square_attacked:         788
 is_black_king_square_attacked:         697
solution finished. 



          s#2 with set play

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
4   .  -P   .   .   .   .   .   .   4
|                                   |
3  -K   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   Q   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#2                         2 + 3

      1...b4-b3
          2.Qc2-e4
              2...b3-b2 #
          2.Qc2-c4
              2...b3-b2 #

   1.Qc2-d1 ! zugzwang.
      1...b4-b3
          2.Qd1-g4
              2...b3-b2 #


                        empile:        3275
                     play_move:        1577
 is_white_king_square_attacked:        1580
 is_black_king_square_attacked:         780
solution finished. 



   s#2 with complete short set play

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .  -P   .   .   .  -P   .   5
|                                   |
4   .   .   R   .   .   .   P   .   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#2                         3 + 5

      1...b3-b2 #

   1.Rc4-e4 !
      1...c5-c4
          2.Re4*c4
              2...b3-b2 #


                        empile:         672
                     play_move:         419
 is_white_king_square_attacked:         421
 is_black_king_square_attacked:         146
solution finished. 



   exact s#2 with refutation by s#1

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
4  -P   .   .   .   .   .   .   .   4
|                                   |
3   .  -P   P   .   .   .   .   .   3
|                                   |
2  -P   .  -K   .   P   .   .   .   2
|                                   |
1   K  -B   .   S   Q   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  exact-s#2                   5 + 5

   1.Qe1-d2 + ?
    but
      1...Kc2*d2 !

   1.Sd1-f2 ? zugzwang.
      1...a4-a3
          2.c3-c4
              2...b3-b2 #
          2.Sf2-h1
              2...b3-b2 #
          2.Sf2-h3
              2...b3-b2 #
          2.Sf2-g4
              2...b3-b2 #
          2.Sf2-e4
              2...b3-b2 #
          2.e2-e4
              2...b3-b2 #
          2.e2-e3
              2...b3-b2 #
    but
      1...b3-b2 # !

   1.Sd1-b2 ! zugzwang.
      1...a4-a3
          2.c3-c4
              2...a3*b2 #
          2.e2-e4
              2...a3*b2 #
          2.e2-e3
              2...a3*b2 #


                        empile:        3901
                     play_move:        2085
 is_white_king_square_attacked:        2072
 is_black_king_square_attacked:         985
solution finished. 



                r#0.5

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
3  -K   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .  -S   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#1                         1 + 3

      1...Se1-c2 #

                        empile:           4
                     play_move:           4
 is_white_king_square_attacked:           5
 is_black_king_square_attacked:           2
solution finished. 



                 r#1

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
3  -K   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   .   K   .   .  -S   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#1                         1 + 3

   1.Kb1-a1 !
      1...Se1-c2 #


                        empile:          30
                     play_move:          29
 is_white_king_square_attacked:          29
 is_black_king_square_attacked:           7
solution finished. 



          r#1 with set play

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
3  -K   .   .   .   .   .   .   P   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .  -S   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#1                         2 + 3

      1...Se1-c2 #

   1.h3-h4 !
      1...Se1-c2 #


                        empile:          19
                     play_move:          19
 is_white_king_square_attacked:          20
 is_black_king_square_attacked:           5
solution finished. 



   r#1, but White must deliver mate

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   B   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -P   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#1                         2 + 4

   1.Bg7-f8 #

                        empile:           7
                     play_move:           6
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:           7
solution finished. 



                r#1.5

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
4  -P  -P   .   .   .   .   .   .   4
|                                   |
3  -K   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   S   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#2                         2 + 4

 zugzwang.
      1...b4-b3
          2.Sc1-e2
              2...b3-b2 #

                        empile:          52
                     play_move:          44
 is_white_king_square_attacked:          43
 is_black_king_square_attacked:          10
solution finished. 



      r#1.5 with immediate mate

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
4  -P  -P   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   P   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#2                         2 + 6

      1...b3-b2 #

                        empile:           5
                     play_move:           5
 is_white_king_square_attacked:           6
 is_black_king_square_attacked:           2
solution finished. 



                 r#2

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   B   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -P  -P   .   .   .   .   .   .   4
|                                   |
3  -K   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#2                         2 + 3

   1.Bh7-c2 ! zugzwang.
      1...b4-b3
          2.Bc2-b1
              2...b3-b2 #


                        empile:         307
                     play_move:         201
 is_white_king_square_attacked:         174
 is_black_king_square_attacked:          56
solution finished. 



r#2 with short set play and short solution

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .  -P   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -P  -P   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   P   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#2                         2 + 6

      1...b3-b2 #

   1.h3-h4 !
      1...b3-b2 #


                        empile:          24
                     play_move:          24
 is_white_king_square_attacked:          25
 is_black_king_square_attacked:           6
solution finished. 



exact r#2 that would have a solution in 1

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .  -K   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   P   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   R   .  -P   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  exact-r#2                   3 + 2

   1.Ra2-a8 + !
      1...Kh8-h7
          2.Ra8-a2
              2...c2-c1=Q #


                        empile:         849
                     play_move:         610
 is_white_king_square_attacked:         564
 is_black_king_square_attacked:         175
solution finished. 



r#2, but White must deliver mate immediately

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   B   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -P   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#2                         2 + 4

   1.Bg7-f8 #

                        empile:           7
                     play_move:           6
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:           7
solution finished. 



                r#2.5

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   S   .   .   .   .   .   .   .   6
|                                   |
5  -P   .   .   .   .   .   .   .   5
|                                   |
4  -P   P   .   .   .   .   .   .   4
|                                   |
3  -K   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   B   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#3                         4 + 3

 zugzwang.
      1...a5*b4
          2.Bd1-c2 zugzwang.
              2...b4-b3
                  3.Bc2-b1
                      3...b3-b2 #

                        empile:        1038
                     play_move:         680
 is_white_king_square_attacked:         601
 is_black_king_square_attacked:         182
solution finished. 



                 r#3

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   S   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5  -P   .   .   .   .   .   .   .   5
|                                   |
4  -P   P   .   .   .   .   .   .   4
|                                   |
3  -K   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   B   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#3                         4 + 3

  1  (Sd8-b7 )
  2  (Sd8-c6 )
   1.Sd8-c6 ! zugzwang.
      1...a5*b4
          2.Bd1-c2 zugzwang.
              2...b4-b3
                  3.Bc2-b1
                      3...b3-b2 #

  3  (Sd8-e6 )
  4  (Sd8-f7 )
  5  (b4-b5 )
  6  (b4*a5 )
  7  (Bd1-h5 )
  8  (Bd1-g4 )
  9  (Bd1-f3 )
 10  (Bd1-e2 )
 11  (Bd1*a4 )
 12  (Bd1-b3 )
 13  (Bd1-c2 )
 14  (Ka1-b1 )
 15  (Ka1-a2 + )
 16  (Ka1-b2 + )

                        empile:        9415
                     play_move:        6012
 is_white_king_square_attacked:        5347
 is_black_king_square_attacked:        1272
solution finished. 



r#3, but White must deliver mate immediately

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   B   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -P   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#3                         2 + 4

   1.Bg7-f8 #

                        empile:           7
                     play_move:           6
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:           7
solution finished. 



a r#2 solution does not solve an exact r#3

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
3   .   .  -P   .   .   .   .   .   3
|                                   |
2   P   .   .   .   .   .   .   .   2
|                                   |
1   .   .   K   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  exact-r#3                   2 + 1


                        empile:        1104
                     play_move:         482
 is_white_king_square_attacked:         465
 is_black_king_square_attacked:           0
solution finished. 



r#2 - there is no threat, because White at the move
would have to deliver mate (issue 2843251)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .  -P   .   7
|                                   |
6   .   .   .   .   .   .  -P  -K   6
|                                   |
5   .   .   .   .   .   .  -P   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .  -P   .   .   .   .   .   .   3
|                                   |
2   .   P   .  -P   .   .   .   .   2
|                                   |
1   .   .   K   R   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#2                         3 + 6

   1.Kc1-b1 ! zugzwang.
      1...g5-g4
          2.Rd1-e1
              2...d2*e1=Q #
              2...d2*e1=R #
      1...Kh6-h7
          2.Rd1-e1
              2...d2*e1=Q #
              2...d2*e1=R #
      1...Kh6-h5
          2.Rd1-e1
              2...d2*e1=Q #
              2...d2*e1=R #


                        empile:         484
                     play_move:         343
 is_white_king_square_attacked:         307
 is_black_king_square_attacked:          94
solution finished. 



              semi-r#0.5

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
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  semi-r#1                    1 + 3

      1...b3-b2 #

                        empile:           4
                     play_move:           4
 is_white_king_square_attacked:           5
 is_black_king_square_attacked:           2
solution finished. 



               semi-r#1

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   B   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -P   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  semi-r#1                    2 + 4

   1.Bg7-h6 !
      1...b3-b2 #


                        empile:         104
                     play_move:          59
 is_white_king_square_attacked:          59
 is_black_king_square_attacked:          10
solution finished. 



        semi-r##! with setplay

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   Q   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3  -K   .   R   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   K   .  -B   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  semi-r##!1                  3 + 2

      1...Ka3-b2 ##!

the side to play can capture the king
                        empile:         191
                     play_move:         182
 is_white_king_square_attacked:          47
 is_black_king_square_attacked:         147
solution finished. 



semi-r#2 with short solution; White needn't deliver mate immediately

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   B   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -P   .   .   .   .   .   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  semi-r#2                    2 + 4

   1.Bg7-h6 !
      1...b3-b2 #


                        empile:         225
                     play_move:         165
 is_white_king_square_attacked:         154
 is_black_king_square_attacked:          29
solution finished. 



  #3.5 with restricted threat length

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .  -K   .   .   .   8
|                                   |
7   .  -P  -P   .   .  -P  -P   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5  -B   .   B   .  -P   B   .   R   5
|                                   |
4  -R   .   .   .   .   .   .   .   4
|                                   |
3  -P  -P   .   .   .   .   S   .   3
|                                   |
2   .   .   .   .   K   .   .   .   2
|                                   |
1   R   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #4/1                       6 + 10

 threat:
          2.Rh5-h8 #
      1...Ra4-h4
          2.Rh5*h4 threat:
                  3.Rh4-h8 #
              2...f7-f6
                  3.Bf5-e6 threat:
                          4.Rh4-h8 #
                  3.Rh4-h8 +
                      3...Ke8-f7
                          4.Rh8-f8 #
      1...Ra4-e4 +
          2.Sg3*e4 threat:
                  3.Rh5-h8 #
              2...f7-f6
                  3.Rh5-h8 +
                      3...Ke8-f7
                          4.Rh8-f8 #
                  3.Bf5-e6 threat:
                          4.Rh5-h8 #
      1...f7-f6
          2.Rh5-h8 +
              2...Ke8-f7
                  3.Rh8-f8 #

                        empile:       20197
                     play_move:       16944
 is_white_king_square_attacked:        8689
 is_black_king_square_attacked:        9655
solution finished. 



           Didier Innocenti
            Ph\'enix 2001
       no variations, but tries

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .  -Q   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   R   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   Q   .   .  -P   .   .   .   .   4
|                                   |
3   .   .   .   .  -K   .   .   .   3
|                                   |
2   .   .  -R   S   .   .   .   .   2
|                                   |
1   .   .   .   K   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  r#2                         4 + 4
             Uncapturable
              GhostChess

   1.Qa4*c2 ?
    but
      1...Ke3-f4 !

   1.Qa4*d4 + ?
    but
      1...Ke3*d4 !
          2.Sd2-f3 #

   1.Kd1*c2 !


                        empile:       55042
                     play_move:       32181
 is_white_king_square_attacked:       28761
 is_black_king_square_attacked:       10228
solution finished. 


