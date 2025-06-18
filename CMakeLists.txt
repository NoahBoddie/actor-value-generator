cmake_minimum_required(VERSION 3.21)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

########################################################################################################################
## Define project
########################################################################################################################
project(
        ActorValueExtension
        VERSION 1.9.5.12
        DESCRIPTION "An SKSE modding tool that enables users to make new actor values as well as extending their existing functionally."
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(GNUInstallDirs)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY)

#trying this out for right now.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/Plugin.h")
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.h.in"
	"${VERSION_HEADER}"
	@ONLY
)

set(headers
#I would really like to make this as a sort of dependency instead. But for now.
        include/Arthmetic.h
        include/Serialization.h
        include/RoguesGallery.h

        include/API_Arithmetic.h
        include/API_ActorValueGenerator.h
)

set(sources
        src/Arthmetic/Constructor.h
        src/Arthmetic/ArthmeticObject.h
        src/Arthmetic/IDelegate.h
        src/Arthmetic/DelegateType.h
        src/Arthmetic/IRoutine.h
        src/Arthmetic/Coroutine.h
        src/Arthmetic/Subroutine.h
        src/Arthmetic/DelegateArgument.h
        src/Arthmetic/DelegateArgument.h
        #src/Arthmetic/DualOperator.h
        #src/Arthmetic/OpenOperator.h
        #src/Arthmetic/FunctionDelegate.h
        src/Arthmetic/IReadyArthmetic.h
        src/Arthmetic/IOperator.h
        src/Arthmetic/OperatorType.h
        src/Arthmetic/ArgumentType.h
        src/Arthmetic/RoutineArgument.h
        src/Arthmetic/RoutineDelegate.h
        src/Arthmetic/RoutineItem.h
        src/Arthmetic/RoutineItemType.h
        src/Arthmetic/TargetType.h
        src/Arthmetic/Record.h
        src/Arthmetic/IDirective.h
        src/Arthmetic/IFormula.h
        src/Arthmetic/FunctionInterface.h
        src/Arthmetic/FormulaDelegate.h
        src/Arthmetic/ParameterSetting.h

        src/Arthmetic/ArthmeticConstructor.h
        src/Arthmetic/ArthmeticParser.h
        src/Arthmetic/ArthmeticUtility.h
        src/Arthmetic/RoutineItem.cpp
        src/Arthmetic/IReadyArthmetic.cpp
        src/Arthmetic/DelegateArgument.cpp


        src/Serialization/ISerializer.h
        src/Serialization/SerialArgument.h
        src/Serialization/SerialComponent.h
        src/Serialization/SerialConstructor.h
        src/Serialization/SerialIterator.h
        src/Serialization/SerializableList.h
        src/Serialization/SerializableMap.h
        src/Serialization/SerializableObject.h
        src/Serialization/SerializablePair.h
        src/Serialization/SerializationTypePlayground.h
        src/Serialization/SerializingWrapper.h

      
        src/RoguesGallery/Utility.h
        src/RoguesGallery/TemporaryFlag.h


        src/Types.h
        src/ExtraValue.h
        src/BGSEntryPointFunctionDataTwoValue.h
        src/main.cpp
        src/ActorValueExtendedList.h
        src/ExtraValueInfo.h
        src/ExtraValueStorage.h
        src/ExtraValueInfo.cpp
        src/ExtraValueStorage.cpp
        src/EventSingleton.h
        src/FormExtraValueHandler.h
        src/Hooks.h
        src/Addresses.h
        src/Utility.h
        src/ValueAliasHandler.h

        src/API_Arithmetic.cpp
        src/API_ActorValueGenerator.cpp

        ${CMAKE_CURRENT_BINARY_DIR}/version.rc)

#set(tests
#        test/HitCounterManager.cpp)

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${headers}
        ${sources}
        ${tests})

#########################################################################################################################
### Build options
#########################################################################################################################
message("Options:")
option(BUILD_TESTS "Build unit tests." OFF)
message("\tTests: ${BUILD_TESTS}")

########################################################################################################################
## Configure target DLL
########################################################################################################################
find_package(CommonLibSSE CONFIG REQUIRED)


add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        src/PCH.h)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/Sample"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

########################################################################################################################
## Configure unit tests
########################################################################################################################
if(BUILD_TESTS)
    find_package(Catch2 CONFIG REQUIRED)

    include(CTest)
    include(Catch)

    add_executable(
            ${PROJECT_NAME}Tests
            ${headers}
            ${tests})

    target_link_libraries(
            ${PROJECT_NAME}Tests
            PRIVATE
            ${PROJECT_NAME}
            Catch2::Catch2WithMain)

    target_precompile_headers(${PROJECT_NAME}Tests
            PRIVATE
            src/PCH.h)

    catch_discover_tests(${PROJECT_NAME}Tests)
    add_test(NAME ${PROJECT_NAME}Tests COMMAND ${PROJECT_NAME}Tests)
endif()

########################################################################################################################
## Automatic plugin deployment
########################################################################################################################
# Automatic deployment to FOMOD directory.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_NAME "Debug")
else()
    set(BUILD_NAME "Release")
endif()
install(DIRECTORY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Papyrus${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
endif()
file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")
file(GLOB_RECURSE OUTPUT_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pex")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_SCRIPTS}")

# Automatic deployment to Mod Organizer 2 mod directory.
foreach(DEPLOY_TARGET $ENV{CommonLibSSESamplePluginTargets})
    message("Adding deployment target ${DEPLOY_TARGET}.")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")
    endif()
endforeach()
