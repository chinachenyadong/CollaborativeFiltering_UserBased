target:
	g++ UserCF.cpp
	./a.out 5 5
	g++ cal_precision_recall.cpp
	./a.out

clean:
	rm a.out
