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

load "034.txt";
K034 := filter(pols, 5^3*7^4);

load "044.txt";
K044 := filter(pols, 5^4*7^4);

load "045.txt";
K045 := filter(pols, -5^4*7^5);

