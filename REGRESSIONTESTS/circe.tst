
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


