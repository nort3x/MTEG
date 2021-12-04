OUTPUT = client
CFLAGS = -g -Wall -Wvla --std=c11 `sdl2-config --cflags`
LFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

%.o: %.c %.h
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

all: $(OUTPUT)

client: client.o
	gcc $(CFLAGS) -o $@ $^ $(LFLAGS)

clean:
	rm -f $(OUTPUT) *.o
