version = 0

ayane: *.cc *.h
	g++ -O3 -fno-rtti -funsigned-char -oayane -std=c++11 *.cc -lgmp

clean:
	rm ayane
	rm ayane-$(version).tgz

debug:
	g++ -DDEBUG -fno-rtti -funsigned-char -g -oayane -std=c++11 *.cc -lgmp

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
