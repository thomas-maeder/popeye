
Alice Chess move generation and execution

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .  -P   .   5
|                                   |
4   .   .   .   .   .   P   .   B   4
|                                   |
3   .   .   .   .   .   .  -P   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   K   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  ~1                          3 + 2
           BoardA h4 g3 d1
            BoardB g5 f4
             AliceChess

   1.f4-f5A !

   1.f4*g5A !

   1.Kd1-e2B !

   1.Kd1-c2B !

   1.Kd1-c1B !

   1.Kd1-d2B !

   1.Kd1-e1B !

   1.Bh4*g3B !

   1.Bh4-d8B !

   1.Bh4-e7B !

   1.Bh4-f6B !


solution finished.



  Alice check only from same board

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .  -K   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   P   .   P   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  +1                          2 + 1
            BoardA d5 c3
              BoardB e3
             AliceChess

   1.e3-e4A + !


solution finished.


