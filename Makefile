%: loader/%.cpp
	g++ -std=c++17 $< -o bin/$@
