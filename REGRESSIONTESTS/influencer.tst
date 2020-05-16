

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
3   .   S  -S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   B  -R   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  zb21                        2 + 2
             Influencer
               Duplex

   1.Ba1-b2[b1=B][c3=P] z !

  add_to_move_generation_stack:           7
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

   1.Rb1-b2[b3=P][a1=R] z !

  add_to_move_generation_stack:          16
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0

solution finished.



1.Rb8-b2[a1=R] 0-0-0 doesn't solve because of self-check by newborn bPb2

+---a---b---c---d---e---f---g---h---+
|                                   |
8  -R  -R   .   .   .   .   .   .   8
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
1   B   .   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h001                        2 + 2
             Influencer

  1.Ra8-a2[a1=R] 0-0-0
  add_to_move_generation_stack:          61
                     play_move:          24
 is_white_king_square_attacked:           8
 is_black_king_square_attacked:           0

solution finished.




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   B   .   .   .   .   .   8
|                                   |
7   .  -R  -B   .   .   .   .   .   7
|                                   |
6   .  -R  -K   .   .   .   .   .   6
|                                   |
5   .   S   .   .   B   .   .   .   5
|                                   |
4   .   .   K   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                          4 + 4
             Influencer

   1.Sb5-a7[b7=P][b6=P] + !
      1...Bc7-b8[a7=P]
          2.a7*b8=S #

  add_to_move_generation_stack:         829
                     play_move:         328
 is_white_king_square_attacked:          95
 is_black_king_square_attacked:         331

solution finished.




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .  -B  -K   8
|                                   |
7   .   .   .   .  -P   .   .   .   7
|                                   |
6   .   .   .  -P   .   .   .   K   6
|                                   |
5   B   .   .   R   .   .   .   .   5
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
  #2                          3 + 4
             Influencer

   1.Ba5-c3 + !
      1...e7-e5[d5=P]
          2.d5*e6 ep. #

  add_to_move_generation_stack:         304
                     play_move:         114
 is_white_king_square_attacked:          32
 is_black_king_square_attacked:         117

solution finished.




+---a---b---c---d---e---f---g---h---+
|                                   |
8  -R   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   Q   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   P   .  -K   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   P   .   .   .   3
|                                   |
2   .   P   .   .   P   .   .   .   2
|                                   |
1   R   B   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #3                          8 + 2
             Influencer

   1.Ra1*a8 ? zugzwang.
    but
      1...Kd5-c4 !
      1...Kd5-e6[e7=P] !
      1...Kd5-d6[e7=P] !

   1.Bb1-e4 + ?
    but
      1...Kd5-c4 !
      1...Kd5-e6[e7=P] !
      1...Kd5-d6[e7=P] !

   1.Bb1-a2 + ?
    but
      1...Kd5-d6[e7=P] !
      1...Ra8*a2 !
      1...Ra8-a3[a2=P] !

   1.Qe7-c5 + ?
    but
      1...Kd5-c4[c5=P] !
      1...Kd5-e6 !
      1...Kd5*c5 !

   1.Qe7-g5 + ?
    but
      1...Kd5-c4 !
      1...Kd5-e6 !
      1...Kd5-d6 !

   1.Qe7-e4 + ?
    but
      1...Kd5-c5 !
      1...Kd5-d6 !
      1...Kd5-e5[e4=P] !

   1.Qe7-e5 + ?
    but
      1...Kd5-c4 !
      1...Kd5-e6[e5=P] !
      1...Kd5*e5 !

  add_to_move_generation_stack:      279508
                     play_move:       92535
 is_white_king_square_attacked:       44675
 is_black_king_square_attacked:       68178

solution finished.




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .  -P   .   .   .   .   .   7
|                                   |
6  -B   .   K   .   .   .   .   .   6
|                                   |
5   .  -P   .   .   .   .   .   .   5
|                                   |
4  -K  -P   .   .   .   .   .   .   4
|                                   |
3  -P   S   .   .   .   .   .   .   3
|                                   |
2   P   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #2                          3 + 6
             Influencer

   1.Sb3-c5 + !
      1...Ka4-a5
          2.Sc5-b3 #

  add_to_move_generation_stack:         125
                     play_move:          88
 is_white_king_square_attacked:          29
 is_black_king_square_attacked:          90

solution finished.




+---a---b---c---d---e---f---g---h---+
|                                   |
8  -R   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   B   P   .  -K   .   B   .   .   3
|                                   |
2   .   .   .   .   .   P   .   .   2
|                                   |
1   S   .   .   .   K   .  -S   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                         6 + 3
             Influencer

  1.Sg1-h3 Ba3-b2   2.Ra8-a2[b2=P][a1=R] 0-0-0 #
  1.Sg1-h3 Ba3-b2   2.Ra8-a2[b2=P][a1=R] Ra1-d1 #
  add_to_move_generation_stack:       48407
                     play_move:       25112
 is_white_king_square_attacked:        5368
 is_black_king_square_attacked:       24705

solution finished.


