all:
	mkdir build
	gcc -o build/cfdp_sender main.c cfdp.c
	gcc -o build/cfdp_receiver receiver.c

clean:
	rm -rf build/

run:
	@$(SHELL) -c build/cfdp_receiver
	@$(SHELL) -c build/cfdp_sender