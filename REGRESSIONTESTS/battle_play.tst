
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
            jouecoup:           0
        orig_rbechec:           0
        orig_rnechec:           1
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
            jouecoup:           4
        orig_rbechec:           1
        orig_rnechec:           5
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
            jouecoup:          48
        orig_rbechec:          10
        orig_rnechec:          48
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
            jouecoup:          58
        orig_rbechec:          12
        orig_rnechec:          58
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
            jouecoup:          46
        orig_rbechec:          14
        orig_rnechec:          48
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

              empile:         518
            jouecoup:         348
        orig_rbechec:         251
        orig_rnechec:         140
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

              empile:          83
            jouecoup:          77
        orig_rbechec:          41
        orig_rnechec:          53
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

   1.Rc3-c1 # !

   1.Rc3-b3 = !

              empile:        1561
            jouecoup:         728
        orig_rbechec:         352
        orig_rnechec:         984
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
            jouecoup:          53
        orig_rbechec:          18
        orig_rnechec:          55
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
            jouecoup:         131
        orig_rbechec:          48
        orig_rnechec:         132
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
            jouecoup:          48
        orig_rbechec:          29
        orig_rnechec:          50
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
      1...Se8-c7 +
          2.Se6*c7 #
      1...Se8-g7
          2.Se6-c7 #
      1...Se8-f6
          2.Se6-c7 #
      1...Se8-d6
          2.Se6-c7 #
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

              empile:         473
            jouecoup:         312
        orig_rbechec:         124
        orig_rnechec:         222
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
      1...Se8-c7 +
          2.Se6*c7 #
      1...Se8-g7
          2.Se6-c7 #
      1...Se8-f6
          2.Se6-c7 #
      1...Se8-d6
          2.Se6-c7 #
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

              empile:        3545
            jouecoup:        2549
        orig_rbechec:        1486
        orig_rnechec:        1174
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
            jouecoup:          21
        orig_rbechec:          22
        orig_rnechec:           4
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

              empile:          95
            jouecoup:          70
        orig_rbechec:          70
        orig_rnechec:          22
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

              empile:          87
            jouecoup:          68
        orig_rbechec:          66
        orig_rnechec:          26
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

              empile:         131
            jouecoup:         119
        orig_rbechec:         120
        orig_rnechec:          34
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

              empile:         223
            jouecoup:         186
        orig_rbechec:         162
        orig_rnechec:         114
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

              empile:         234
            jouecoup:         207
        orig_rbechec:         160
        orig_rnechec:         113
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
            jouecoup:          18
        orig_rbechec:          11
        orig_rnechec:          10
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
            jouecoup:         165
        orig_rbechec:         167
        orig_rnechec:          66
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
            jouecoup:        1440
        orig_rbechec:        1441
        orig_rnechec:         769
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
            jouecoup:         340
        orig_rbechec:         340
        orig_rnechec:         159
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
      1...a3*b2 #
      1...c3*b2 #

              empile:        2835
            jouecoup:        1193
        orig_rbechec:        1484
        orig_rnechec:         722
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
            jouecoup:        1577
        orig_rbechec:        1580
        orig_rnechec:         779
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
            jouecoup:         419
        orig_rbechec:         421
        orig_rnechec:         146
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

   1.Sd1-f2 ?
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

   1.Sd1-b2 !
      1...a4-a3
          2.c3-c4
              2...a3*b2 #
          2.e2-e4
              2...a3*b2 #
          2.e2-e3
              2...a3*b2 #

              empile:        3679
            jouecoup:        1942
        orig_rbechec:        1929
        orig_rnechec:         918
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
            jouecoup:           4
        orig_rbechec:           5
        orig_rnechec:           1
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
            jouecoup:          29
        orig_rbechec:          29
        orig_rnechec:           7
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
            jouecoup:          19
        orig_rbechec:          20
        orig_rnechec:           5
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
            jouecoup:           6
        orig_rbechec:           2
        orig_rnechec:           7
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
            jouecoup:          44
        orig_rbechec:          43
        orig_rnechec:           8
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
            jouecoup:           5
        orig_rbechec:           6
        orig_rnechec:           1
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

              empile:         331
            jouecoup:         217
        orig_rbechec:         190
        orig_rnechec:          59
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
            jouecoup:          24
        orig_rbechec:          25
        orig_rnechec:           6
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

              empile:         866
            jouecoup:         627
        orig_rbechec:         581
        orig_rnechec:         175
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
            jouecoup:           6
        orig_rbechec:           2
        orig_rnechec:           7
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

              empile:        1236
            jouecoup:         793
        orig_rbechec:         713
        orig_rnechec:         223
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
              empile:       11343
            jouecoup:        6677
        orig_rbechec:        5934
        orig_rnechec:        1619
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
            jouecoup:           6
        orig_rbechec:           2
        orig_rnechec:           7
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

              empile:        1406
            jouecoup:         581
        orig_rbechec:         564
        orig_rnechec:           0
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

              empile:         567
            jouecoup:         400
        orig_rbechec:         364
        orig_rnechec:         104
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
            jouecoup:           4
        orig_rbechec:           5
        orig_rnechec:           1
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
            jouecoup:          59
        orig_rbechec:          59
        orig_rnechec:          10
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
              empile:          74
            jouecoup:          71
        orig_rbechec:          50
        orig_rnechec:          28
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
            jouecoup:         165
        orig_rbechec:         154
        orig_rnechec:          29
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
      1...Ra4-e4 +
          2.Sg3*e4 threat:
                  3.Rh5-h8 #
              2...f7-f6
                  3.Rh5-h8 +
                      3...Ke8-f7
                          4.Rh8-f8 #
                  3.Bf5-e6 threat:
                          4.Rh5-h8 #
      1...Ra4-h4
          2.Rh5*h4 threat:
                  3.Rh4-h8 #
              2...f7-f6
                  3.Bf5-e6 threat:
                          4.Rh4-h8 #
                  3.Rh4-h8 +
                      3...Ke8-f7
                          4.Rh8-f8 #
      1...f7-f6
          2.Rh5-h8 +
              2...Ke8-f7
                  3.Rh8-f8 #

              empile:       23086
            jouecoup:       20141
        orig_rbechec:       11314
        orig_rnechec:       10234
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

              empile:       62961
            jouecoup:       37000
        orig_rbechec:       33580
        orig_rnechec:       11634
solution finished. 


