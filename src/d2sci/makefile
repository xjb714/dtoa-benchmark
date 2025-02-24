lib:
	icpx    -O3 -fPIC -c -mavx512f -march=native d2sci.cpp       -o d2sci.o
	clang++ -O3 -fPIC -c -mavx512f -march=native d2sci_32.cpp    -o d2sci_32.o
	clang++ -O3 -fPIC -c -mavx512f -march=native d2sci_32v.cpp   -o d2sci_32v.o
	icpx    -O3 -fPIC -c -mavx512f -march=native d2s_32v.cpp     -o d2s_32v.o
	icpx    -O3 -fPIC -c -mavx512f -march=native d2s.cpp         -o d2s.o
	icpx    -O3 -fPIC -c -mavx512f -march=native d2sci_table.cpp -o d2sci_table.o
	ar rcs libd2sci2.a d2sci_table.o d2sci.o d2sci_32.o d2sci_32v.o d2s_32v.o d2s.o

	# ubuntu 24.04.2 (1)icpx 2025.0.4  (2)clang++ 18.1.3
demo:
	g++ demo.cpp -L. -ld2sci2 -o demo
	./demo