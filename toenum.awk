BEGIN {
	Enum= 0;
}

/\/\*--- Start of typedef enum/ {
	printf "%s", "typedef enum {";
	EnumCnt= 0;
	Enum= 1;
	next;
}
 
/typedef/ {
	if (Enum) {
		next;
	}
}

/\/\*--- End of }/ {
	if (Enum) {
		printf "\n%s\n", substr($0,14,length($0)-18);
		Enum= 0;
	}
	else
		print $0;
	next;
}

/#define/{	
	if (Enum) {
		if (EnumCnt)
			printf ",\n";
		else
			printf "\n";
		printf "\t/*%2d*/\t%s", EnumCnt++, $2;
	}
	else
		print $0;
	next;
}

{
	print $0
}
