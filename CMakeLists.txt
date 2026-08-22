cmake_minimum_required(VERSION 3.21)
project(sala_do_neston C)

psn00bsdk_add_executable(sala_do_neston GPREL main.c)
target_link_libraries(sala_do_neston PUBLIC psxgpu psxgte psxpad psxetc psxapi)

psn00bsdk_add_cd_image(
    iso
    sala_do_neston
    iso.xml
    DEPENDS sala_do_neston
)
