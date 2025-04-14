double:
	$(eval CFLAGS = -DUSE_DOUBLE)
	g++ $(CFLAGS) src/main.cpp -o sin_array
	./sin_array

float:
	$(eval CFLAGS = -DUSE_FLOAT)
	g++ $(CFLAGS) src/main.cpp -o sin_array
	./sin_array

# Очистка
clean:
	rm -f sin_array
