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

load "245.txt";
K245 := filter(pols, -3^2*5^4*7^5);

load "247.txt";
K247 := filter(pols, -3^2*5^4*7^7);

load "265.txt";
K265 := filter(pols, -3^2*5^6*7^5);

load "267.txt";
K267 := filter(pols, -3^2*5^6*7^7);

load "445.txt";
K445 := filter(pols, -3^4*5^4*7^5);

load "447.txt";
K447 := filter(pols, -3^4*5^4*7^7);

load "465.txt";
K465 := filter(pols, -3^4*5^6*7^5);

load "467.txt";
K467 := filter(pols, -3^4*5^6*7^7);

load "645.txt";
K645 := filter(pols, -3^6*5^4*7^5);

load "647.txt";
K647 := filter(pols, -3^6*5^4*7^7);

load "665.txt";
K665 := filter(pols, -3^6*5^6*7^5);

load "667.txt";
K667 := filter(pols, -3^6*5^6*7^7);
