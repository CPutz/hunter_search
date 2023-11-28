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



load "1467_4ds1.txt";
time K1467 := filter(pols, -2^14*5^6*7^7);




