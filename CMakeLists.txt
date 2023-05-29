# CMakeList.txt: проект CMake для Erxpected; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Erxpected")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Erxpected "Erxpected.cpp" "Erxpected.h" "Expected.h" "TypeId.h" "Error.h" "Try.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Erxpected PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
