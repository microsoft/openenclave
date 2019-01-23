Creating your own enclave and using it from a Windows host app
=============

This documentation will walk you through how the calls on the various platforms. This document is a work in progress.

## Generating API calls between Trusted Apps ("Enclaves") and Untrusted ("Host") Apps

To use this SDK, you must define your own
[EDL](https://software.intel.com/en-us/sgx-sdk-dev-reference-enclave-definition-language-file-syntax)
file that defines any APIs you want to
use, and use **oeedger8r** to generate code from it.
The same generated code will work equally with both SGX
and OP-TEE, as long as the Untrusted App and the Trusted App both use the right include paths and libs,
and the following additional code constraint is met:

OP-TEE only allows one thread per TA to be in an ECALL (i.e., a call into
a TA from a host app).  Even if it has an OCALL (i.e., an out-call
back into the host app) in progress, the ECALL must complete before
another ECALL can enter the TA.  SGX, on the other hand, would allow a
second ECALL to enter.  So if you want them to function identically, host apps
can pass the OE\_ENCLAVE\_FLAG\_SERIALIZE\_ECALLS
flag when creating an enclave to automatically get the OP-TEE like behavior
for both SGX and TrustZone.

## Include paths, preprocessor defines, and libraries

### SGX Enclave DLL

The SGX Enclave DLL should link with **oeenclave.lib** and have the following
additional include paths:

* $(OESdkDir)new\_platforms\include\sgx\enclave
* $(OESdkDir)new\_platforms\include

To use stdio APIs, the SGX Enclave DLL should link with **oestdio\_enc.lib**.
See the helloworld sample for an example.

To use socket APIs, the SGX Enclave DLL should link with **oesocket\_enc.lib**.
See the sockets sample for an example.

### SGX Rich Application

The EXE should link with **oehost.lib** and have the following additional
include path:

* $(OESdkDir)new\_platforms\include

To allow the SGX Enclave DLL to use stdio APIs, the EXE should link with **oestdio\_host.lib**.
See the helloworld sample for an example.

To allow the SGX Enclave DLL to use socket APIs, the EXE should link with **oesocket\_host.lib**.
See the sockets sample for an example.

### OP-TEE TA

The OP-TEE TA should link with **liboeenclave** and have the following
additional include paths, in this order (the order is important because
files in a deeper directory override files at higher levels with the
same filename):

* $(OESdkDir)new\_platforms/include/optee/enclave
* $(OESdkDir)new\_platforms/include/optee
* $(OESdkDir)new\_platforms/include

To use stdio APIs, the OP-TEE TA should link with **liboestdio\_enc**.
See the helloworld sample for an example.

To use socket APIs, the OP-TEE TA should link with **liboesocket\_enc**.
See the sockets sample for an example.

### OP-TEE Rich Application

The EXE should link with **oehost.lib** and have the following
additional include paths, in any order:

* $(OESdkDir)new\_platforms\include
* $(OESdkDir)new\_platforms\include\optee
* $(OESdkDir)new\_platforms\include\optee\host

To allow the OP-TEE TA to use stdio APIs, the EXE should link with **oestdio\_host.lib**.
See the helloworld sample for an example.

To allow the OP-TEE TA to use socket APIs, the EXE should link with **oesocket\_host.lib**.
See the sockets sample for an example.

## Open Enclave APIs

This SDK implements the Open Enclave APIs covered in
[API docs](https://ms-iot.github.io/openenclave/api/files.html).

This SDK also provides support for a number of APIs that are not
available in SGX and/or OP-TEE.  For APIs that would normally be in some
standard C header (e.g., "stdio.h"), the convention is that instead of
including *token*.h, one would include (instead or in addition to the
one provided by SGX or OP-TEE if any), tcps\_*token*\_t.h for defines
common to both SGX and OP-TEE, or tcps\_*token*\_optee\_t.h for defines
unique to OP-TEE, since the Intel SGX SDK already provides more than OP-TEE
provides and such files provide the equivalent for OP-TEE.  For example,
the following such headers exist:

* tcps\_stdlib\_t.h
* tcps\_string\_t.h
* tcps\_time\_t.h

Additional OP-TEE only headers that provide support already present in
SGX include:

* tcps\_ctype\_optee\_t.h
* tcps\_string\_optee\_t.h

If none of the above are needed, trusted code might need to #include
<openenclave/enclave.h> (as well as the \*\_t.h file generated from your application's
EDL file) to get basic definitions, but it should be pulled in automatically
if any of the above are included.

Untrusted code might need to #include <openenclave/host.h>  (as well as the \*\_u.h
file generated from your application's EDL file) to get basic definitions.

## Using this SDK from Windows

To use this SDK for a new application to run on both SGX and OP-TEE,
do the following.

**To build for SGX:**

1. In Visual Studio, add a new Visual C++ "Intel(R) SGX Enclave Project".
Make sure the Project Type is "Enclave" and the EDL File checkbox is checked.
2. Edit the _YourProjectName_.edl file, and add at least one public ECALL
in the trusted{} section.  E.g., "public void ecall\_DoWork();"
Also, to use stdio APIs such as printf, above (outside) the trusted{} section, add the following line:

```
from "openenclave/stdio.edl" import *;
```
3. Update the command line for your EDL to use oeedger8r.
To do this, right click on the EDL file in the Solution Explorer window,
select "Properties"->"Configuration Properties"->"Custom Build Tool"->"General"
and edit the "Command Line" value for All Configurations and All Platforms.
Change it to "$(NewPlatformsDir)oeedger8r.exe" --trusted "%(FullPath)" --search-path "$(NewPlatformsDir)include;$(SGXSDKInstallPath)include" 
where $(NewPlatformsDir) is the path to the new\_platforms subdirectory of this SDK.
The projects referenced by the OpenEnclave.sln file have the following line added
which pulls in definitions for NewPlatformsDir and other macros:
```
<Import Project="$(SolutionDir)\vssettings.props" />
```
4. In Visual Studio, add a new or existing Visual C++ project that will
build a normal application that will make ECALLs into your enclave.
5. Right click on your application project, select
"Intel(R) SGX Configuration"->"Import Enclave", and import the EDL
6. Right click on your application project, select "Properties"->"Debugging"
and change the "Debugger to launch" to "Intel(R) SGX Debugger".  You may
also want to change the Working Directory to the Output Directory of the
enclave project, where your enclave DLL will be placed.
7. Find the EDL file in your application project in the Solution Explorer window
and repeat step 3 here to update the command line to use oeedger8r.
8. In your enclave project, add implementations of the ECALL(s) you added.
You will need to #include <openenclave/enclave.h> and <_YourEDLFileName_\_t.h> for your ECALLs.
9. In your application project properties, under "Linker"->"Input", add
oehost.lib;ws2\_32.lib;shell32.lib to the Additional Dependencies
(All Configurations, All Platforms).  Make sure you configure the
additional library directory as appropriate under
"Linker"->"General"->"Additional Library Directories".
10. If you want access to the full set of Open Enclave APIs from within your enclave,
in your enclave project properties, add oeenclave.lib;sgx\_tstdc.lib to the
Additional Dependencies, and the path to oeenclave.lib to the Additional
Library Directories.
11. Add code in your app to call oe\_create\__YourEDLFileName_\_enclave(),
any ECALLs you added, and
oe\_terminate\_enclave().  You will need to #include <openenclave/host.h> 
and <_YourEDLFileName_\_u.h> for your ECALLs.  Make sure you configure the
additional include directories as appropriate in your application project
Properties->"C/C++"->"General"->"Additional Include Directories".  Usually
this means you need to insert "$(NewPlatformsDir)include;$(NewPlatformsDir)include\sgx\enclave;"
at the beginning.  See the sample apps for an example.
12. In your enclave project, update the Additional Include Directories to
include $(OESdkDir)3rdparty\RIoT\CyReP\cyrep

**Then to build for OP-TEE:**

1. Right click the solution, select "Configuration Manager", create an ARM
solution platform (if one doesn't already exist), and then create ARM
configurations of your application project (and oehost if you include that
directly).  Copy the configuration from the existing Win32 one.  Don't do
this for your enclave project, as that needs to be built from a bash shell
rather than in Visual Studio.
2. Manually edit your application .vcxproj file to add the ability to
compile for ARM, since Visual Studio cannot do it from the UI.  To do so, add the
line "<WindowsSDKDesktopARMSupport\>true</WindowsSDKDesktopARMSupport\>"
to each ARM configuration property group.  (See the sample apps'
vcxproj file for examples.)
3. Copy the files from the samples/helloworld/HelloWorldEnc/optee directory into your
enclave project, preferably into an "optee" subdirectory
4. Create a new GUID for your TA and fill it in in your linux\_gcc.mak,
user\_ta\_header\_defines.h, main.c, and uuids.reg files.  You can use the
guidgen.exe utility that comes with Visual Studio, or uuidgen (available
in a bash window), or
[https://www.uuidgenerator.net/](https://www.uuidgenerator.net/)
5. For any new source files you add to your enclave project in
Visual Studio, also add them to the sub.mk file in your optee subdirectory
6. In your application project properties, under "Linker"->"Input", add
rpcrt4.lib to the Additional Dependencies (All Configurations, ARM platform)
which is required for string-to-UUID conversion, and remove any sgx libs.
7. In your application project properties, update the Additional Include
Directories to insert the $(NewPlatformsDir)include\optee\host and
$(NewPlatformsDir)include\optee paths before the $(NewPlatformsDir)include path that you
added earlier.
8. Edit the sub.mk file in your optee subdirectory to change the "SampleTA"
in filenames to the name used with your .edl file.
9. On the destination machine, apply the uuids.reg file
("reg.exe import uuids.reg") and reboot.

## Debugging

**For SGX:** You can use Visual Studio for debugging, including the SGX
simulator, that comes with the Intel SGX SDK.  Simply use the Debug
configuration in Visual Studio if you have SGX-capable hardware, or
the DebugSimulation configuration in Visual Studio for software emulation.

**For TrustZone:** You can use a basic software emulation environment with OP-TEE
by creating and using a DebugOpteeSimulation configuration in Visual Studio,
as follows...

1. Create a new configuration (say, for x86 and called DebugOpteeSimulation)
based on the Debug configuration. A new configuration can be created
inside Visual Studio by right clicking on the solution, and accessing
the "Configuration Manager" screen.
2. Go to your application project properties.  In the
"Configuration Properties"->"C/C++"->"Preprocessor" properties for All Platforms
for the DebugOpteeSimulation configuration, add
**OE\_SIMULATE\_OPTEE**.  Then do the same for your enclave project.
3. Add oehost\_opteesim.lib to the Additional Dependencies of your app and
remove any sgx libraries, for All Platforms for the DebugOpteeSimulation
configuration. Your libs might look like this:
"oehost.lib;ws2\_32.lib;rpcrt4.lib;shell32.lib;oehost\_opteesim.lib"
4. Your app Additional Include Directories for DebugOpteeSimulation
should include at least:
* $(NewPlatformsDir)include\optee\host
* $(NewPlatformsDir)include\optee
* $(NewPlatformsDir)include
* $(SGXSDKInstallPath)\include
5. In the "Configuration Properties->"Debugging", change Debugger to launch
back to Local Windows Debugger, and make the working directory be the
directory your enclave is built in (usually "$(OutDir)").
6. The same files that you build for OP-TEE should be built for
DebugOpteeSimulation, so if you have files selectively built (e.g., if you
have C files marked as Exclude From Build for certain configurations),
update your configuration so that the same files get built, not any
SGX-specific ones.
7. In your TA's "Configuration Properties"->"C/C++"->"General" properties of the
DebugOpteeSimulation configuration for All Platforms, the Additional Include
Directories should NOT include $(SGXSDKInstallPath)include\tlibc or
$(SGXSDKInstallPath)include\libc++, and should include at least:
* $(OESdkDir)3rdparty\RIoT\CyReP\cyrep
* $(NewPlatformsDir)include\optee\enclave\Simulator
* $(NewPlatformsDir)include\optee\enclave
* $(NewPlatformsDir)include\optee
* $(NewPlatformsDir)include
* $(SGXSDKInstallPath)include
* $(OESdkDir)3rdparty\optee\_os\lib\libutee\include
* $(OESdkDir)3rdparty\optee\_os\lib\libutils\ext\include
8. In your TA's "Configuration Properties"->"VC++ Directories" properties of the
DebugOpteeSimulation configuration for All Platforms, change the Include Directories
and Library Directories from $(NoInherit) back to \<inherit from parent or
project defaults\>.
9. In your TA's "Configuration Properties"->"Linker"->"Input" in the
DebugOpteeSimulation configuration for All Platforms, clear "Ignore All Default
Libraries", and add oeenclave\_opteesim.lib to the Additional Dependencies of
your TA, and remove any sgx libs.  Your libs might look like this:
"oeenclave.lib;oeenclave\_opteesim.lib;kernel32.lib;vcruntime.lib;ucrtd.lib"
10. In your TA's "Configuration Properties"->"Build Events"->"Post-Build Event",
change the "Use in Build" to No for the DebugOpteeSimulation configuration for all
Platforms.
11. In your TA's "Configuration Properties"->"General" section in the
DebugOpteeSimulation configuration for All Platforms, change the Target Name
to be your TA's UUID (without curly braces).

# Understanding the SDK

To understand how to build a host app and TA, this section examines one of the
samples included with the Open Enclave SDK. These samples are found under
`new_platforms/samples`.

This section examines the `sockets` sample. This sample consists of two host apps
and one TA. The TA contains both server and client functionality used by both
host apps. The server host app invokes the server functionality in the TA and the client
host app invokes the client functionality. Server and client exchange simple text
messages over a TCP/IP socket.

##  Sample Structure

This is the folder structure of the `sockets` sample:

```
new_platforms/samples/sockets/
    | SampleTA.edl
    | enclave
        | SampleTA.c
        | optee
            | linux_gcc.mak
            | sub.mak
            | user_ta_header_defines.h
    | host
        | ClientServerApp
            | main.c
            | Makefile      
        | SampleServerApp
            | main.c
            | Makefile
```

The `SampleTA.edl` file describes the interface via which the host app and TA
communicate with one another. This EDL file contains only functionality that is
specific to the sample. However, it references other EDL files provided by the
Open Enclave SDK. These referenced EDL files specify the interface through which
the trusted components of the SDK, which are linked into your TA, and the
corresponding untrusted components of the SDK, which are linked into your host
app, talk with each other. The `oeedger8r` tool processes this file and
generates code that may be called by either the trusted or the untrusted side to
transparently communicate with the other side.

Under the `enclave` folder there is a single `SampleTA.c` file. This file
contains all the TA's functionality. Note how there is no TEE-specific code:
nothing is specific to either Intel SGX or ARM TrustZone, these details are
taken care of by the SDK.

Next, there is a Visual Studio project file. This file is used by Visual Studio
on Windows to build this TA as an Intel SGX enclave. Similarly, under the
`optee` folder are Makefiles that are consumed by the OP-TEE TA Dev Kit to
produce an OP-TEE-compatible TA. Both the Visual Studio project and the
Makefiles reference `SampleTA.c`.

Lastly, there is a file called `user_ta_header_defines.h`. This file is also
consumed by the OP-TEE TA Dev Kit and it specifies the UUID of the TA as well as
other OP-TEE-specific parameters. You must fill in these values in accordance
with how you plan to use the TA. In this instance, you will need to understand
how OP-TEE loads and manages sessions with TA's in order to configure yours
properly. Each TA must have a unique UUID.

Under the `host` folder there are two more folders: `ClientServerApp` and
`SampleServerApp`. These folders contain the client and server host programs,
respectively.

Under the first folder there is a `main.c` file. This file contains OS- and
TEE-agnostic host code that loads the TA and invokes the client-related
functionality. Note that the code that you write for your host app need not know how
to launch an enclave under Intel SGX or a TA under ARM TrustZone nor how to do
so under both Windows and Linux. Again, the Open Enclave SDK deals with these
details on your behalf. The `SampleServerApp` folder follows the same
principles.

In general then, there is an EDL file that both the trusted and untrusted
components consume that specifies how the pair communicate across the trust
boundary. The EDL file only contains functions that are specific to your
use case. The trusted component need only be written once and targets the Open
Enclave SDK's API. This way it can be seamlessly compiled as an Intel SGX
enclave and as an ARM TrustZone trusted application. Similarly, the host
programs also need only be written once and, assuming you do not make use of
OS-specific functionality, they too can be compiled into Windows and Linux
programs that can seamlessly launch and operate Intel SGX enclaves and ARM
TrustZone trusted applications.

## Dependencies

The trusted component depends on the following Open Enclave-provided libraries:

* oeenclave: Provides core Open Enclave functionality;
* oesocket\_enc: Provides sockets functionality inside the TA:
    * Linking is necessary only if you use sockets;
    * This library marshals socket calls out to the host app.
* oestdio\_enc: Provides standard I/O functionality inside the TA:
    * Linking is necessary only if you use standard I/O;
    * This library marshals standard I/O calls out to the host app.

You can see these libraries being listed in the `sub.mk` file under
`enclave/optee` and in the corresponding Visual Studio project file. The
sub-makefile includes `oe_sub.mk` which specifies `oeenclave`.

**Note**: When using sockets or I/O APIs, or any other trusted-to-untrusted
call, the data that you send into these APIs and out to the host app is not
automatically protected.

The untrusted component depends on the following Open Enclave-provided
libraries:

* oehost: Provides core Open Enclave functionality;
* oesocket\_host: Provides the implementation of the socket calls that the
  trusted component makes:
    * Linking is necessary only if you use sockets inside the trusted component.
* oestdio\_host: Provides the implementation of the standard I/O calls that the
  trusted component makes:
    * Linking is necessary only if you use standard I/O inside the trusted
      component.

Notice how these libraries come in pairs. The libraries provide Open
Enclave-provided APIs inside the trusted component. When you call these APIs,
they in turn invoke code generated by the `oeedger8r` tool as specified by the
SDK's EDL files, included by yours. This code marshals the calls across the
trust boundary which are captured by functions implemented in the corresponding
libraries in the host app. The same is true the other way around.
