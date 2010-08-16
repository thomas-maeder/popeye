
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
        orig_rbechec:          11
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
        orig_rbechec:          14
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
        orig_rbechec:          16
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
        orig_rbechec:         269
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
        orig_rbechec:         615
        orig_rnechec:         983
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
        orig_rbechec:          19
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

              empile:         262
            jouecoup:         125
        orig_rbechec:          71
        orig_rnechec:         128
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
        orig_rbechec:          36
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
        orig_rbechec:         133
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

              empile:        3535
            jouecoup:        2539
        orig_rbechec:        1526
        orig_rnechec:        1171
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

              empile:          38
            jouecoup:          38
        orig_rbechec:          40
        orig_rnechec:           5
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

              empile:         112
            jouecoup:          87
        orig_rbechec:          88
        orig_rnechec:          23
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

              empile:          96
            jouecoup:          77
        orig_rbechec:          79
        orig_rnechec:          25
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

              empile:         156
            jouecoup:         144
        orig_rbechec:         148
        orig_rnechec:          33
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
5   .   .  -P   .   .   .   .   .   5
|                                   |
4   .   .   R   .   .   P   .   .   4
|                                   |
3  -K  -P   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  s#1                         3 + 4

   1.f4-f5 !
      1...b3-b2 #

   1.Rc4-a4 + ?
    but
      1...Ka3*a4 !

   1.Rc4-e4 ?
      1...b3-b2 #
    but
      1...c5-c4 !

              empile:         190
            jouecoup:         177
        orig_rbechec:         169
        orig_rnechec:          50
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

              empile:         401
            jouecoup:         269
        orig_rbechec:         241
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
        orig_rbechec:          10
        orig_rnechec:          11
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

              empile:         281
            jouecoup:         191
        orig_rbechec:         194
        orig_rnechec:          67
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

              empile:        3039
            jouecoup:        1466
        orig_rbechec:        1468
        orig_rnechec:         770
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

              empile:         767
            jouecoup:         354
        orig_rbechec:         355
        orig_rnechec:         160
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

              empile:        2845
            jouecoup:        1203
        orig_rbechec:        1500
        orig_rnechec:         728
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

              empile:        3355
            jouecoup:        1657
        orig_rbechec:        1664
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

              empile:         710
            jouecoup:         457
        orig_rbechec:         463
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

              empile:        3891
            jouecoup:        2154
        orig_rbechec:        2152
        orig_rnechec:         929
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

              empile:          11
            jouecoup:          11
        orig_rbechec:          12
        orig_rnechec:           2
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

              empile:          65
            jouecoup:          52
        orig_rbechec:          48
        orig_rnechec:          15
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

              empile:          44
            jouecoup:          40
        orig_rbechec:          39
        orig_rnechec:          11
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

              empile:          16
            jouecoup:          15
        orig_rbechec:           4
        orig_rnechec:          16
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

              empile:         121
            jouecoup:          98
        orig_rbechec:          77
        orig_rnechec:          44
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

              empile:        1524
            jouecoup:         984
        orig_rbechec:         764
        orig_rnechec:         477
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

              empile:        1928
            jouecoup:        1288
        orig_rbechec:         968
        orig_rnechec:         449
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

              empile:          16
            jouecoup:          15
        orig_rbechec:           4
        orig_rnechec:          16
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

              empile:        4177
            jouecoup:        2883
        orig_rbechec:        2042
        orig_rnechec:        1648
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

   1.Sd8-c6 ! zugzwang.
      1...a5*b4
          2.Bd1-c2 zugzwang.
              2...b4-b3
                  3.Bc2-b1
                      3...b3-b2 #

              empile:       63866
            jouecoup:       38253
        orig_rbechec:       30364
        orig_rnechec:       10720
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

              empile:          16
            jouecoup:          15
        orig_rbechec:           4
        orig_rnechec:          16
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

              empile:        2121
            jouecoup:         936
        orig_rbechec:         794
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

              empile:        1333
            jouecoup:         895
        orig_rbechec:         687
        orig_rnechec:         284
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

              empile:           7
            jouecoup:           7
        orig_rbechec:           8
        orig_rnechec:           2
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

              empile:         142
            jouecoup:          86
        orig_rbechec:          75
        orig_rnechec:          29
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
              empile:          63
            jouecoup:          60
        orig_rbechec:          49
        orig_rnechec:          16
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

              empile:         391
            jouecoup:         266
        orig_rbechec:         240
        orig_rnechec:          57
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

              empile:       23379
            jouecoup:       20134
        orig_rbechec:       12086
        orig_rnechec:       10417
solution finished. 


