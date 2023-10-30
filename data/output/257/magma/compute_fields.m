Q := Rationals();
R<t> := PolynomialRing(Q);

function trivial_quartic_subfield(f);
	return Coefficient(f,7) eq 0 and Coefficient(f,5) eq 0
		and Coefficient(f,3) eq 0 and Coefficient(f,1) eq 0;
end function;

function filter(pols,disc);
	pols := [R!Reverse(f) : f in pols];
	Ks := [NumberField(f) : f in pols | IsIrreducible(f)];
	//Ks := [NumberField(f) : f in pols | not trivial_quartic_subfield(f) and IsIrreducible(f)];
    	Ks := [K : K in Ks | Discriminant(Integers(K)) eq disc];
	return Ks;
end function;

function uptoiso(L);
	Ls := {@ @};
	for K in L do
		if not exists { K2 : K2 in Ls | IsIsomorphic(K,K2) } then
			Include(~Ls, K);
		end if;
	end for;
	return Ls;
end function;

load "045.txt";
K045 := filter(pols, -5^4*7^5);

load "047.txt";
K047 := filter(pols, -5^4*7^7);

load "065.txt";
K065 := filter(pols, -5^6*7^5);

load "067.txt";
K067 := filter(pols, -5^6*7^7);

load "645.txt";
K645 := filter(pols, -2^6*5^4*7^5);

load "647.txt";
K647 := filter(pols, -2^6*5^4*7^7);

load "665.txt";
K665 := filter(pols, -2^6*5^6*7^5);

load "667.txt";
time K667 := filter(pols, -2^6*5^6*7^7);

load "1245.txt";
K1245 := filter(pols, -2^12*5^4*7^5);

load "1247.txt";
K1247 := filter(pols, -2^12*5^4*7^7);

load "1265.txt";
K1265 := filter(pols, -2^12*5^6*7^5);

//load "1267.txt";
//K1267 := filter(pols, -2^12*5^6*7^7);

load "1445.txt";
K1445 := filter(pols, -2^14*5^4*7^5);

load "1447.txt";
K1447 := filter(pols, -2^14*5^4*7^7);

load "1465.txt";
K1465 := filter(pols, -2^14*5^6*7^5);

load "1467.txt";
time K1467 := filter(pols, -2^14*5^6*7^7);




