//
// Created by an on 3/3/26.
//

#include "Mem_manager.h"

#include <iostream>

udp_packet_t* Mem_manager::setup_mem_package_pool(Mem_manager& mm, size_t pool_size) {
       std::cout<<"dang tao vung nho\n";
        void *ptr = mmap(NULL, pool_size,
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_POPULATE | MAP_SHARED | MAP_HUGETLB,
                         -1, 0);

        if (ptr == MAP_FAILED) {
            if (mm.retry < 2){
                mm.retry++;
                perror("loi 1 ahihi");
                if (request_hugepages((pool_size + (2048 * 1024) - 1) / (2048 * 1024))) {
                    perror("Dang xin cap phat page \n");
                    return Mem_manager::setup_mem_package_pool(mm, pool_size);
                }
            }
            perror("Cannot hugepage,use default page");
            ptr = mmap(NULL, pool_size,
                       PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_POPULATE | MAP_SHARED,
                       -1, 0);

            if (ptr == MAP_FAILED) {
                perror("Cannot mmap");
                return nullptr;
            }
        }
        //madvise(ptr, pool_size, MADV_WILLNEED | MADV_SEQUENTIAL);
        //memset(ptr, 0, pool_size);

        return static_cast<udp_packet_t *>(ptr);
    }
bool Mem_manager::request_hugepages(int num_pages) {
    std::ofstream hp_file("/proc/sys/vm/nr_hugepages");
    if (!hp_file.is_open()) {
        perror("cannot mo file nay");
        return false;
    }
    hp_file << num_pages;
    hp_file.close();
    std::ifstream check_file("/proc/sys/vm/nr_hugepages");
    int actual_pages;
    check_file >> actual_pages;

    if (actual_pages < num_pages) {
        std::cout << "max " << actual_pages << " ?.\n";
    }
    return true;
}
