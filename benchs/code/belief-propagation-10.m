meld fil    
   d   c   c   b   b   a   a   `   `   _   _   ^   ^   ]   ]   \   \   [   [   Z   Z   Y   Y   X   X   W   W   V   V   U   U   T   T   S   S   R   R   Q   Q   P   P   O   O   N   N   M   M   L   L   K   K   J   J   I   I   H   H   G   G   F   F   E   E   D   D   C   C   B   B   A   A   @   @   ?   ?   >   >   =   =   <   <   ;   ;   :   :   9   9   8   8   7   7   6   6   5   5   4   4   3   3   2   2   1   1   0   0   /   /   .   .   -   -   ,   ,   +   +   *   *   )   )   (   (   '   '   &   &   %   %   $   $   #   #   "   "   !   !                                                                                         	   	                                                                                                   
   
           	                init -o axiomsZ  start() -o belief(cons(-1.60944,cons(-1.60944,cons(-1.60944,cons(-1.60944,cons(-1.60944,nil)))))), update(), {B, Side | 
			!edge(B, Side) | neighbor-belief(B, cons(-1.60944,cons(-1.60944,cons(-1.60944,cons(-1.60944,cons(-1.60944,nil)))))), sent-neighbor-belief(B, cons(-1.60944,cons(-1.60944,cons(-1.60944,cons(-1.60944,cons(-1.60944,nil))))))}.   !update(), update() -o 1._   neighbor-belief(B, Belief), new-neighbor-belief(B, NewBelief) -o neighbor-belief(B, NewBelief).�   copy-messages(), !potential(Potential), belief(MyBelief) -o sum-messages(Potential), 
			{B, Belief | neighbor-belief(B, Belief) | neighbor-belief-old(B, Belief), neighbor-belief-copy(Belief), 
			neighbor-belief(B, Belief)}.R   sum-messages(L1), neighbor-belief-copy(L2) -o sum-messages(addfloatlists(L1, L2)).n   sum-messages(NewBelief), Normalized = normalize(NewBelief) -o update-messages(Normalized), belief(Normalized).�   check-residual(B, Delta, OutMessage), !edge(B, MV43), Delta > 1.e-4 -o update()@B, 
			new-neighbor-belief(host-id, OutMessage)@B.c   check-residual(B, Delta, OutMessage), !edge(B, MV44) -o new-neighbor-belief(host-id, OutMessage)@B.<  !update-messages(NewBelief), !edge(B, MV45), neighbor-belief-old(B, OldIn), sent-neighbor-belief(B, OldOut), 
			Cavity = normalize(divide(NewBelief, OldIn)), Convolved = normalize(convolve(cons(-0.0,cons(-2.0,cons(-4,cons(-6,cons(-8,cons(-2,cons(0,cons(-2,cons(-4,cons(-6,cons(-4,cons(-2,cons(0,cons(-2,cons(-4,cons(-6,cons(-4,cons(-2,cons(0,cons(-2,cons(-8,cons(-6,cons(-4,cons(-2,cons(0,nil))))))))))))))))))))))))), Cavity)), OutMessage = damp(Convolved, OldOut, 0.1) -o sent-neighbor-belief(B, OutMessage), check-residual(B, residual(OutMessage, OldOut), OutMessage).    update-messages(NewBelief) -o 1.   update() -o copy-messages().         �                !       �  !"      �  "#      �  #$       �  $%       �  %&          &'       �  '(      �  ()      �  )*      �  *+       �  +,          ,-       �  -.      �  ./      �  /0      �  01       �  12          23       �  34       �  45      �  56      �  67       �  78        08           D���   !    D���  !"    D���  "#    D���  #$    D���0$   0    ����?   0    ��6?   0       0      0      0      �              _init                                                               set-priority                                                        setcolor                                                            setedgelabel                                                        write-string                                                        add-priority                                                         schedule-next                                                       setColor2                                                            edge                                                                  potential                                                            pixel                                                                belief                                                               new-neighbor-belief                                                  sent-neighbor-belief                                                 neighbor-belief                                                      neighbor-belief-copy                                                 neighbor-belief-old                                                  start                                                                update                                                               copy-messages                                                        sum-messages                                                         update-messages                                                      check-residual                                                           �                         �5      �  �5   �
   d   +  �  �  A  �  �  W  	  �  m    �  �  5  �  �  K  �  �  a    �  w  )  �  �  ?  �  �  U    �  k    �  �  3  �  �  I  �  �  _    �  u  '  �  �  =  �  �  S    �  i    �    1  �  �  G  �
  �
  ]
  
  �	  s	  %	  �  �  ;  �  �  Q    �  g    �  }  /  �  �  E  �  �  [    �  q  #  �  �  9  �   �   O      @	   !   ����  !"   `�]��  "#   ���  #$   �����  $    V���    /  @	   !   @#��  !"   `,7�  "#   `5���  #$   ���  $    )?	�    �  @	   !    t���  !"   � ��  "#   @�!��  #$   �kC��  $   `e��    �  @	   !    �L�  !"    -���  "#   �W���  #$    ����  $    ��    E  @	   !    w�ؿ  !"    ����  "#   �[��  #$   ��  $   ��5�    �  @	   !   @ ��  !"   ����  "#   �A}��  #$   ��@�  $    8�    �  @	   !    P��  !"   @�
�  "#   @���  #$    �4�  $   ���    [  @	   !   � � �  !"    ����  "#    �3��  #$   �c���  $   � M��      @	   !   �Nc�  !"   ���	�  "#   ��� �  #$    ��  $   @#��    �  @	   !   �zN��  !"   @�8��  "#   `�"��  #$   ���  $    <���    q  @	   !   ���  !"   ����  "#   `�i��  #$   @C��  $    *��    #  @	   !   ����  !"   ���  "#   �J��  #$   `��  $   ���ۿ    �  @	   !   `��  !"    �N�  "#   ����  #$    �e�  $   ����    �  @	   !    �h��  !"   ����  "#   `I��  #$   `�w��  $    ����    9  @	   !   ��� �  !"    2���  "#    u<��  #$   ���  $   ����    �  @	   !   `
]�  !"    �i�  "#    3���  #$    ��  $   ��@�    �  @	   !   �-��  !"    5�  "#   �г��  #$   ����  $    ���    O  @	   !    �N�  !"   `�� �  "#   @���  #$   �Ր�  $    ���      @	   !   `��	�  !"    s�  "#   ���  #$   @Y#�  $    B�    �  @	   !   ��<�  !"    $��  "#   `('��  #$    ��  $    ��    e  @	   !   �y�  !"   ����  "#   ��`��  #$    ��  $   ��}�      @	   !    ��  !"   ��y�  "#   �~��  #$   �N���  $   ���߿    �  @	   !   �{x�  !"   �����  "#   �M���  #$   `�q��  $   ��G��    {  @	   !   �'F�  !"   �7�  "#   ���  #$   `*��  $   ��_�    -  @	   !    �a��  !"   @���  "#    ���  #$   �[���  $   @��    �  @	   !    ���  !"   ����  "#   �L��  #$   `s �  $    �i��    �  @	   !   ��	�  !"   �?��  "#   �ă��  #$   �	?�  $   �N��    C  @	   !    �  !"   �� �  "#   �����  #$   `���  $   `�]�    �  @	   !   `SK�  !"    ql�  "#    ��  #$   �N]�  $   ���    �  @	   !   ���
�  !"    ���  "#    jj��  #$    ��  $   ��y�    Y  @	   !    "�  !"   �E�  "#   ��E�  #$   @����  $   `��      @	   !   ���  !"   ���  "#   �����  #$   �[�  $    �X�    �  @	   !   `��  !"    7���  "#   �H���  #$   �d��  $   �v��    o  @	   !   �\A�  !"    Aq�  "#   @�_�  #$    ����  $   `w��    !  @	   !    ��  !"   ��� �  "#   �b���  #$    0��  $   ��H�    �  @	   !   @��
�  !"   ��+�  "#   �e���  #$   ���  $   �3��    �  @	   !   ��y��  !"   ��`��  "#    �G��  #$   @�.��  $   ��
�    7  @	   !   ����  !"   �i��  "#   �^��  #$    ����  $   `��    �  @	   !   `j��  !"    �r
�  "#    (8�  #$   ����  $   ���    �  @	   !   ��h �  !"   �>���  "#   @�.��  #$   �c���  $   �����    M  @	   !   �t�  !"   ��x�  "#    t���  #$   ��  $   `��    �  @	   !    pk��  !"   �+���  "#    ���  #$   ��]��  $    /��    �  @	   !   �R�  !"   @��	�  "#   @�� �  #$   ����  $    !��    c  @	   !    �  !"   `I@�  "#   ����  #$   ��	�  $   �6��      @	   !    k�  !"    5���  "#    fZ��  #$    ���  $   �����    �  @	   !   �A���  !"   �[=��  "#   @����  #$   @�1��  $   `���    y  @	   !   @���  !"   ��r�  "#    A���  #$   �`��  $   ���    +  @	   !   `y��  !"   �m��  "#   @���  #$   ��P��  $    j��    �  @	   !   @�p
�  !"   `���  "#   @�Y��  #$   ���  $   ����    �  @	   !    �J��  !"   �H���  "#   ��3��  #$   �w���  $   @�� �    A  @	   !    �`�  !"    2l�  "#    ����  #$    ��  $    b<�    �  @	   !   @���  !"    +��  "#    ���  #$   �����  $   �����    �  @	   !    ��  !"   �o� �  "#   �dp��  #$   @��  $    z��    W  @	   !   ��D �  !"   @�Y��  "#   @o*��  #$   @����  $   `S���    	  @	   !   ��	�  !"   ����  "#    ����  #$   `�<�  $   ����    �  @	   !   �u��  !"   �ZS��  "#    �	��  #$    /���  $   ��v��    m  @	   !    �Q�  !"   @���  "#   �f���  #$   @33�  $   ���      @	   !   �"� �  !"    � ��  "#   `�E��  #$   � j��  $   ྎ��    �  @	   !   `���  !"   ��8�  "#   �����  #$   @J�  $    �3	�    �  @	   !   @���  !"    ��  "#   �x���  #$   ��Z�  $   �S��    5  @	   !   `VZ�  !"   `H���  "#    �=��  #$    ����  $   �%���    �  @	   !   @%�  !"   �!+�  "#   ��T��  #$   �#��  $   ��)�    �  @	   !    �P�  !"    ����  "#   �h���  #$   �A���  $   ����    K  @	   !    �M�  !"   �n�  "#    ���  #$   ��\�  $   �)��    �
  @	   !   `��  !"   �� �  "#   �����  #$   ���  $   �C�    �
  @	   !   �3��  !"   `����  "#   �!���  #$   �t��  $   ����    a
  @	   !   ��
��  !"   ��Y��  "#   �m���  #$    <���  $   �
��    
  @	   !   �C���  !"   �"���  "#    �;��  #$    ����  $   `�� �    �	  @	   !   @�  !"   `���  "#   `LO��  #$   ��u �  $   @=��    w	  @	   !    v���  !"   @����  "#   �R���  #$   @����  $   �}�    )	  @	   !   @�R��  !"   ����  "#   �=3��  #$    n���  $   `ω �    �  @	   !   ����  !"   `����  "#   �u��  #$    L���  $   �"2��    �  @	   !    #a�  !"   `\8 �  "#   �+��  #$   ����  $   `|�    ?  @	   !    p�  !"   `^ �  "#   �u��  #$   ����  $   `*��    �  @	   !   ��  !"   �����  "#   `����  #$   �@<��  $    ����    �  @	   !   `��  !"   �a���  "#   `����  #$    ���  $    ;��    U  @	   !    ���  !"   `�d �  "#   ��A��  #$   `��  $   ��1�      @	   !    ���  !"    /��  "#    �_ �  #$   `w��  $    ���    �  @	   !   �&N�  !"   ��$�  "#   �����  #$    Ti�  $    ��	�    k  @	   !   `���  !"   @�Y�  "#   �P%��  #$    Wx�  $    ^�      @	   !   ��w�  !"   �L!�  "#   ����  #$    v�  $   `���    �  @	   !   ����  !"    A���  "#    f=��  #$   �����  $   ��� �    �  @	   !    2��  !"   @�;�  "#   �>���  #$   `b��  $   �%	�    3  @	   !    �F�  !"    F%�  "#   �G'��  #$   औ�  $   ��
�    �  @	   !   `.b�  !"   ��'�  "#   �=���  #$    eY�  $   �+�	�    �  @	   !   �`� �  !"   ���  "#    LC��  #$   ��s��  $   ����    I  @	   !   �t��  !"   �p��  "#   `ws��  #$   �9� �  $    ��    �  @	   !    �G�  !"   � $�  "#   `s ��  #$   �]n�  $    ��	�    �  @	   !   �+���  !"    ��  "#     c��  #$   �����  $   �o��    _  @	   !    \,�  !"   ���  "#   �����  #$   �*u�  $   ��m�      @	   !   ���  !"   �$���  "#    �j�  #$    #��  $   `�N�    �  @	   !   �P��  !"   @����  "#   �4c��  #$   �} ��  $    ǝ��    u  @	   !   `V���  !"   @To��  "#   �GD��  #$   �E��  $   �!� �    '  @	   !   ����  !"   ���  "#   @�H��  #$    ���  $   `;��    �  @	   !   @@0��  !"    ����  "#   ��s��  #$   �"��  $   �\��    �  @	   !    ���  !"   �����  "#    �f�  #$   @}�  $    �I�    =  @	   !   ����  !"    p��  "#   �9i��  #$   ��� �  $   �f��    �   @	   !    j�޿  !"   �@<��  "#   �Td�  #$   @�*�  $   `\x�    �   @	   !   ���  !"   `���  "#    �]�  #$    
�
�  $    1�    S   @	   !   �ט�  !"   �����  "#   �z���  #$   `e_��  $   �E��       
�  d   �  �  �  f  8  
  �  �  �  [  -  �  �  �  Q    �  �  u  G    �  �  t  =    �  �  a  3    �  �  `  )  �  �  �  M    �  �  �  L    �
  �
  p
  9
  
  �	  �	  o	  8	  	  �  �  \  %  �  �  �  [  $  �  �    H    �  �  ~  G    �  �  k  4  �  �  �  j  3  �  �  �  W     �  �  �  h  :    �   �   �   T   &          b      Y       
	   	   �  )   c      a      X       
	      �  )   b      `      W       
	      �  )   a      _      V       
	      j  )   `      ^      U       
	      <  )   _      ]      T       
	        )   ^      \      S       
	      �  )   ]      [      R       
	      �  )   \      Z      Q       
	      �      [      P       
	       _  )   X      c      O       
   	   1  2   Y      W      b      N       
      �  2   X      V      a      M       
      �  2   W      U      `      L       
      �  2   V      T      _      K       
      U  2   U      S      ^      J       
        2   T      R      ]      I       
      �  2   S      Q      \      H       
      �  2   R      P      [      G       
      y  )   Q      Z      F       
       K  )   N      Y      E       
   	     2   O      M      X      D       
      �  2   N      L      W      C       
      �  2   M      K      V      B       
      x  2   L      J      U      A       
      A  2   K      I      T      @       
      
  2   J      H      S      ?       
      �  2   I      G      R      >       
      �  2   H      F      Q      =       
      e  )   G      P      <       
       7  )   D      O      ;       
   	   	  2   E      C      N      :       
      �  2   D      B      M      9       
      �  2   C      A      L      8       
      d  2   B      @      K      7       
      -  2   A      ?      J      6       
      �  2   @      >      I      5       
      �  2   ?      =      H      4       
      �  2   >      <      G      3       
      Q  )   =      F      2       
       #  )   :      E      1       
   	   �  2   ;      9      D      0       
      �  2   :      8      C      /       
      �  2   9      7      B      .       
      P  2   8      6      A      -       
        2   7      5      @      ,       
      �
  2   6      4      ?      +       
      �
  2   5      3      >      *       
      t
  2   4      2      =      )       
      =
  )   3      <      (       
       
  )   0      ;      '       
   	   �	  2   1      /      :      &       
      �	  2   0      .      9      %       
      s	  2   /      -      8      $       
      <	  2   .      ,      7      #       
      	  2   -      +      6      "       
      �  2   ,      *      5      !       
      �  2   +      )      4              
      `  2   *      (      3             
      )  )   )      2             
       �  )   &      1             
   	   �  2   '      %      0             
      �  2   &      $      /             
      _  2   %      #      .             
      (  2   $      "      -             
      �  2   #      !      ,             
      �  2   "             +             
      �  2   !            *             
      L  2                )             
        )         (             
       �  )         '             
   	   �  2               &             
      �  2               %             
      K  2               $             
        2               #             
      �  2               "             
      �  2               !             
      o  2                             
      8  2                            
        )               
       
       �  )               	       
   	   �  2                            
      n  2                            
      7  2                            
         2                            
      �  2                            
      �  2                            
      [  2                            
      $  2         
                   
      �  )                       
       �                  
    	   �  )   	                  
       l  )                     
       >  )                     
         )                     
       �   )                     
       �   )                     
       �   )                     
       X   )                      
       *             
      
           @   0 � 0         �      �      � �    �0 @  "    D���  "#    D���  #$    D���  $%    D���  %    D��� @A   �  ;    �0!@0  0   @0  0   �� �     &      �       �0 �     �0!��     >      � 8    �0 � *    B  0!@0   0�� �     �      � �    �0 �	  {    �0!� m    �0"@0  L   � F    �0#@0  0@0 @0  0���� �     	=      � 7    �0 � )    �0!@    0# �� �     5      � /    �0    @0! @0! � �     [      � U    �0 �!    `<   �  6    B  0!@0#  @0 0 0#  � �     ?      � 9    �0 �  +    B  0!@0 0 0#  � �     �   	   �  �    �0 �  �    �0!� �    B 0"� r    B 0#    $     % & '   @	0  	0(			@	0  	 
(0*	0(			���        
   �     �0 � �           �     �0 @� �     