meld fil       d                                                                                                
                                   	              
                                                                                                                                     	                                                                                                                                                                         !               "       !              "       #       #              $       $       %       %       &       &       '       '       (       *       )       +       *       ,       +       (       ,       -       -       )       .       .       /       /       0       0       1       1       2       4       3       5       4       6       5       2       6       7       7       3       8       8       9       9       :       :       ;       ;       <       >       =       ?       >       @       ?       <       @       A       A       =       B       B       C       C       D       D       E       E       F       H       G       I       H       J       I       F       J       K       K       G       L       L       M       M       N       N       O       O       P       R       Q       S       R       T       S       P       T       U       U       Q       V       V       W       W       X       X       Y       Y       Z       \       [       ]       \       ^       ]       Z       ^       _       _       [       `       `       a       a       b       b       c       c        	                init -o axioms�   propagate-left(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {L | !left(L), 
			L != host-id | propagate-left(Nodes, Coords)@L}.�   propagate-right(Nodes, Coords) -o test-and-send-down(Nodes, Coords), {R | !right(R), 
			R != host-id | propagate-right(Nodes, Coords)@R}.T   test-and-send-down(Nodes, Coords), !coord(X, Y) -o test-y(Y, Coords, Nodes, Coords).v   test-y(Y, MV3, Nodes, Coords), !coord(OX, OY), test-nil(MV3) -o test-diag-left(OX - 1, OY - 1, Coords, Nodes, Coords).x   test-y(Y, MV4, Nodes, Coords), MV5 = tail(MV4), Y1 = head(MV5), Y = Y1, 
			not(test-nil(MV4)), not(test-nil(MV5)) -o 1.�   test-y(Y, MV7, Nodes, Coords), MV8 = tail(MV7), Y1 = head(MV8), RestCoords = tail(MV8), 
			Y != Y1, not(test-nil(MV7)), not(test-nil(MV8)) -o test-y(Y, RestCoords, Nodes, Coords).�   test-diag-left(X, Y, MV1, Nodes, Coords), !coord(OX, OY), (X < 0) || (Y < 0) -o test-diag-right(OX - 1, OY + 1, Coords, Nodes, Coords).�   test-diag-left(X, Y, MV10, Nodes, Coords), X1 = head(MV10), MV11 = tail(MV10), Y1 = head(MV11), 
			X = X1, Y = Y1, not(test-nil(MV10)), not(test-nil(MV11)) -o 1.�   test-diag-left(X, Y, MV13, Nodes, Coords), X1 = head(MV13), MV14 = tail(MV13), Y1 = head(MV14), 
			RestCoords = tail(MV14), (X != X1) || (Y != Y1), not(test-nil(MV13)), not(test-nil(MV14)) -o test-diag-left(X - 1, Y - 1, RestCoords, Nodes, Coords).�   test-diag-right(X, Y, MV16, Nodes, Coords), !coord(OX, OY), (X < 0) || (Y >= 10), test-nil(MV16) -o 
			send-down(cons(host-id,Nodes), cons(OX,cons(OY,Coords))).�   test-diag-right(X, Y, MV17, Nodes, Coords), X1 = head(MV17), MV18 = tail(MV17), Y1 = head(MV18), 
			X = X1, Y = Y1, not(test-nil(MV17)), not(test-nil(MV18)) -o 1.�   test-diag-right(X, Y, MV20, Nodes, Coords), X1 = head(MV20), MV21 = tail(MV20), Y1 = head(MV21), 
			RestCoords = tail(MV21), (X != X1) || (Y != Y1), not(test-nil(MV20)), not(test-nil(MV21)) -o test-diag-right(X - 1, Y + 1, RestCoords, Nodes, Coords).T   send-down(Nodes, Coords), !coord(MV23, MV2), MV23 = 9 -o final-state(Nodes, Coords).�   send-down(Nodes, Coords) -o {B | !down-right(B), B != host-id | 
			propagate-right(Nodes, Coords)@B}, {B | !down-left(B), B != host-id | 
			propagate-left(Nodes, Coords)@B}.           �                  _init                                                               set-priority                                                        setcolor                                                            setedgelabel                                                        write-string                                                        add-priority                                                         schedule-next                                                       setColor2                                                            left                                                                 right                                                                down-right                                                           down-left                                                            coord                                                                propagate-left                                                       propagate-right                                                      test-and-send-down                                                    test-y                                                                test-diag-left                                                        test-diag-right                                                      send-down                                                            final-state                                                              �                                                                                                           a      �          
    [   � 
  d   F  �    {  �  D     �    �  j  �  3  �  �  h  �  1  �  W  �  U    �  �  �  �  y  B    f  /  �  �  �  �  �  S    �  @  	  �  �  �  w  d  -  �
  �
  
  �	  �	  �
  u	  Q
  >	  	  �  �  �  �  �  b  O  +    �  �  s  �  �  `  <  )    �  �  �  M  �  q  :      �  �  �  ^  '  �  K    �  �   �  �   o   8      2   	c       b       c       
c       	   	   J  2   	c       a       b       
b       	        2   	b       `       a       
a       	      �  2   	a       ^       `       
`       	      �  2   	`       \       ^       
^       	      n  2   	^       [       \       
\       	      7  2   	\       Z       [       
[       	         2   	[       _       Z       
Z       	      �  2   	Z       ]       _       
_       	      �  2   	_       ]       ]       
]       	       [  2   	Y       X       a       
Y          	   $  2   	Y       W       `       
X             �  2   	X       V       ^       
c             �  2   	W       T       \       
b               2   	V       R       [       
a             H  2   	T       Q       Z       
`               2   	R       P       _       
^             �  2   	Q       U       ]       
\             �  2   	P       S       U       
[             l  2   	U       S       S       
Z              5  2   	O       N       W       
O          	   �  2   	O       M       V       
N             �  2   	N       L       T       
Y             �  2   	M       J       R       
X             Y  2   	L       H       Q       
W             "  2   	J       G       P       
V             �  2   	H       F       U       
T             �  2   	G       K       S       
R             }  2   	F       I       K       
Q             F  2   	K       I       I       
P                2   	E       D       M       
E          	   �  2   	E       C       L       
D             �  2   	D       B       J       
O             j  2   	C       @       H       
N             3  2   	B       >       G       
M             �  2   	@       =       F       
L             �  2   	>       <       K       
J             �  2   	=       A       I       
H             W  2   	<       ?       A       
G                2   	A       ?       ?       
F              �  2   	;       :       C       
;          	   �  2   	;       9       B       
:             {  2   	:       8       @       
E             D  2   	9       6       >       
D               2   	8       4       =       
C             �  2   	6       3       <       
B             �  2   	4       2       A       
@             h  2   	3       7       ?       
>             1  2   	2       5       7       
=             �
  2   	7       5       5       
<              �
  2   	1       0       9       
1          	   �
  2   	1       /       8       
0             U
  2   	0       .       6       
;             
  2   	/       ,       4       
:             �	  2   	.       *       3       
9             �	  2   	,       )       2       
8             y	  2   	*       (       7       
6             B	  2   	)       -       5       
4             	  2   	(       +       -       
3             �  2   	-       +       +       
2              �  2   	'       &       /       
'          	   f  2   	'       %       .       
&             /  2   	&       $       ,       
1             �  2   	%       "       *       
0             �  2   	$               )       
/             �  2   	"              (       
.             S  2   	               -       
,               2   	       #       +       
*             �  2   	       !       #       
)             �  2   	#       !       !       
(              w  2   	              %       
          	   @  2   	              $       
             	  2   	              "       
'             �  2   	                      
&             �  2   	                     
%             d  2   	                     
$             -  2   	              #       
"             �  2   	              !       
              �  2   	                     
             �  2   	                     
              Q  2   	                     
          	     2   	                     
             �  2   	                     
             �  2   	                     
             u  2   	                     
             >  2   	                     
               2   	                     
             �  2   	       	              
             �  2   	              	       
             b  2   		                     
              +  2   	                     
           	   �  2   	                     
              �  2   	                     
              �  2   	                     
              O  2   	       
              
                2   	                     
              �   2   	
              	       
              �   2   	                     
              s   2   	                      
              <   2   	                       
                  7 6        9 `   @ p  p w #         � �  6          �  �  �  �  �  �  �  �  x  o  f  ]  A  8  /  &  
    �  �  �  �  �  �  �  �  �  �  e  \  S  J  .  %      �  �  �  �  �  �  �  �  �  �  w  n  R  I  @  7      	     �  �  �  �  �  �  �  �  v  m  d  [  ?  6  -  $    �  �  �  �  �  �  �  �  �  �    c  Z  Q  H  ,  #      �  �  �  �  �  �  �  �  �  ~  u  l  P  G  >  5        �  �  �  �  �  �  �  �  �  t  k  b  Y  =  4  +  "    �  �  �  �  �  �  �  �  �  �  }  a  X  O  F  *  !      �  �  �  �  �  �  �  �  �  |  s  j  N  E  <  3        �  �  �  �  �  �  �  �  �  r  i  `  W  ;  2  )       �  �  �  �  �  �  �  �  �  �  {  _  V  M  D  (        �  �  �  �  �  �  �  �  �  z  q  h  L  C  :  1        �
  �
  �
  �
  �
  �
  �
  �
  �
  p
  g
  ^
  U
  9
  0
  '
  
  
  �	  �	  �	  �	  �	  �	  �	  �	  �	  �	  y	  ]	  T	  K	  B	  &	  	  	  	  �  �  �  �  �  �  �  �  �  x  o  f  J  A  8  /    
    �  �  �  �  �  �  �  �  �  n  e  \  S  7  .  %       �  �  �  �  �  �  �  �  �  �  w  [  R  I  @  $      	  �  �  �  �  �  �  �  �    v  m  d  H  ?  6  -      �  �  �  �  �  �  �  �  �  �  l  c  Z  Q  5  ,  #    �  �  �  �  �  �  �  �  �  �  ~  u  Y  P  G  >  "        �  �  �  �  �  �  �  �  }  t  k  b  F  =  4  +      �  �  �  �  �  �       r      �         
    l    @%   % wB   �            <    " 78`   @%   % " �� �         r      �         
    l    @%   % wB   �        	    <    " 78`   @%   % " �� �         Q      �         
    K    �            4    @! % %  % w� �         r      �         
    l   �            S    @"    :& "   :&% % % w� �         :      �         
    4   Z S"  ;`
   � �         Y      �         
    S   Z S"  <`)   Z Y@!   '% % z� �         �      �         
    �    "      >"     >A`Y   �            S    @"    :& "   =&% % % w� �         L      �         
    F   Z S" ;`   T "  ;`
   � �         �   	   �         
    �   Z ST "  <" <A`H   Z Y@"     :& "    :&'% % z� �         y   
   �         
    s   "      >" 
   ?A`:   �            4    @7a  c b w� �         L      �         
    F   Z S" ;`   T "  ;`
   � �         �      �         
    �   Z ST "  <" <A`H   Z Y@"     :& "    =&'% % z� �         M      �         
    G    �            0    	   @%   % w� �         �      �         
    �    B   �        
    <    " 78`   @%   % " �B   �            <    " 78`   @%   % " �� �         