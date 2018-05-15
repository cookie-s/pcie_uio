#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "generic.h"

using phys_addr = size_t;
using virt_addr = size_t;

class Udmabuf {
	public:
		Udmabuf() = delete;
		Udmabuf(int idx) {
			if(0 <= idx && idx < 10) {
				char buf1024[1024];

				{
					int fd;
					char sizefn[] = "/sys/class/udmabuf/udmabuf\0/size";
					sizefn[strlen(sizefn)] = '0' + idx;
					if((fd = open(sizefn, O_RDONLY)) == -1)  {
						perror("open size");
						panic("");
					}
					read(fd, buf1024, 1024);
					sscanf(buf1024, "%lu", &_size);
					close(fd);
				}

				{
					int fd;
					char buffn[] = "/dev/udmabuf\0";
					buffn[strlen(buffn)] = '0' + idx;
					if((fd = open(buffn, O_RDWR)) == -1) {
						perror("open udmabuf");
						panic("");
					}
					if((_virt = mmap(NULL, _size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
						perror("page alloc");
						panic("");
					}
					close(fd);
				}

				{
					int fd;
					char physaddrfn[] = "/sys/class/udmabuf/udmabuf\0/phys_addr";
					physaddrfn[strlen(physaddrfn)] = '0' + idx;
					if((fd = open(physaddrfn, O_RDONLY)) == -1)  {
						perror("open phyaddr");
						panic("");
					}
					read(fd, buf1024, 1024);
					sscanf(buf1024, "%lx", &_phys);
					close(fd);
				}
			} else {
				fprintf(stderr, "invalid udmabuf index\n");
				panic("");
				_virt = 0;
				_phys = 0;
				_size = 0;
			}
		}
		~Udmabuf() {
			if (_size > 0) {
				if (munmap(_virt, _size) != 0) {
					perror("munmap:");
					panic("");
				}
			}
		}
		template <class T> T *GetVirtPtr() {
			return reinterpret_cast<T *>(_virt);
		}
		phys_addr GetPhysPtr() const { return _phys; }
		size_t GetSize() { return _size; }

	private:
		void *_virt;
		phys_addr _phys;
		size_t _size;
};
