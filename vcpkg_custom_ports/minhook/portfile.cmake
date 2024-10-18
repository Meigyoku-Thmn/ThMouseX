if (VCPKG_TARGET_ARCHITECTURE MATCHES "x86")
    set(BUILD_ARCH "Win32")
    set(OUTPUT_DIR "Win32")
elseif (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
    set(BUILD_ARCH "x64")
    set(OUTPUT_DIR "Win64")
else()
    message(FATAL_ERROR "Unsupported architecture: ${VCPKG_TARGET_ARCHITECTURE}")
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Meigyoku-Thmn/minhook
    REF 83a857be06e1926d641fbad3f72efa2f6f6c5c4b
    SHA512 bd1b7277bd24fdc5a4c6fcdc639f53271650786f60dfb341196633ba31f000648ea4972a9a98c6981eef6f9beccb081b7b2f9a0d2ffd2bd90c3e3947bf838d10
    HEAD_REF feature/hook-thunk
)
#[[
set(SOURCE_PATH "D:/Draftbook/minhook")
]]

vcpkg_replace_string(
    "${SOURCE_PATH}/cmake/minhook-config.cmake.in" "\${PACKAGE_PREFIX_DIR}/lib/minhook" "\${CMAKE_CURRENT_LIST_DIR}"
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/minhook)
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)