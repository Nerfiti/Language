all:
	g++ au_stack.cpp Backend.cpp MyGeneralFunctions.cpp Tree.cpp treeSyntaxAnalyzer.cpp logs.cpp -o Backend.out
	./Backend.out 

debug:
	g++ -g au_stack.cpp Backend.cpp MyGeneralFunctions.cpp Tree.cpp treeSyntaxAnalyzer.cpp logs.cpp -o Backend.out
	gdb ./Backend.out