meld fil    
   y                                         !   !   "   "   ,   ,   -   -   7   7   8   8   B   B   C   C   M   M   N   N   X   X   Y   Y   c   c   d   d   n   n   o   o                     #   #   .   .   9   9   D   D   O   O   Z   Z   e   e   p   p                     $   $   /   /   :   :   E   E   P   P   [   [   f   f   q   q                     %   %   0   0   ;   ;   F   F   Q   Q   \   \   g   g   r   r                     &   &   1   1   <   <   G   G   R   R   ]   ]   h   h   s   s                     '   '   2   2   =   =   H   H   S   S   ^   ^   i   i   t   t                     (   (   3   3   >   >   I   I   T   T   _   _   j   j   u   u                     )   )   4   4   ?   ?   J   J   U   U   `   `   k   k   v   v   	   	               *   *   5   5   @   @   K   K   V   V   a   a   l   l   w   w   
   
                 +   +   6   6   A   A   L   L   W   W   b   b   m   m   x   x    	                init -o axioms�   receive-down(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {R | !right(R), 
			R != host-id | propagate-right(Nodes, Coords)@R}, {L | !left(L), 
			L != host-id | propagate-left(Nodes, Coords)@L}.�   propagate-left(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {L | !left(L), 
			L != host-id | propagate-left(Nodes, Coords)@L}.�   propagate-right(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {R | !right(R), 
			R != host-id | propagate-right(Nodes, Coords)@R}.T   test-and-send-down(Nodes, Coords), !coord(X, Y) -o test-y(Y, Coords, Nodes, Coords).x   test-y(Y, MV23, Nodes, Coords), !coord(OX, OY), test-nil(MV23) -o test-diag-left(OX - 1, OY - 1, Coords, Nodes, Coords).~   test-y(Y, MV24, Nodes, Coords), MV25 = tail(MV24), Y1 = head(MV25), Y = Y1, 
			not(test-nil(MV24)), not(test-nil(MV25)) -o 1.�   test-y(Y, MV27, Nodes, Coords), MV28 = tail(MV27), Y1 = head(MV28), RestCoords = tail(MV28), 
			Y != Y1, not(test-nil(MV27)), not(test-nil(MV28)) -o test-y(Y, RestCoords, Nodes, Coords).�   test-diag-left(X, Y, MV22, Nodes, Coords), !coord(OX, OY), (X < 0) NIL (Y < 0) -o test-diag-right(OX - 1, OY + 1, Coords, Nodes, Coords).�   test-diag-left(X, Y, MV30, Nodes, Coords), X1 = head(MV30), MV31 = tail(MV30), Y1 = head(MV31), 
			X = X1, Y = Y1, not(test-nil(MV30)), not(test-nil(MV31)) -o 1.�   test-diag-left(X, Y, MV33, Nodes, Coords), X1 = head(MV33), MV34 = tail(MV33), Y1 = head(MV34), 
			RestCoords = tail(MV34), (X != X1) NIL (Y != Y1), not(test-nil(MV33)), not(test-nil(MV34)) -o test-diag-left(X - 1, Y - 1, RestCoords, Nodes, Coords).�   test-diag-right(X, Y, MV36, Nodes, Coords), !coord(OX, OY), (X < 0) NIL (Y >= 11), test-nil(MV36) -o 
			send-down(cons(host-id,Nodes), cons(OX,cons(OY,Coords))).�   test-diag-right(X, Y, MV37, Nodes, Coords), X1 = head(MV37), MV38 = tail(MV37), Y1 = head(MV38), 
			X = X1, Y = Y1, not(test-nil(MV37)), not(test-nil(MV38)) -o 1.�   test-diag-right(X, Y, MV40, Nodes, Coords), X1 = head(MV40), MV41 = tail(MV40), Y1 = head(MV41), 
			RestCoords = tail(MV41), (X != X1) NIL (Y != Y1), not(test-nil(MV40)), not(test-nil(MV41)) -o test-diag-right(X - 1, Y + 1, RestCoords, Nodes, Coords).N   send-down(Nodes, Coords), !down(A), A = host-id -o final-state(Nodes, Coords).R   send-down(Nodes, Coords), !down(B), host-id != B -o receive-down(Nodes, Coords)@B.          0       �              _init                                                               set-priority                                                        setcolor                                                            setedgelabel                                                        write-string                                                        add-priority                                                         schedule-next                                                       setColor2                                                            left                                                                 right                                                                down                                                                 coord                                                                propagate-left                                                       propagate-right                                                      receive-down                                                         test-and-send-down                                                    test-y                                                                test-diag-left                                                        test-diag-right                                                      send-down                                                            final-state                                                              �                       "      �     �
   y                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      @ p  p      
�  y   �  {    �  
  �  -  �  A  �  U  �  Y  �  m  �	  �    �    �  3  �  7  �  K  �	  _  �  s  �  �    �    �  )  �	  =  �  Q  �  e  �   i  �  }    �	    �  /  �  C  �   G  �  [  �
  o	  �  �    �  !  �   %  �  9  �
  M	  �  a  �  u  �  �     �    �
  +	  �  ?  �  S  �  g   �  k  �  
  		  �    �  1  �  E   �  I  �  ]
  �  q  �  �    �  #   �  '  �  ;
  �  O  �  c  �  w        	x   m   
x   
   
   �     	x   b   
m   
   	   �     	m   W   
b   
      �     	b   L   
W   
      �     	W   A   
L   
      m     	L   6   
A   
      K     	A   +   
6   
      )     	6       
+   
           	+      
    
      �     	    
   
   
      �     	   
   

   
       �     	w   l   
x   	   
        	w   a   
m   	   	   ]     	l   V   
b   	      ;     	a   K   
W   	           	V   @   
L   	      �     	K   5   
A   	      �     	@   *   
6   	      �     	5      
+   	      �     	*      
    	      o     	   	   
   	      M     	   	   

   	       +     	v   k   
w      
   	     	v   `   
l      	   �     	k   U   
a         �     	`   J   
V         �     	U   ?   
K         �     	J   4   
@         _     	?   )   
5         =     	4      
*              	)      
         �     	      
         �     	      
	          �     	u   j   
v      
   �     	u   _   
k      	   q     	j   T   
`         O     	_   I   
U         -     	T   >   
J              	I   3   
?         �
     	>   (   
4         �
     	3      
)         �
     	(      
         �
     	      
         a
     	      
          ?
     	t   i   
u      
   
     	t   ^   
j      	   �	     	i   S   
_         �	     	^   H   
T         �	     	S   =   
I         �	     	H   2   
>         s	     	=   '   
3         Q	     	2      
(         /	     	'      
         	     	      
         �     	      
          �     	s   h   
t      
   �     	s   ]   
i      	   �     	h   R   
^         c     	]   G   
S         A     	R   <   
H              	G   1   
=         �     	<   &   
2         �     	1      
'         �     	&      
         �     	      
         u     	      
          S     	r   g   
s      
   1     	r   \   
h      	        	g   Q   
]         �     	\   F   
R         �     	Q   ;   
G         �     	F   0   
<         �     	;   %   
1         e     	0      
&         C     	%      
         !     	      
         �     	      
          �     	q   f   
r      
   �     	q   [   
g      	   �     	f   P   
\         w     	[   E   
Q         U     	P   :   
F         3     	E   /   
;              	:   $   
0         �     	/      
%         �     	$      
         �     	      
         �     	      
          g     	p   e   
q      
   E     	p   Z   
f      	   #     	e   O   
[              	Z   D   
P         �     	O   9   
E         �     	D   .   
:         �     	9   #   
/         y     	.      
$         W     	#      
         5     	      
              	      
          �     	o   d   
p      
   �     	o   Y   
e      	   �     	d   N   
Z         �     	Y   C   
O         i     	N   8   
D         G     	C   -   
9         %     	8   "   
.              	-      
#         �     	"      
         �     	      
         �     	      
          {     	n   c   
o       
   Y     	n   X   
d       	   7     	c   M   
Y               	X   B   
N          �      	M   7   
C          �      	B   ,   
8          �      	7   !   
-          �      	,      
"          k      	!      
          I      	       
          '      	       
              0 � 0         �      �      � �    �0 @0   0 ;   �	  5    �0!�" `    @0   0 0# �;   �  5    �0!�" `    @0   0 0# �� �     g      � a    �0 @0   0 ;   �  5    �0!�" `    @0   0 0# �� �     g      � a    �0 @0   0 ;   �	  5    �0!�" `    @0   0 0# �� �     H      � B    �0 �  4    �0!@0 0 0  0 � �     f      � `    �0 ! `M   �  G    �0!@�     �   0 0 0 � �     I      � C    �0 ! !"`-    ! !""#`    !"�"#  `
   � �     s      � m    �0 ! !"`W    ! !""#`E    !"�"#  `4    !  !"@0   0"0 0 � �     |      � v    �0 �!      �"     �!"#`M   �  G    �0!@�     �   0 0 0 � �     \   	   � V    �0 ! !"`@    ! !""#`.    !"�"# `    # �#$  `
   � �     �   
   � �    �0 ! !"`|    ! !""#`j    !" # �#$  �"% �$%&`G    !  !"@�      �    0"0 0 � �     s      � m    �0 ! `Z   �!      �"     �!"#`9   �  3    �0!@   #  # � �     \      � V    �0 ! !"`@    ! !""#`.    !"�"# `    # �#$  `
   � �     �      � �    �0 ! !"`|    ! !""#`j    !" # �#$  �"% �$%&`G    !  !"@�      �    0"0 0 � �     G      � A    �0 �
  3    �0!�" `   @0   0 � �     
L      � F    �0 �
  8    �0!�" `#   @0   0 0# � �     
