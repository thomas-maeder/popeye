
      only example of Rank Circe

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .  -Q   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4  -S   .   R  -S   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 3
              Circe Rank

a) 

   1.Rc4*a4[+bSg4] x !

   1.Rc4*c6[+bQd6] x !

   1.Rc4*d4[+bSb4] x !


  add_to_move_generation_stack:           8
                     play_move:           3
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
b) shift a1 ==> a2

   1.Rc5*a5[+bSg5] x !

   1.Rc5*c7[+bQd7] x !

   1.Rc5*d5[+bSb5] x !


  add_to_move_generation_stack:           9
                     play_move:           3
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



  only example of Mirror File Circe

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .  -S   .   .   .   5
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
  x1                          1 + 1
              Circe File

a) 

   1.d4*e5[+bSe8] x !


  add_to_move_generation_stack:           2
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
b) Circe File Mirror

   1.d4*e5[+bSe1] x !


  add_to_move_generation_stack:           2
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



  only example of Diagram Anticirce

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -R   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
          AntiCirce Diagram

   1.Sc3*d5[wSd5->c3] x !


  add_to_move_generation_stack:           8
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



   only example of Mirror Anticirce

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -R   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
           AntiCirce Mirror

   1.Sc3*d5[wSd5->g8] x !


  add_to_move_generation_stack:           8
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



    only example of File Anticirce

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -R   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
            AntiCirce File

   1.Sc3*d5[wSd5->d1] x !


  add_to_move_generation_stack:           8
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



  only example of Symmetry Anticirce

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -R   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
          AntiCirce Symmetry

   1.Sc3*d5[wSd5->e4] x !


  add_to_move_generation_stack:           8
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



only example of Mirror File Anticirce

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -R   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
        AntiCirce File Mirror

   1.Sc3*d5[wSd5->d8] x !


  add_to_move_generation_stack:           8
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



 only example of Antipodean Anticirce

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -R   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
         AntiCirce Antipodes

   1.Sc3*d5[wSd5->h1] x !


  add_to_move_generation_stack:           8
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



only example of Equipollents Anticirce

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -R   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   S   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  x1                          1 + 1
        AntiCirce Equipollents

   1.Sc3*d5[wSd5->e7] x !


  add_to_move_generation_stack:           8
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



only example of Anticirce with Locust

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
4   .   .   .   .  -P   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   L   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 1
              AntiCirce

   1.Lc2*e4-f5[wLf5->e8] !


  add_to_move_generation_stack:           1
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



    check in Circe Mirror Assassin

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5  -R  -P   .   .   .   .   .   .   5
|                                   |
4  -P   .  -S   .   .   .   .   .   4
|                                   |
3   .   .   K   .   .   .   .   .   3
|                                   |
2  -K   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  #1                          1 + 5
        Circe Mirror Assassin

   1.Kc3-b4 # !


  add_to_move_generation_stack:          24
                     play_move:          56
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



          Anticirce Assassin

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
2  -P   .   .   .   .   .   .   .   2
|                                   |
1   K   .   .   .  -S   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          1 + 2
          AntiCirce Assassin

   1.Ka1-b2 !

   1.Ka1*a2[wKa2->e1] !


  add_to_move_generation_stack:           3
                     play_move:           4
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 



     check in Anticirce Assassin

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7  -P   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   K   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .  -K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  +1                          1 + 2
          AntiCirce Assassin

   1.Ka5-a6 + !


  add_to_move_generation_stack:           5
                     play_move:           8
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7  -P   .   .   .   .   .   .   .   7
|                                   |
6   .   S   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .  -S   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hzh31                       1 + 2
           Circe Parachute

  1.a7*b6[+wSg1] Sg1-h3[+bSg1] z

  add_to_move_generation_stack:          39
                     play_move:           7
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .  -B   8
|                                   |
7   .   .   .   .   .   .   S   .   7
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
1   .   .   .   .   .   .   S   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  hzh31                       2 + 1
            Circe Volcanic

  1.Bh8*g7[[+wSg1]] Sg1-h3[+wSg1] z

  add_to_move_generation_stack:           4
                     play_move:           2
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 




+---a---b---c---d---e---f---g---h---+
|                                   |
8  -B   .   S   .   .   .   .   .   8
|                                   |
7   .   P   .   .   .   .   .   .   7
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
  hze71                       2 + 1
       Circe Couscous Volcanic

a) 

  1.Ba8*b7[[+wPc8]] Sc8-e7[+wPc8=Q] z
  1.Ba8*b7[[+wPc8]] Sc8-e7[+wPc8=S] z
  1.Ba8*b7[[+wPc8]] Sc8-e7[+wPc8=R] z
  1.Ba8*b7[[+wPc8]] Sc8-e7[+wPc8=B] z

  add_to_move_generation_stack:           5
                     play_move:           5
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
b) Circe Couscous Parachute

  1.Ba8*b7[+wPc8=S] Sc8-e7[+wSc8] z

  add_to_move_generation_stack:          45
                     play_move:           5
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   B   8
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
2   .  -P   .   .   .   .   .   .   2
|                                   |
1   .   .   R   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ser-zf12                    2 + 1
            Paralysing b2
       Circe Couscous Volcanic

a) 

  1.Bh8*b2[[+bpPc1]]   2.Rc1-f1[+bpPc1=pQ] z
  1.Bh8*b2[[+bpPc1]]   2.Rc1-f1[+bpPc1=pS] z
  1.Bh8*b2[[+bpPc1]]   2.Rc1-f1[+bpPc1=pR] z
  1.Bh8*b2[[+bpPc1]]   2.Rc1-f1[+bpPc1=pB] z

  add_to_move_generation_stack:          86
                     play_move:          10
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
b) ser-za12  Circe Couscous Parachute

  1.Bh8*b2[+bpPc1=pS]   2.Bb2-a1 z
  1.Bh8*b2[+bpPc1=pR]   2.Bb2-a1 z

  add_to_move_generation_stack:          82
                     play_move:          11
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 




+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .  -B   .   .   S   8
|                                   |
7   .   .   .   .   .   B  -P   .   7
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
  hzf71                       2 + 2
         Circe Cage Volcanic

  1.Be8*f7[[+wBh8]] Sh8*f7[+wBh8][[+bBh8]] z

  add_to_move_generation_stack:        1122
                     play_move:         134
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 


