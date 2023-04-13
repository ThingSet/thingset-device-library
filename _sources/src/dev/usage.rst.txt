Library Usage
=============

Example Programs
----------------

Example programs can be found in the ``examples`` directory.

The basic example shows a very simple setup to get started with ThingSet.

It provides a shell to access the data from ``test/test_data.c`` via ThingSet protocol.

Build instructions for the examples (requires CMake and a GCC):

.. code-block:: bash

    cd examples/basic       # or examples/interactive
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ./sample


Basic C++ code snippet
----------------------

Assuming the data is stored in a static array ``data_objects`` as in the example, a ThingSet object
is created by:

.. code-block:: cpp

    ThingSet ts(data_objects, sizeof(data_objects)/sizeof(ThingSetDataObject));

Afterwards, it can be used with any communication interface using the ``process`` function:

.. code-block:: cpp

    uint8_t req_buf[500];        // buffer to store incoming data
    uint8_t resp_buf[500];       // buffer to store ThingSet response

    /*
     * Listen to a communication interface (e.g. UART) and store
     * incoming data in the request buffer.
     *
     * After receiving a new-line character, process the request.
     */

    int req_len = strlen((char *)req_buf);  // only works for text mode

    ts.process(req_buf, req_len, resp_buf, sizeof(resp_buf));

    /*
     * The response including the requested data is now in the response buffer
     * and can be sent back to the communication interface.
     */
