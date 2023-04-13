Unit Tests
==========

The library contains an extensive set of unit tests to validate that the specification is
implemented correctly.

The tests can either be run using PlatformIO or with Zephyr for the native POSIX board.

PlatformIO
----------

The tests are implemented using the UNITY environment integrated in PlatformIO. The tests can be
run on the device and in the native environment of the computer. For native (and more quick) tests
run:

.. code-block:: bash

    pio test -e native-std

The test in native environment is also set as the default unit-test, so it is run if you push the
test button in PlatformIO.

To run the unit tests on the device, execute the following command:

.. code-block:: bash

    pio test -e device-std -e device-newlib-nano

Zephyr RTOS
-----------

In order to run the tests with Zephyr, a wrapper for the UNITY test functions was implemented.

Build and run the tests with the following command from the root directory of the library:

.. code-block:: bash

    west build zephyr/tests -t run
