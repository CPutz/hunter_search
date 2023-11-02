Q := Rationals();
R<t> := PolynomialRing(Q);

function filter(pols,disc);
	pols := [R!Reverse(f) : f in pols];
	Ks := [NumberField(f) : f in pols | IsIrreducible(f)];
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
