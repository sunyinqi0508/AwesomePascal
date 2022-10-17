	const a=10;
	var b,c,e;
	procedure p;
		var d;
		begin
			d:=20;
			c:=d/a;
			c:=c+b;
			if a<c then c:=c*2;
			write(c);
		end;

	begin
		read (b);
		while b#0 do
			begin
				e := b;
				e := 4*e/2/3+(3*1);
				call p;			
				write(e);
				read(b);
			end;
		b:=999999999;
		write(b);
	end.