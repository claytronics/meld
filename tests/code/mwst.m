meld fil       #                                                                                        	                init -o axioms   start(), state(SN), !edge(B, W), edgetype(B, Type), 
			SN = 0 -o state(2), edgetype(B, 2), connect(host-id, 0)@B, findcount(0), 
			level(0).­   connect(J, L), !level(LN), edgetype(J, OldType), !edge(J, Weight), 
			!state(SN), !core(FN), L < LN -o edgetype(J, 2), inc-if-find(SN), 
			initiate(host-id, LN, FN, SN)@J.@   inc-if-find(State), findcount(N), State = 1 -o findcount(N + 1).$   inc-if-find(State), State != 1 -o 1.   connect(J, L), !level(LN), !edgetype(J, NotBasic), !edge(J, Wj), 
			L >= LN, NotBasic != 0 -o initiate(host-id, LN + 1, Wj, 1)@J.b  initiate(J, L, F, S), level(LN), state(SN), core(FN), 
			inbranch(MV79), best-edge(MV80), best-wt(MV81) -o level(L), state(S), 
			core(F), inbranch(J), best-edge(host-id), best-wt(999999.9), do-test-if-find(S), 
			{B, W, Branch | !edge(B, W), edgetype(B, Branch), B != J, Branch = 2 | 
			edgetype(B, 2), inc-if-find(S), initiate(host-id, L, F, S)@B}.,   do-test-if-find(Find), Find = 1 -o dotest().(   do-test-if-find(State), State != 1 -o 1.   dotest(), !edge(B, W), edgetype(B, Basic), test-edge(MV82), 
			!level(LN), !core(FN), Basic = 0 -o test-edge(B), edgetype(B, 0), 
			test(host-id, LN, FN)@B.<   dotest(), test-edge(MV83) -o test-edge(host-id), doreport().¬   test(J, L, F), !level(LN), !core(FN), !test-edge(TestEdge), 
			L <= LN, F = FN -o maysendreject(TestEdge, J), {Basic | edgetype(J, Basic), 
			Basic = 0 | edgetype(J, 1)}.5   maysendreject(TestEdge, J), TestEdge = J -o dotest().O   maysendreject(TestEdge, J), !edge(J, MV84), TestEdge != J -o reject(host-id)@J.4   maysendreject(TestEdge, J), TestEdge = host-id -o 1.`   test(J, L, F), !edge(J, MV85), !level(LN), !core(FN), 
			L <= LN, FN != F -o accept(host-id)@J.J   accept(J), test-edge(MV86) -o test-edge(host-id), may-change-best-edge(J).   may-change-best-edge(J), best-wt(BW), !edge(J, ThisW), best-edge(MV87), 
			ThisW < BW -o best-edge(J), best-wt(ThisW), doreport().V   may-change-best-edge(J), !best-wt(BW), !edge(J, ThisW), ThisW >= BW -o 
			doreport().d   reject(J) -o dotest(), {W, Basic | !edge(J, W), 
			edgetype(J, Basic), Basic = 0 | edgetype(J, 1)}.È   doreport(), findcount(FC), test-edge(Nil), state(SN), 
			!inbranch(In), !edge(In, MV88), !best-wt(BW), FC = 0, Nil = host-id -o 
			findcount(0), test-edge(host-id), state(2), report(host-id, BW)@In.   doreport() -o 1.   report(J, W), !inbranch(InBranch), findcount(FindCount), InBranch != J -o 
			findcount(FindCount - 1), may-set-best-edge(J, W).j   may-set-best-edge(J, W), best-wt(BW), best-edge(MV89), W < BW -o 
			best-wt(W), best-edge(J), doreport().=   may-set-best-edge(J, W), !best-wt(BW), W >= BW -o doreport().s   report(J, W), !inbranch(InBranch), !state(SN), !best-wt(BW), 
			InBranch = J, SN != 1, W > BW -o do-change-root().   report(J, W), !inbranch(InBranch), !state(SN), best-wt(BW), 
			InBranch = J, SN != 1, W <= BW, W = 999999.9, BW = 999999.9 -o 
			best-wt(999999.9), halt().t   report(J, W), !inbranch(InBranch), !state(SN), !best-wt(BW), 
			InBranch = J, SN != 1, W <= BW, W != 999999.9 -o 1.x   do-change-root(), !best-edge(B), !edge(B, MV90), edgetype(B, Branch), 
			Branch = 2 -o edgetype(B, 2), change-root()@B.   do-change-root(), !best-edge(B), !edge(B, MV91), edgetype(B, Branch), 
			!level(LN), Branch != 2 -o edgetype(B, 2), connect(host-id, LN)@B."   change-root() -o do-change-root().           ð                  _init                                                               set-priority                                                        setcolor                                                             setedgelabel                                                         write-string                                                        add-priority                                                         schedule-next                                                       setColor2                                                           state                                                                edge                                                                 start                                                               edgetype                                                            connect                                                             findcount                                                           level                                                               initiate                                                            inc-if-find                                                         core                                                                inbranch                                                            best-wt                                                             best-edge                                                           do-test-if-find                                                      dotest                                                              report                                                              test                                                                test-edge                                                           accept                                                              maysendreject                                                       reject                                                              may-change-best-edge                                                 doreport                                                            may-set-best-edge                                                    do-change-root                                                       halt                                                                 change-root                                                              Ð                                                                                                                                                                                 5                 
    /    
K        -   Â   Y   î      '   	           @	          ÀÌÌ@  '   	            @	          ÀÌÌ @×   8   	          ÀÌÌ@	          ÀÌÌ @	          `ff@   '   	          `ff@	          @33û?n   '   	           ÀÌÌ@	           ñ?B   8   	          ÀÌÌ@	          @33û?	           ñ?   7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,              w 7 6       9 `   @ ,               w 7 6        9 `   @ ,              w 7 6       9 `   @ ,               w 7 6        9 `   @ ,              w @       w @ (  w @ -   À.A  w @ (  w @ (  w @ -          w @
 w #         ð *   Ì  µ      r  [  E  .      ë  Ô  ¾  §    z  d  M  7     
  ó  Ý  Æ  °      l  T  C  2      ë   Ú   ¿   ®         q   V   E        ½               
 
    ·             
                      	                    l     @    z@!     z@(     " @     w@     w ð         	
ç                
    á                 Ê    " " >`§                ¡               	                        k                 T    @!      z@!  w@( ! ! ! "   ð         	W                
    Q                
    4    @"    =& z ð         5                
    /    "     <`
    ð         ¿                
    ¹                 ¢    " " ?`                 y      "    <`P            	     J      @( "    =&!   "   ð         	               
                
    n            
    W            
    @            
    )            
                
    û    @!  z@!  z@!  z@!   z@( z@-   À.A z@!  w            	        " "  	8		`	a                 [        @	!  	   	z	@	!  	w	@	( 	! 	! 	! 	" 
	
ð ð         ,                
    &       @w ð         5                
    /    "     <`
    ð         Ô   	             
    Î             
    ·                                                	   r                  [         @!  z@!      z@( ! ! "  ð         	G   
             
    A             
    *    @( z@w ð         Ô                
    Î                 ·    " " B`                    " " J`k                e    @!  !  w;                 5          @!      zð ð         8                
    2    "  " 9`   @w ð         [                
    U    "  " 8`2            	     ,     @( "  ð         	1                
    +    "  79`
    ð                         
                 	     ~                   c    " " B`@                :    " " K`   @( "   ð         	L                
    F             
    /    @( z@!   w ð                         
                 
    |             	     e      "" L`>            
    8    @!   z@! z@w ð         	j                
    d                 M             	     6      "" O`   @w ð         	|                
    v    @wV            	     P                    5          @!      zð ð         ý                
    ÷             
    à                 
    Ã    " 79`¢            
                                  	     n                  S    @     z@( z@    z@( ! "  ð         	!                
         ð                         
                     l    " "  8`I            
    C    @"    :& z@!   ! w ð         ~                
    x             
    a    " " L`>            
    8    @!  z@!   z@w ð         O                
    I                 2    " " O`   @w ð         £                
                         " "  9`c                ]    "    <`8                2    " " N`   @ w ð         É                
     Ã       À.A             ¢    " "  9`                y    "    <`T            
     N        À.A" " M`!   @-   À.A z@!w ð         ¶                
    °    " .   À.AK`                    " "  9`^                X    "    <`3                -    " " M`
    ð                          
                     t             	     ]                   B        @!     z@""  ð         	 ¾                 
    ¸                 ¡             	                       o     "   <`F                @    @!     z@( ! "  ð          	&               " 
         @ w ð         "