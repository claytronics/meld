meld fil                                                 init -o axioms  start() -o pagerank(1.0), updates(0), update(), 
			{B, W | !input(B, W) | input-rank(B, 1.0)}, {B, W | 
			!output(B, W) | output-rank(B, 0.0)}, [ COUNT => C,  | B, W | 
			!output(B, W) | !numOutput(C)], [ COUNT => C,  | B, W | !input(B, W) | 
			!numInput(C)].>   new-input-rank(B, V), input-rank(B, OldV) -o input-rank(B, V).A   new-output-rank(B, V), output-rank(B, OldV) -o output-rank(B, V)._   add-input-ranks(Total) -o [ SUM => V,  | B | input-rank-copy(B, V) | 
			sum-ranks(V + Total)].r  sum-ranks(Acc), pagerank(Old), V = (0.15 / float(world)) + ((1.0 - 0.15) * Acc) -o pagerank(V), 
			{B, W, O | !output(B, W), output-rank(B, O), (fabs((O - V)) * W) > 1.e-5 | 
			update()@B, new-input-rank(host-id, V)@B, output-rank(B, O)}, {B, W, O | !output(B, W), 
			output-rank(B, O), (fabs((O - V)) * W) <= 1.e-5 | new-input-rank(host-id, V)@B, output-rank(B, O)}.   !update(), update() -o 1.�   update(), updates(N) -o add-input-ranks(0.0), updates(N + 1), 
			{B, V, W | input-rank(B, V), !input(B, W) | input-rank(B, V), 
			input-rank-copy(B, V * W), new-output-rank(host-id, V)@B}.        9   0    @33�?    0    ����>   0      0       �?   �   2   �  !        `
   0 	 !!`   �  	      � 0	
!                                          _init                                                                                              set-priority                                                                                        setcolor                                                            	                              setedgelabel                                                        	                               write-string                                                                                       add-priority                                                                                         schedule-next                                                                                       setColor2                                                                                          input                                                                                              output                                                                                             pagerank                                                                                          input-rank                                                                                           start                                                                                                update                                                                                            new-input-rank                                                                                     add-input-ranks                                                                                    sum-ranks                                                                                         input-rank-copy                                                                                   output-rank                                                                                       new-output-rank                                                                                     updates                                                                                              numOutput                                                                                            numInput                                                               Y@�                         �      �  �   �
            �   ,   W   @ 0     �        �?      @   �   @ 0     �        �?      @   �   @ 0     �        �?      @   |   @	 0     �        �?      @   @	 0     �        �?      @   @	 0     �        �?      @      
m        �   L   }     F   	         �?	         �?	         �?	         �?         �?
  ,             �?         �?         �?�   9             �?         �?	         �?	         �?�   S             �?	          �?	         �?	         �?	         �?         �?C   9             �?         �?         �?	         �?   @   0 � 0         �      �      � �    �0 @
0    @0     @+   �  %    �0!@0  0   �/   �	  )    �0!@0  0         �0 !        )   �	      �0"�!!   �@0! 0 !        )   �      �0"�!!   �@0! � �     >      � 8    �0 � *    B  0!@0   0 �� �     >      � 8    �0 � *    B  0!@0   0 �� �     P      � J    �0 0 !        -   �     �0"�!!��@�!   � �     e     � _   �0 �
 Q   �0!	"   �"#    � $      �?    �$%  �#%&@
0& �   �	  �    �0'� t    B 0(�& 0
	! 0	) �)*�*+   `=   @	0* 	
@	0 	0&	0* 	
@	0  	0			��}   �	  w    �0'� i    B 0(�& 0
	! 0	) �)*�*+   `2   @	0 	0&	0* 	
@	0  	0			���� �     
&      �       �0 �     �0!��     �      � �    �0 � �    �0!@0          @�     j   � d    �0"�  V    B 0#@0  0@0  �@0 00% ���� �     