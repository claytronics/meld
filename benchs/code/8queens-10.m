meld fil    	   d                                         	   	                                       "   "   #   #   +   +   ,   ,   5   5   6   6   ?   ?                                 !   !   *   *   4   4   >   >   G   G                                   )   )   3   3   =   =   F   F   N   N   
   
                     (   (   2   2   <   <   E   E   M   M   T   T                     '   '   1   1   ;   ;   D   D   L   L   S   S   Y   Y               &   &   0   0   :   :   C   C   K   K   R   R   X   X   ]   ]         %   %   /   /   9   9   B   B   J   J   Q   Q   W   W   \   \   `   `   $   $   .   .   8   8   A   A   I   I   P   P   V   V   [   [   _   _   b   b   -   -   7   7   @   @   H   H   O   O   U   U   Z   Z   ^   ^   a   a   c   c                  init -o axioms�   receive-down(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {R | !right(R), 
			R != host-id | propagate-right(Nodes, Coords)@R}, {L | !left(L), 
			L != host-id | propagate-left(Nodes, Coords)@L}.�   propagate-left(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {L | !left(L), 
			L != host-id | propagate-left(Nodes, Coords)@L}.�   propagate-right(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {R | !right(R), 
			R != host-id | propagate-right(Nodes, Coords)@R}.T   test-and-send-down(Nodes, Coords), !coord(X, Y) -o test-y(Y, Coords, Nodes, Coords).r   test-y(Y, MV2, Nodes, Coords), !coord(OX, OY), MV2 = nil -o test-diag-left(OX - 1, OY - 1, Coords, Nodes, Coords).x   test-y(Y, MV3, Nodes, Coords), MV4 = tail(MV3), Y1 = head(MV4), Y = Y1, 
			not(test-nil(MV3)), not(test-nil(MV4)) -o 1.�   test-y(Y, MV6, Nodes, Coords), MV7 = tail(MV6), Y1 = head(MV7), RestCoords = tail(MV7), 
			Y != Y1, not(test-nil(MV6)), not(test-nil(MV7)) -o test-y(Y, RestCoords, Nodes, Coords).�   test-diag-left(X, Y, MV1, Nodes, Coords), !coord(OX, OY), (X < 0) NIL (Y < 0) -o test-diag-right(OX - 1, OY + 1, Coords, Nodes, Coords).�   test-diag-left(X, Y, MV9, Nodes, Coords), X1 = head(MV9), MV10 = tail(MV9), Y1 = head(MV10), 
			X = X1, Y = Y1, not(test-nil(MV9)), not(test-nil(MV10)) -o 1.�   test-diag-left(X, Y, MV12, Nodes, Coords), X1 = head(MV12), MV13 = tail(MV12), Y1 = head(MV13), 
			RestCoords = tail(MV13), (X != X1) NIL (Y != Y1), not(test-nil(MV12)), not(test-nil(MV13)) -o test-diag-left(X - 1, Y - 1, RestCoords, Nodes, Coords).�   test-diag-right(X, Y, MV15, Nodes, Coords), !coord(OX, OY), (X < 0) NIL (Y >= 10), MV15 = nil -o 
			send-down(cons(host-id,Nodes), cons(OX,cons(OY,Coords))).�   test-diag-right(X, Y, MV16, Nodes, Coords), X1 = head(MV16), MV17 = tail(MV16), Y1 = head(MV17), 
			X = X1, Y = Y1, not(test-nil(MV16)), not(test-nil(MV17)) -o 1.�   test-diag-right(X, Y, MV19, Nodes, Coords), X1 = head(MV19), MV20 = tail(MV19), Y1 = head(MV20), 
			RestCoords = tail(MV20), (X != X1) NIL (Y != Y1), not(test-nil(MV19)), not(test-nil(MV20)) -o test-diag-right(X - 1, Y + 1, RestCoords, Nodes, Coords).N   send-down(Nodes, Coords), !down(A), A = host-id -o final-state(Nodes, Coords).R   send-down(Nodes, Coords), !down(B), host-id != B -o receive-down(Nodes, Coords)@B.           0
       �                                              _init                                                                                              set-priority                                                                                        setcolor                                                            	                              setedgelabel                                                        	                               write-string                                                                                       add-priority                                                                                         schedule-next                                                                                       setColor2                                                                                           left                                                                                                right                                                                                               down                                                                                                 coord                                                                                             propagate-left                                                                                    propagate-right                                                                                   receive-down                                                                                      test-and-send-down                                                                               test-y                                                                                           test-diag-left                                                                                   test-diag-right                                                                                   send-down                                                                                         final-state                                                              �                             �     �
�  d   '  �    
  �  �  +	  ]
  �  �  �  		  ;
  m  �  �  �  �  
  K  }  /  a  �  �  �	  )  [  �    ?  q  �  �	    9  �  �  �    O  �  �	  �
    3  e  �  �  �  -  _  �	  �
  �    C  u  �  �    =  o	  �
  �   !  S  �  �  �    M	  �   �  1  c  �  �  �  �   �    A  s  �  �   �  �    Q  g   �  �  �  E   w  �  #   U        	c   a   
c   	   	   .     	c   ^   
a   	           	a   Z   
^   	      �     	^   U   
Z   	      �     	Z   O   
U   	      �     	U   H   
O   	      �     	O   @   
H   	      b     	H   7   
@   	      @     	@   -   
7   	           	7   -   
-   	       �     	b   _   
c      	   �     	b   [   
a         �     	_   V   
^         �     	[   P   
Z         t     	V   I   
U         R     	P   A   
O         0     	I   8   
H              	A   .   
@         �
     	8   $   
7         �
     	.   $   
-          �
     	`   \   
b      	   �
     	`   W   
_         d
     	\   Q   
[         B
     	W   J   
V          
     	Q   B   
P         �	     	J   9   
I         �	     	B   /   
A         �	     	9   %   
8         �	     	/      
.         v	     	%      
$          T	     	]   X   
`      	   2	     	]   R   
\         	     	X   K   
W         �     	R   C   
Q         �     	K   :   
J         �     	C   0   
B         �     	:   &   
9         f     	0      
/         D     	&      
%         "     	      
                	Y   S   
]      	   �     	Y   L   
X         �     	S   D   
R         �     	L   ;   
K         x     	D   1   
C         V     	;   '   
:         4     	1      
0              	'      
&         �     	      
         �     	      
          �     	T   M   
Y      	   �     	T   E   
S         h     	M   <   
L         F     	E   2   
D         $     	<   (   
;              	2      
1         �     	(      
'         �     	      
         �     	   
   
         z     	   
   
          X     	N   F   
T      	   6     	N   =   
M              	F   3   
E         �     	=   )   
<         �     	3       
2         �     	)      
(         �     	       
         j     	      
         H     	      
         &     	      

               	G   >   
N      	   �     	G   4   
F         �     	>   *   
=         �     	4   !   
3         |     	*      
)         Z     	!      
          8     	      
              	      
         �     	      
         �     	      
          �     	?   5   
G      	   �     	?   +   
>         l     	5   "   
4         J     	+      
*         (     	"      
!              	      
         �     	      
         �     	      
         �     	      
         ~     	      
          \     	6   ,   
?       	   :     	6   #   
5               	,      
+          �      	#      
"          �      	      
          �      	   	   
          �      	      
          n      		      
          L      	       
          *       	       
                0 � 0         �      �      � �    �0 @0   0 ;   �	  5    �0!�" `    @0   0 0# �;   �  5    �0!�" `    @0   0 0# �� �     g      � a    �0 @0   0 ;   �  5    �0!�" `    @0   0 0# �� �     g      � a    �0 @0   0 ;   �	  5    �0!�" `    @0   0 0# �� �     H      � B    �0 �  4    �0!@0 0 0  0 � �     f      � `    �0 ! `M   �  G    �0!@�     �   0 0 0 � �     I      � C    �0 ! !"`-    ! !""#`    !"�"#  `
   � �     s      � m    �0 ! !"`W    ! !""#`E    !"�"#  `4    !  !"@0   0"0 0 � �     |      � v    �0 �!      �"     �!"#`M   �  G    �0!@�     �   0 0 0 � �     \   	   � V    �0 ! !"`@    ! !""#`.    !"�"# `    # �#$  `
   � �     �   
   � �    �0 ! !"`|    ! !""#`j    !" # �#$  �"% �$%&`G    !  !"@�      �    0"0 0 � �     s      � m    �0 �!      �"     �!"#`D   ! `9   �  3    �0!@   #  # � �     \      � V    �0 ! !"`@    ! !""#`.    !"�"# `    # �#$  `
   � �     �      � �    �0 ! !"`|    ! !""#`j    !" # �#$  �"% �$%&`G    !  !"@�      �    0"0 0 � �     G      � A    �0 �
  3    �0!�" `   @0   0 � �     
L      � F    �0 �
  8    �0!�" `#   @0   0 0# � �     
