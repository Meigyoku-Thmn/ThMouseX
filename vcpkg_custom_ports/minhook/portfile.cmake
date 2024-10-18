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
    REF 3482886f355cf9837fb8f0147915592e887fcff9
    SHA512 f63d327211c606615fb611a320d25e48a4858d387d43754cb2da9850370fbceaeb6860d9e816d8ace80971543a3053a337c14d7cda801b6a51f2101b30c4e58f
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