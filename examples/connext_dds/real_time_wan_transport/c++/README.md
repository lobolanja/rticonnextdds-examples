# Example code: Real Time Wan Transport

## Prerequisites before building example

1)  Before building the example, make sure the environment variable `NDDSHOME` is
    set to the directory where your version of *RTI Connext* is installed.

2)  Set the load library path to include the Connext DDS libraries and OpenSSL

    On MacOSX you can do that with the command:

    ```sh
    export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${NDDSHOME}/lib/x64Darwin17clang9.0:${NDDSHOME}/third_party/openssl-1.1.1i/x64Darwin17clang9.0/release/lib
    ```

    On Linux you do it with the command:

    ```sh
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${NDDSHOME}/lib/x64Linux4gcc7.3.0:${NDDSHOME}/third_party/openssl-1.1.1i/x64Linux4gcc7.3.0/release/lib
    ```

## Building the Example :wrench:

To build this example, first run CMake to generate the corresponding build
files. We recommend you use a separate directory to store all the generated
files (e.g., ./build).

```sh
mkdir build
cd build
cmake ..
```

Once you have run CMake, you will find a number of new files in your build
directory (the list of generated files will depend on the specific CMake
Generator). To build the example, run CMake as follows:

```sh
cmake --build .
```

**Note**: if you are using a multi-configuration generator, such as Visual
Studio solutions, you can specify the configuration mode to build as follows:

```sh
cmake --build . --config Release|Debug
```

Alternatively, you can use directly the generated infrastructure (e.g.,
Makefiles or Visual Studio Solutions) to build the example. If you generated
Makefiles in the configuration process, run make to build the example. Likewise,
if you generated a Visual Studio solution, open the solution and follow the
regular build process.

## Running the Example

To run a specific scenario use the command line option `--scenario <value>`
in the publisher and subscriber applications.

You can get a full list of command line parameters by running with the
command line option `-h`.

If communication is established you should see the following messages in the
`real_time_wan_transport_subscriber` console output.

```plaintext
Received data

   msg: "Hello World 1"
Received data

   msg: "Hello World 2"
```

## Customizing the Build

### Configuring Build Type and Generator

By default, CMake will generate build files using the most common generator for
your host platform (e.g., Makefiles on Unix-like systems and Visual Studio
solution on Windows), \. You can use the following CMake variables to modify the
default behavior:

-   `-DCMAKE_BUILD_TYPE` -- specifies the build mode. Valid values are Release
    and Debug. See the [CMake documentation for more details.
    (Optional)](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)

-   `-DBUILD_SHARED_LIBS` -- specifies the link mode. Valid values are ON for
    dynamic linking and OFF for static linking. See [CMake documentation for
    more details.
    (Optional)](https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html)
```

### Configuring Connext DDS Installation Path and Architecture

The CMake build infrastructure will try to guess the location of your Connext
DDS installation and the Connext DDS architecture based on the default settings
for your host platform.If you installed Connext DDS in a custom location, you
can use the CONNEXTDDS_DIR variable to indicate the path to your RTI Connext DDS
installation folder. For example:

```sh
cmake -DCONNEXTDDS_DIR=/home/rti/rti_connext_dds-x.y.z ..
```

Also, If you installed libraries for multiple target architecture on your system
(i.e., you installed more than one target rtipkg), you can use the
CONNEXTDDS_ARCH variable to indicate the architecture of the specific libraries
you want to link against. For example:

```sh
cmake -DCONNEXTDDS_ARCH=x64Linux3gcc5.4.0 ..
```

### CMake Build Infrastructure

The CMakeListst.txt script that builds this example uses a generic CMake
function called connextdds_add_example that defines all the necessary constructs
to:

1.  Run RTI Code Generator to generate the serialization/deserialization code
    for the types defined in the IDL file associated with the example.

2.  Build the corresponding Publisher and Subscriber applications.

3.  Copy the USER_QOS_PROFILES.xml file into the directory where the publisher
    and subscriber executables are generated.

You will find the definition of connextdds_add_example, along with detailed
documentation, in
[resources/cmake/ConnextDdsAddExample.cmake](../../../../resources/cmake/ConnextDdsAddExample.cmake).

For a more comprehensive example on how to build an RTI Connext DDS application
using CMake, please refer to the
[hello_world](../../../connext_dds/build_systems/cmake/) example, which includes
a comprehensive CMakeLists.txt script with all the steps and instructions
described in detail.