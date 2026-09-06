vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO alandtse/CommonLibVR
    REF 7a60f4de794095d7b0f8928d1b930a52e9a7da83
    SHA512 03f4cfde37f585196603ac54ccf17ffe22d64e420427a1a2de47058fbed7429228bb11c55395c52e9855d598ded3df5fb884cab5536429562f215f1171770289
    HEAD_REF NG
)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH2
    REPO ValveSoftware/openvr
    REF ebdea152f8aac77e9a6db29682b81d762159df7e
    SHA512 4fb668d933ac5b73eb4e97eb29816176e500a4eaebe2480cd0411c95edfb713d58312036f15db50884a2ef5f4ca44859e108dec2b982af9163cefcfc02531f63
    HEAD_REF master
)

file(GLOB OPENVR_FILES "${SOURCE_PATH2}/*")

file(COPY ${OPENVR_FILES} DESTINATION "${SOURCE_PATH}/extern/openvr")

vcpkg_configure_cmake(
    SOURCE_PATH "${SOURCE_PATH}"
    PREFER_NINJA
    OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on
)

vcpkg_install_cmake()
vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake)
vcpkg_copy_pdbs()

file(GLOB CMAKE_CONFIGS "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE/*.cmake")
file(INSTALL ${CMAKE_CONFIGS} DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(INSTALL "${SOURCE_PATH}/cmake/CommonLibSSE.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE")
