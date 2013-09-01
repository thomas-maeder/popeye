
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
              RankCirce

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
              FileCirce

a) 

   1.d4*e5[+bSe8] x !


  add_to_move_generation_stack:           2
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
b) MirrorFileCirce

   1.d4*e5[+bSe1] x !


  add_to_move_generation_stack:           2
                     play_move:           1
 is_white_king_square_attacked:           0
 is_black_king_square_attacked:           0
solution finished. 


