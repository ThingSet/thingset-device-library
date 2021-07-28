Unit testing for the ThingSet device Library
############################################

The tests are implemented using the ztest environment integrated in Zephyr.
The tests are run on the "native-std" board using the host environment for build.

Run the tests using west to build:

    cd <root of thingset-device-library>
    west build zephyr/tests
    cd build
    ./zephyr/zephyr.elf

Run the tests using CMake to build:

    cd <root of thingset-device-library>
    mkdir build
    cd build
    cmake -GNinja ../zephyr/tests
    ninja
    ./zephyr/zephyr.elf

Test coverage can be obtained (after build and run) by:

    cd <root of thingset-device-library>/build
    lcov --capture --directory ./ --output-file lcov.info -q --rc lcov_branch_coverage=1
    genhtml lcov.info --output-directory lcov_html -q --ignore-errors source --branch-coverage --highlight --legend
    firefox lcov_html/index.html &
