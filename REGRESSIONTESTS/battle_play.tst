
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

the side to play can capture the king  add_to_move_generation_stack:           0
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

  add_to_move_generation_stack:           4
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

  add_to_move_generation_stack:           7
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
  add_to_move_generation_stack:          51
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
  add_to_move_generation_stack:          64
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

   1.Ka6-b6 ! threat:
          2.Se8-c7 #

   1.Se8-c7 # !

  add_to_move_generation_stack:          63
                     play_move:          44
 is_white_king_square_attacked:          13
 is_black_king_square_attacked:          47

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

  add_to_move_generation_stack:         786
                     play_move:         567
 is_white_king_square_attacked:         490
 is_black_king_square_attacked:         107

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

  add_to_move_generation_stack:         246
                     play_move:         236
 is_white_king_square_attacked:         212
 is_black_king_square_attacked:          44

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

  add_to_move_generation_stack:        1649
                     play_move:         764
 is_white_king_square_attacked:         295
 is_black_king_square_attacked:         484

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

  add_to_move_generation_stack:          75
                     play_move:          57
 is_white_king_square_attacked:          19
 is_black_king_square_attacked:          60

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

  add_to_move_generation_stack:         274
                     play_move:         160
 is_white_king_square_attacked:          46
 is_black_king_square_attacked:         162

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

  add_to_move_generation_stack:         108
                     play_move:          53
 is_white_king_square_attacked:          25
 is_black_king_square_attacked:          56

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
  add_to_move_generation_stack:         403
                     play_move:         233
 is_white_king_square_attacked:          58
 is_black_king_square_attacked:         211

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

  add_to_move_generation_stack:        3375
                     play_move:        2379
 is_white_king_square_attacked:        1408
 is_black_king_square_attacked:        1082

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
  add_to_move_generation_stack:          21
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

  add_to_move_generation_stack:         106
                     play_move:          73
 is_white_king_square_attacked:          73
 is_black_king_square_attacked:          25

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

  add_to_move_generation_stack:         104
                     play_move:          71
 is_white_king_square_attacked:          69
 is_black_king_square_attacked:          29

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

  add_to_move_generation_stack:         160
                     play_move:         123
 is_white_king_square_attacked:         124
 is_black_king_square_attacked:          38

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

   1.Ka1-b1 ?
    but
      1...b3-b2 !

   1.Bd3-b1 !
      1...b3-b2 #

   1.Bd3-f1 ?
    but
      1...b3-b2 + !

   1.Bd3-e2 ?
    but
      1...b3-b2 + !

   1.Rc4-a4 + ?
    but
      1...Ka3*a4 !

   1.Rc4-d4 ?
    but
      1...b3-b2 + !

   1.e4-e5 ?
    but
      1...b3-b2 + !

  add_to_move_generation_stack:         233
                     play_move:         221
 is_white_king_square_attacked:         197
 is_black_king_square_attacked:         125

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

  add_to_move_generation_stack:         561
                     play_move:         545
 is_white_king_square_attacked:         102
 is_black_king_square_attacked:         511

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

  add_to_move_generation_stack:          18
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
  add_to_move_generation_stack:         255
                     play_move:         176
 is_white_king_square_attacked:         178
 is_black_king_square_attacked:          79

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

  add_to_move_generation_stack:        3033
                     play_move:        1532
 is_white_king_square_attacked:        1533
 is_black_king_square_attacked:         859

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

  add_to_move_generation_stack:         801
                     play_move:         361
 is_white_king_square_attacked:         361
 is_black_king_square_attacked:         180

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

  add_to_move_generation_stack:        3070
                     play_move:        1360
 is_white_king_square_attacked:         970
 is_black_king_square_attacked:         677

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

  add_to_move_generation_stack:        3136
                     play_move:        1659
 is_white_king_square_attacked:        1662
 is_black_king_square_attacked:         852

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

  add_to_move_generation_stack:         747
                     play_move:         428
 is_white_king_square_attacked:         430
 is_black_king_square_attacked:         155

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

   1.Sd1-f2 ? zugzwang.
      1...a4-a3
          2.e2-e4
              2...b3-b2 #
          2.e2-e3
              2...b3-b2 #
          2.Sf2-h1
              2...b3-b2 #
          2.Sf2-h3
              2...b3-b2 #
          2.Sf2-g4
              2...b3-b2 #
          2.Sf2-e4
              2...b3-b2 #
          2.c3-c4
              2...b3-b2 #
    but
      1...b3-b2 # !

   1.Sd1-b2 ! zugzwang.
      1...a4-a3
          2.e2-e4
              2...a3*b2 #
          2.e2-e3
              2...a3*b2 #
          2.c3-c4
              2...a3*b2 #

   1.Qe1-d2 + ?
    but
      1...Kc2*d2 !

  add_to_move_generation_stack:        3446
                     play_move:        2010
 is_white_king_square_attacked:        1997
 is_black_king_square_attacked:         931

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
  add_to_move_generation_stack:           4
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

  add_to_move_generation_stack:          30
                     play_move:          21
 is_white_king_square_attacked:          21
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

  add_to_move_generation_stack:          19
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
  add_to_move_generation_stack:           7
                     play_move:           7
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:           8

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
  add_to_move_generation_stack:          52
                     play_move:          45
 is_white_king_square_attacked:          44
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
  add_to_move_generation_stack:           5
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

  add_to_move_generation_stack:         315
                     play_move:         244
 is_white_king_square_attacked:         197
 is_black_king_square_attacked:          77

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

  add_to_move_generation_stack:          24
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

  add_to_move_generation_stack:         845
                     play_move:         597
 is_white_king_square_attacked:         556
 is_black_king_square_attacked:         170

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
  add_to_move_generation_stack:           7
                     play_move:           7
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:           8

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
  add_to_move_generation_stack:         859
                     play_move:         757
 is_white_king_square_attacked:         695
 is_black_king_square_attacked:         143

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

  1  (Ka1-b2 + )
  2  (Ka1-a2 + )
  3  (Ka1-b1 )
  4  (Bd1-h5 )
  5  (Bd1-g4 )
  6  (Bd1-f3 )
  7  (Bd1-e2 )
  8  (Bd1*a4 )
  9  (Bd1-b3 )
 10  (Bd1-c2 )
 11  (b4-b5 )
 12  (b4*a5 )
 13  (Sd8-b7 )
 14  (Sd8-c6 )
   1.Sd8-c6 ! zugzwang.
      1...a5*b4
          2.Bd1-c2 zugzwang.
              2...b4-b3
                  3.Bc2-b1
                      3...b3-b2 #

 15  (Sd8-e6 )
 16  (Sd8-f7 )
  add_to_move_generation_stack:        8466
                     play_move:        6252
 is_white_king_square_attacked:        5550
 is_black_king_square_attacked:        1181

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
  add_to_move_generation_stack:           7
                     play_move:           7
 is_white_king_square_attacked:           2
 is_black_king_square_attacked:           8

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

  add_to_move_generation_stack:        1100
                     play_move:         470
 is_white_king_square_attacked:         453
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
      1...Kh6-h5
          2.Rd1-e1
              2...d2*e1=Q #
              2...d2*e1=R #
      1...Kh6-h7
          2.Rd1-e1
              2...d2*e1=Q #
              2...d2*e1=R #

  add_to_move_generation_stack:         484
                     play_move:         349
 is_white_king_square_attacked:         329
 is_black_king_square_attacked:          78

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
  r#1                         1 + 3

      1...b3-b2 #
  add_to_move_generation_stack:           4
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

  add_to_move_generation_stack:         104
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

the side to play can capture the king  add_to_move_generation_stack:         191
                     play_move:         185
 is_white_king_square_attacked:          49
 is_black_king_square_attacked:         148

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

  add_to_move_generation_stack:         225
                     play_move:         160
 is_white_king_square_attacked:         148
 is_black_king_square_attacked:          30

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
                  3.Rh4-h8 +
                      3...Ke8-f7
                          4.Rh8-f8 #
                  3.Bf5-e6 threat:
                          4.Rh4-h8 #
      1...Ra4-e4 +
          2.Sg3*e4 threat:
                  3.Rh5-h8 #
              2...f7-f6
                  3.Bf5-e6 threat:
                          4.Rh5-h8 #
                  3.Rh5-h8 +
                      3...Ke8-f7
                          4.Rh8-f8 #
      1...f7-f6
          2.Rh5-h8 +
              2...Ke8-f7
                  3.Rh8-f8 #
  add_to_move_generation_stack:       19409
                     play_move:       15662
 is_white_king_square_attacked:        8665
 is_black_king_square_attacked:        8372

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

   1.Kd1*c2 !

   1.Qa4*c2 ?
    but
      1...Ke3-f4 !

   1.Qa4*d4 + ?
    but
      1...Ke3*d4 !
          2.Sd2-f3 #

  add_to_move_generation_stack:       39941
                     play_move:       22184
 is_white_king_square_attacked:       19454
 is_black_king_square_attacked:        7494

solution finished.


