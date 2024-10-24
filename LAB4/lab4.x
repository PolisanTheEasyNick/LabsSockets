struct package {
    uint32_t id;
    uint32_t size;
    opaque data[1024];
};

program FILE_TRANSFER_PROG {
    version FILE_TRANSFER_VERS {
        uint32_t request_num_packages(void) = 1;
        package request_package(uint32_t package_id) = 2;
    } = 1;
} = 0x31234567;
