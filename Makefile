CXX = g++
CXXFLAGS = -std=gnu++17 -Wall -Wextra -I/home/rank/rapidjson/include
LDFLAGS = -lcurl

app: main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f app main.o