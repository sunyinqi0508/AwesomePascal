	const a=10;
	var b,c;
	procedure p;
		var d;
		begin
			d:=20;
write(d);
			c:=d/a;
write(c);
			c:=c+b;
write(c);
			if a<c then c:=c*2;
write(c);
                end;
	begin
		read (b);
		while b#0 do
			begin
				call p;			
				write(c);
				read(b);
				b := 4*b/2/3+(3*1);
write(b);
			end;
		b:=1;
write(b);
	end.