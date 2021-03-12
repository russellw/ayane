version = 3

ayane: *.cc *.h
	g++ -O3 -funsigned-char -oayane -s -std=c++11 *.cc -lgmp

debug:
	g++ -DDEBUG -Og -funsigned-char -g -oayane -std=c++11 *.cc -lgmp

prof:
	g++ -funsigned-char -pg -oayane -std=c++11 *.cc -lgmp

clean:
	rm ayane
	rm ayane-$(version).tgz

dist: ayane
	mkdir ayane-$(version)
	cp *.cc ayane-$(version)
	cp *.h ayane-$(version)
	cp *.py ayane-$(version)
	cp LICENSE.txt ayane-$(version)
	cp Makefile ayane-$(version)
	tar cfa ayane-$(version).tgz ayane-$(version)
	rm -r ayane-$(version)

install:
	mv ayane /usr/local/bin

uninstall:
	rm /usr/local/bin/ayane
