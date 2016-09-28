griffin: griffin.cpp
	g++ -std=c++11 -o griffin griffin.cpp -lfuse

/tmp/.griffin:
	mkdir /tmp/.griffin

/tmp/.griffin/real_proc: /tmp/.griffin
	mkdir /tmp/.griffin/real_proc
	mount none -t proc /tmp/.griffin/real_proc

/tmp/.griffin/fake_proc: /tmp/.griffin/real_proc griffin
	mkdir /tmp/.griffin/fake_proc
	./griffin -o allow_other /tmp/.griffin/fake_proc

mount: /tmp/.griffin/fake_proc
	mount -o bind /tmp/.griffin/fake_proc /proc

umount:
	umount /proc
	umount /tmp/.griffin/fake_proc
	umount /tmp/.griffin/real_proc
	rm -rf /tmp/.griffin
