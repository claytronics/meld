meld fil    
   
       	                  
                                 	      	                init -o axioms�   discover(J), first-time() -o father(J), check-if-only-neighbor(J), 
			setcolor(255, 255, 0), {Q, Flag | !edge(Q), flag(Q, Flag), unvisited(Q), 
			Q != J | unvisited(Q), visited(host-id)@Q, flag(Q, 1)}.+   discover(J), !edge(J) -o return(host-id)@J..   check-if-only-neighbor(J), !unvisited(X) -o 1.Y   check-if-only-neighbor(Node), !edge(Node) -o return(host-id)@Node, setcolor(0, 255, 255).9   return(Q), unvisited(K), !edge(K) -o discover(host-id)@K.r   return(Q), !father(Father), !edge(Father), Father != host-id -o 
			return(host-id)@Father, setcolor(0, 255, 255).-   return(Q), !father(A), A = host-id -o stop().5   visited(K), unvisited(K), !edge(K) -o ack(host-id)@K.U   ack(J), flag(J, MV4), flag(J2, MV6), J != J2, 
			MV6 = 1 -o flag(J, 0), flag(J2, 1).4   ack(J), flag(J, MV5) -o flag(J, 0), return(host-id).           �              _init                                                               set-priority                                                        setcolor                                                             setedgelabel                                                         write-string                                                        add-priority                                                         schedule-next                                                       setColor2                                                            edge                                                                flag                                                                unvisited                                                           father                                                              discover                                                            visited                                                             return                                                              check-if-only-neighbor                                               stop                                                                ack                                                                  first-time                                                               �                     �      �  �   �
b  
   1   t   �      �     �  Z  �  �  +   	        
        	       
        >   	       
      	       
      	       
      �  +   	       
      	        
        �  +   	       
      	        
        c  >   	       
      	       
      	       
         >   	       
      	       
      	       
      �   +   		       
	   	   	       
      �   +   		       
	   	   	       
      }   >   	       
      	       
      	       
      :   0      	       
      		       
	   	      @   0 � 0         �      �      � �    �0 � �    �0!@0  @0  @0�    0�   0    {   �  u    �0"�#  `^   �	 X    B 0#�
 H    B 0$@
0  @0 0& @	0  0   ����� �     8      � 2    �0 �  $    B  0!@0 0#  � �     &      �      �0 �
      �0!� �     
Y      � S    �0 �  E    B  0!@0 0#  @0     0�   0�   � �     H      � B    �0 �
 4    �0!�  &    B 0"@0 0$ �� �     
t      � n    �0 �  `    �0!�" `K   �  E    B 0"@0 0$ @0     0�   0�   � �     9      � 3    �0 �  %    �0!�" `   @� �     J      � D    �0 �
 6    B  0!�  &    B  0"@0 0$  �� �     
�   	   � �    �0 �	 r    B  0!�	 b    �0"�#   `K   �#   `:   @	0   0    @	0  0   ��� �     	K   
   � E    �0 �	 7    B  0!@	0   0    @0 �� �     	