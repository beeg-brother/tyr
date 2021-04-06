CC=g++
CFLAGS=-c -std=c++17

OBJDIR=src/obj
SRCDIR=src
LIBS=-lncurses -lpanel -lzmq -pthread -std=c++17

all: tyr

clean:
	rm -f tyr
	rm -f $(OBJDIR)/*

tyr: $(SRCDIR)/tyr.cpp $(OBJDIR)/dialog.o $(OBJDIR)/editor.o $(OBJDIR)/filebrowser.o $(OBJDIR)/window.o
	$(CC) -o tyr $(SRCDIR)/tyr.cpp $(LIBS) $(OBJDIR)/dialog.o $(OBJDIR)/editor.o $(OBJDIR)/filebrowser.o $(OBJDIR)/window.o

test: test.cpp
	$(CC) test.cpp $(LIBS) --std=c++17

$(OBJDIR)/dialog.o: $(SRCDIR)/dialog.cpp $(SRCDIR)/constants.h $(SRCDIR)/dialog.h $(SRCDIR)/window.h
	$(CC) $(SRCDIR)/dialog.cpp -o $(OBJDIR)/dialog.o $(CFLAGS)

$(OBJDIR)/editor.o: $(SRCDIR)/editor.cpp $(SRCDIR)/constants.h $(SRCDIR)/cursor.h $(SRCDIR)/editor.h $(SRCDIR)/window.h
	$(CC) $(SRCDIR)/editor.cpp -o $(OBJDIR)/editor.o $(CFLAGS)

$(OBJDIR)/filebrowser.o: $(SRCDIR)/filebrowser.cpp $(SRCDIR)/constants.h $(SRCDIR)/filebrowser.h $(SRCDIR)/window.h
	$(CC) $(SRCDIR)/filebrowser.cpp -o $(OBJDIR)/filebrowser.o $(CFLAGS)

$(OBJDIR)/window.o: $(SRCDIR)/window.cpp $(SRCDIR)/constants.h $(SRCDIR)/window.h
	$(CC) $(SRCDIR)/window.cpp -o $(OBJDIR)/window.o $(CFLAGS)

