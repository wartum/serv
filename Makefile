CFLAGS+=-g
OBJECTS=serv.o utils/http.o
serv: $(OBJECTS)
	gcc $^ -o $@

clean:
	rm $(OBJECTS)
