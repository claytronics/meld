meld fil       #                                                                                        	                init -o axioms�   start(), state(SN), !edge(B, W), edgetype(B, Type), 
			SN = 0 -o state(2), edgetype(B, 2), connect(host-id, 0)@B, findcount(0), 
			level(0).�   connect(J, L), !level(LN), edgetype(J, OldType), !edge(J, Weight), 
			!state(SN), !core(FN), L < LN -o edgetype(J, 2), inc-if-find(SN), 
			initiate(host-id, LN, FN, SN)@J.@   inc-if-find(State), findcount(N), State = 1 -o findcount(N + 1).$   inc-if-find(State), State != 1 -o 1.�   connect(J, L), !level(LN), !edgetype(J, NotBasic), !edge(J, Wj), 
			L >= LN, NotBasic != 0 -o initiate(host-id, LN + 1, Wj, 1)@J.b  initiate(J, L, F, S), level(LN), state(SN), core(FN), 
			inbranch(MV79), best-edge(MV80), best-wt(MV81) -o level(L), state(S), 
			core(F), inbranch(J), best-edge(host-id), best-wt(999999.9), do-test-if-find(S), 
			{B, W, Branch | !edge(B, W), edgetype(B, Branch), B != J, Branch = 2 | 
			edgetype(B, 2), inc-if-find(S), initiate(host-id, L, F, S)@B}.,   do-test-if-find(Find), Find = 1 -o dotest().(   do-test-if-find(State), State != 1 -o 1.�   dotest(), !edge(B, W), edgetype(B, Basic), test-edge(MV82), 
			!level(LN), !core(FN), Basic = 0 -o test-edge(B), edgetype(B, 0), 
			test(host-id, LN, FN)@B.<   dotest(), test-edge(MV83) -o test-edge(host-id), doreport().�   test(J, L, F), !level(LN), !core(FN), !test-edge(TestEdge), 
			L <= LN, F = FN -o maysendreject(TestEdge, J), {Basic | edgetype(J, Basic), 
			Basic = 0 | edgetype(J, 1)}.5   maysendreject(TestEdge, J), TestEdge = J -o dotest().O   maysendreject(TestEdge, J), !edge(J, MV84), TestEdge != J -o reject(host-id)@J.4   maysendreject(TestEdge, J), TestEdge = host-id -o 1.`   test(J, L, F), !edge(J, MV85), !level(LN), !core(FN), 
			L <= LN, FN != F -o accept(host-id)@J.J   accept(J), test-edge(MV86) -o test-edge(host-id), may-change-best-edge(J).�   may-change-best-edge(J), best-wt(BW), !edge(J, ThisW), best-edge(MV87), 
			ThisW < BW -o best-edge(J), best-wt(ThisW), doreport().V   may-change-best-edge(J), !best-wt(BW), !edge(J, ThisW), ThisW >= BW -o 
			doreport().d   reject(J) -o dotest(), {W, Basic | !edge(J, W), 
			edgetype(J, Basic), Basic = 0 | edgetype(J, 1)}.�   doreport(), findcount(FC), test-edge(Nil), state(SN), 
			!inbranch(In), !edge(In, MV88), !best-wt(BW), FC = 0, Nil = host-id -o 
			findcount(0), test-edge(host-id), state(2), report(host-id, BW)@In.   doreport() -o 1.�   report(J, W), !inbranch(InBranch), findcount(FindCount), InBranch != J -o 
			findcount(FindCount - 1), may-set-best-edge(J, W).j   may-set-best-edge(J, W), best-wt(BW), best-edge(MV89), W < BW -o 
			best-wt(W), best-edge(J), doreport().=   may-set-best-edge(J, W), !best-wt(BW), W >= BW -o doreport().s   report(J, W), !inbranch(InBranch), !state(SN), !best-wt(BW), 
			InBranch = J, SN != 1, W > BW -o do-change-root().�   report(J, W), !inbranch(InBranch), !state(SN), best-wt(BW), 
			InBranch = J, SN != 1, W <= BW, W = 999999.9, BW = 999999.9 -o 
			best-wt(999999.9), halt().t   report(J, W), !inbranch(InBranch), !state(SN), !best-wt(BW), 
			InBranch = J, SN != 1, W <= BW, W != 999999.9 -o 1.x   do-change-root(), !best-edge(B), !edge(B, MV90), edgetype(B, Branch), 
			Branch = 2 -o edgetype(B, 2), change-root()@B.�   do-change-root(), !best-edge(B), !edge(B, MV91), edgetype(B, Branch), 
			!level(LN), Branch != 2 -o edgetype(B, 2), connect(host-id, LN)@B."   change-root() -o do-change-root().           �              _init                                                               set-priority                                                        setcolor                                                             setedgelabel                                                         write-string                                                        add-priority                                                         schedule-next                                                       setColor2                                                           state                                                                edge                                                                 start                                                               edgetype                                                            connect                                                             findcount                                                           level                                                               initiate                                                            inc-if-find                                                         core                                                                inbranch                                                            best-wt                                                             best-edge                                                           do-test-if-find                                                      dotest                                                              report                                                              test                                                                test-edge                                                           accept                                                              maysendreject                                                       reject                                                              may-change-best-edge                                                 doreport                                                            may-set-best-edge                                                    do-change-root                                                       halt                                                                 change-root                                                              �                                     K      �          
    E   �� 
K        -   �   Y   �   �   '   	          ���@	          ���@  '   	           ���@	          ��� @�   8   	          ���@	          ��� @	          `ff@�   '   	          `ff@	          @33�?n   '   	           ���@	          ����?B   8   	          ���@	          @33�?	          ����?   7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                7 6       9 `   @ ,                 7 6        9 `   @ ,                7 6       9 `   @ ,                 7 6        9 `   @ ,                @         @ (    @ -   ��.A    @ (    @ (    @ -            @
   #         �      �      �        
 
    �    ��        
    �    A    �        	   �    ��            o    B @    @!     @(     " @     @     ��� �     	
�      �         
    �    ��            �    �" " >`�   �            �    B  �        	     �    B  �            o    ��            W    �@!      @!  @( ! ! ! "  �� �     	X      �         
    R    A   �        
    6    �@"    =& �� �     6      �         
    0    �"     <`
   � �     �      �         
    �    ��            �    �" " ?`}   �             w    B  "    <`O   �        	     I    B  @( "    =&!   "  � �     	�     �         
    �   ��        
    }   ��        
    e   ��        
    M   ��        
    5   ��        
       ��        
       �@!  @!  @!  @!   @( @-   ��.A @!  �   �        	    �    �" "  	8		`	b   �            \     A   @	!  	   			@	!  			@	( 	! 	! 	! 	" 
	
��������� �     ,      �         
    &    A   @� �     6      �         
    0    �"     <`
   � �     �   	   �         
    �    ��        
    �    ��            �    ��            �    ��        	   t    ��            \     A    @!  @!      @( ! ! " ��� �     	K   
   �         
    E    ��        
    -    �@( @�� �     �      �         
    �    ��            �    �" " B`�   �            �    �" " J`m   �            g    �@!  !  ;   �            5      A    @!      ��� �     :      �         
    4    �"  " 9`   @� �     [      �         
    U    �"  " 8`1   �        	     +    B @( " � �     	2      �         
    ,    �"  79`
   � �     �      �         
    �    ��        	         B  �            e    �" " B`A   �            ;    �" " K`   @( "  � �     	P      �         
    J    ��        
    2    �@( @!   �� �     �      �         
    �    ��        
    �    ��        	     h    B  "" L`B   �        
    <    �@!   @! @��� �     	l      �         
    f    ��            N    ��        	     6    B  "" O`   @� �     	}      �         
    w    �@U   �        	     O    B  �            5      A    @!      ��� �          �         
    �    ��        
    �    A    �        
    �    �" 79`�   �        
    �    ��            �    ��        	     q    B �            W    �@     @( @    @( ! " ���� �     	"      �         
        �� �     �      �         
    �    ��            p    �" "  8`L   �        
    F    �@"    :& @!   ! �� �     �      �         
    ~    ��        
    f    �" " L`B   �        
    <    �@!  @!   @��� �     R      �         
    L    ��            4    �" " O`   @� �     �      �         
    �    ��            �    �" "  9`f   �            `    �"    <`:   �            4    �" " N`   @ � �     �      �         
    �   @   ��.A�            �    �" "  9`�   �            {    �"    <`U   �        
    O    @   ��.A" " M`#   @-   ��.A @!�� �     �      �         
    �    �" .   ��.AK`�   �            �    �" "  9``   �            Z    �"    <`4   �            .    �" " M`
   � �     �      �          
    �    ��            t    ��        	     \    B �            B     A   @!     @"" �� �     	 �      �          
    �    ��            �    ��        	     �    B �            p    B "   <`H   �            B    �@!     @( ! " �� �      	(      �        " 
    "    �@ � �     "