BEGIN { Enum=0; }
/typedef enum {/ {
	print "/*--- Start of " $0 "---*/"
	Enum= 1;
	EnumCnt=0; 
	next;
}
/}/ {
	if (Enum) {
		print "typedef int " $2;
		print "/*--- End of " $0 "---*/" ;
		Enum= 0;
	}
	else 
		print $0;
	next;
}

/\/\*[ 1-9][0-9]\*\// {
	if (Enum) {
		l= length($NF);
		if (substr($NF,l,1) == ",")
			s= substr($NF,1,l-1);
		else
			s= $NF;
		printf "#define\t%s\t%d\n",s,EnumCnt;
		EnumCnt++;
	}
	else
		print $0;
	next;
}

{
	print $0;
}
