	var a,b,c,d,e,f,g,i,j,f,begi,k,t;	
	PROCedURe p;
		bEgIn
			if j=0 then i:=a;
			if j=1 then i:=b;
			if j=2 then i:=c;
			if j=3 then i:=d;
			if j=4 then i:=e;
			if j=5 then i:=g;

                end;

	procedure q;
		begin
			if j=0 then a:=i;
			if j=1 then b:=i;
			if j=2 then c:=i;
			if j=3 then d:=i;
			if j=4 then e:=i;
			if j=5 then g:=i;

                end;

65
	begin

		a:=0;
		begi:=1;
		while begi#6 do
			begin
				if begi#0 then read(t);
				k:=begi;
				f:=k;
				if k>0 then f:=k+1;
				while k>0 do
				begin
					 j:=k-1;
					call p;
					if i>t then 
						k:=1;
					if i<=t then
					begin
						j:=k;
						call q;	
					end;
					k:=k-1;
					f:=f-1;
				end;
				j:=f;
				i:=t;
				call q;
write(a);
write(b);
				begi:=begi+1;		
			end;
		write(b);
		write(c);
		write(d);
		write(e);
		write(g);
	end.