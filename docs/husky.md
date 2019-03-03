# Husky

<!-- MarkdownTOC levels="2,3,4,5" autoanchor=false autolink=true -->

- [Installation of Husky](#installation-of-husky)
    - [Install dependencies](#install-dependencies)
        - [ZeroMQ](#zeromq)
            - [libzmq](#libzmq)
            - [cppzmq](#cppzmq)
    - [Boost Libraries](#boost-libraries)
    - [Eigen](#eigen)
    - [GCC](#gcc)
    - [Summary of Installation Notes](#summary-of-installation-notes)
    - [Troubleshooting](#troubleshooting)
        - [`function` in namespace `std` does not name a template type](#function-in-namespace-std-does-not-name-a-template-type)
- [Installation of Husky on Abel Cluster](#installation-of-husky-on-abel-cluster)
    - [Dependencies and Modules](#dependencies-and-modules)
        - [GCC](#gcc-1)
        - [Boost Libraries](#boost-libraries-1)
        - [ZeroMQ](#zeromq-1)
            - [libzmq](#libzmq-1)
            - [cppzmq](#cppzmq-1)
    - [GPerfTools](#gperftools)
    - [Summary of Installation Notes](#summary-of-installation-notes-1)
    - [Troubleshooting](#troubleshooting-1)
        - [C compiler cannot create executables](#c-compiler-cannot-create-executables)

<!-- /MarkdownTOC -->


## Installation of Husky

[Husky] project is available to download from [Husky's GitHub repository].

[Husky]: http://www.husky-project.com/
[Husky's GitHub repository]: https://github.com/husky-team/husky/

- Download the project source.

```bash
wget https://github.com/husky-team/husky/archive/v0.1.1.tar.gz
```

or 

```bash
git clone https://github.com/husky-team/husky.git
```

### Install dependencies

#### ZeroMQ

Both [libzmq] and [cppzmq] must be present.

##### libzmq

```bash
sudo yum update
sudo yum install zeromq-devel
```

##### cppzmq

```bash
cd ~/downloads
wget https://github.com/zeromq/cppzmq/archive/master.zip -O cppzmq-source.zip
unzip cppzmq-source.zip
mv cppzmq-master/ cppzmq

cd cppzmq
mkdir build
cd build
cmake ..
sudo  make -j4 install
```

This will install ZeroMQ in `/usr/local`, where `cmake` for Husky will detect automatically.

- `/usr/local/include`
- `/usr/local/share/cmake/cppzmq`

If no sudo access available, you can install to another folder:

```bash
export DESTDIR="~/opt/" &&  make -j4 install
```


[libzmq]: https://github.com/zeromq/libzmq
[cppzmq]: https://github.com/zeromq/cppzmq

### Boost Libraries

Install [Boost]. The version for your distribution may be older than the latest Boost version.

```bash
sudo yum update
sudo yum install epel-release
sudo yum install boost boost-thread boost-devel
```

Or you can try manually installing Boost. However, Husky project relies on Boost libraries that need to be separately built. See [installation instructions](https://www.boost.org/doc/libs/1_69_0/more/getting_started/unix-variants.html#easy-build-and-install).

```bash
wget https://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz

tar xzvf boost_1_69_0.tar.gz
cd boost_1_69_0
./bootstrap.sh --help
CC=~/opt/usr/local/bin/gcc CXX=~/opt/usr/local/bin/g++ \
./bootstrap.sh --prefix=~/opt --show-libraries
CC=~/opt/usr/local/bin/gcc CXX=~/opt/usr/local/bin/g++ \
./bootstrap.sh --prefix=/opt 
# following will take long time!
./b2 install -j 4

export BOOST_ROOT=~/opt/boost
```

When compiling Husky, and `cmake` cannot find Boost, pass the parameter `-DBOOST_ROOT=~/opt/boost`:

```bash
cd $HUSKY_ROOT/release
cmake -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=~/opt/boost ..
```

[Boost]: https://www.boost.org/doc/libs/1_62_0/more/getting_started/unix-variants.html

### Eigen

Download and extract [Eigen] library.

```bash
cd ~/downloads
wget http://bitbucket.org/eigen/eigen/get/3.3.7.tar.bz2
tar jxvf 3.3.7.tar.bz2
mv eigen-eigen-323c052e1731 eigen
mv eigen/ ~/opt/.
```

When compiling Husky, and `cmake` cannot find Eigen, pass the parameter `-DEIGEN_INCLUDE_DIR=~/opt/eigen`:

```bash
cd $HUSKY_ROOT/release
cmake -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=~/opt/boost_1_61_0 -DEIGEN_INCLUDE_DIR=~/opt/eigen ..
```

[Eigen]: http://eigen.tuxfamily.org/dox/GettingStarted.html

### GCC

Download [GCC].

Use following option to build for a location other than `/usr/local`.

`./configure --prefix=/somewhere/else/than/usr/local`

Since you will be using a custom install of `gcc`, you will also have to update `LD_LIBRARY_PATH` environment variable:

```bash
export LD_LIBRARY_PATH:/home/amin/opt/gcc/lib:/home/amin/opt/gcc/lib64:$LD_LIBRARY_PATH
```

In summary:

```bash
cd ~/downloads
wget ftp://ftp.lip6.fr/pub/gcc/releases/gcc-8.3.0/gcc-8.3.0.tar.gz
tar xzvf gcc-8.3.0.tar.gz
cd gcc-8.3.0/
./contrib/download_prerequisites
./configure --disable-multilib --enable-languages=c,c++
# following will take hours!
nohup make -j 4 >> script-output.log 2>&1 &
tail -F script-output.log
export DESTDIR="~/opt/" && make -j4 install

export LD_LIBRARY_PATH:/home/amin/opt/gcc/lib:/home/amin/opt/gcc/lib64:$LD_LIBRARY_PATH
```

[GCC]: https://gcc.gnu.org/install/
[GCC Installation on CentOS]: https://linuxhostsupport.com/blog/how-to-install-gcc-on-centos-7/

### Summary of Installation Notes

```bash
cd ~/downloads
wget https://github.com/husky-team/husky/archive/master.zip -O husky-source.zip
unzip husky-source.zip
mv -v husky-master ~/husky

export HUSKY_ROOT=~/husky
cd $HUSKY_ROOT
mkdir release
cd release
export CC=~/opt/usr/local/bin/gcc
export CXX=~/opt/usr/local/bin/g++

CC=~/opt/usr/local/bin/gcc \
CXX=~/opt/usr/local/bin/g++ \
cmake -DCMAKE_BUILD_TYPE=Release \
    -D CMAKE_C_COMPILER=~/opt/usr/local/bin/gcc \
    -D CMAKE_CXX_COMPILER=~/opt/usr/local/bin/g++ \
    -DBOOST_ROOT=~/opt/boost \
    -DEIGEN_INCLUDE_DIR=~/opt/eigen \
    .. >> install.log 2>&1 &

make help
CC=~/opt/usr/local/bin/gcc CXX=~/opt/usr/local/bin/g++ make -j4 Master
CC=~/opt/usr/local/bin/gcc CXX=~/opt/usr/local/bin/g++ make husky
```

You might see the error that `glog/logging.h` was not found. Note that during build process, either due to `cmake -DCMAKE_PREFIX_PATH=~/opt/`, `$DEST_DIR`, or in general, Husky may build `glog` instead to `~/opt/home/amin/husky/release/`, so you need to manually copy back to the correct location in `release` folder.

```bash
cd $HUSKY_ROOT/release
cp -Rv ~/opt/home/amin/husky/release/include .
cp -Rv ~/opt/home/amin/husky/release/lib/* ./lib/
```

### Troubleshooting

#### `function` in namespace `std` does not name a template type

When building Husky with `make -j4 Master`, if you get the error like:

```
In file included from /home/amin/husky/base/generation_lock.cpp:15:
/home/amin/husky/base/generation_lock.hpp:61:32: error: ‘function’ in namespace ‘std’ does not name a template type
     void operator()(const std::function<void()>& exec);
```

Make sure you're using the latest version of `gcc`, and it is building against C++ 14 standard.

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
    -DBOOST_ROOT=~/opt/boost \
    -DEIGEN_INCLUDE_DIR=~/opt/eigen \
    -DCMAKE_C_COMPILER=~/opt/usr/local/bin/gcc \
    -DCMAKE_CXX_COMPILER=~/opt/usr/local/bin/g++ \
    .. >> install.log 2>&1 &
```

Edit the following files:

- `husky/base/generation_lock.cpp`
- `husky/base/generation_lock.hpp`

And include the following line in the headers at the top.

```cpp
#include <functional>
```

________________

## Installation of Husky on Abel Cluster

Following details are about [Abel cluster] at [University of Oslo, Norway]. Similar steps may be useful for other HPC clusters.

[Abel cluster]: https://www.uio.no/english/services/it/research/hpc/abel/
[University of Oslo, Norway]: https://www.uio.no/

### Dependencies and Modules

Check for available modules.

```bash
module avail cmake
module load cmake

module avail zeromq
module load zeromq

module avail boost
module load boost/1.69.0

module avail gcc
module load gcc/7.2.0

module avail Autoconf
module load Autoconf/2.69

module avail gperf

module list
```

We use following modules:

```bash
module load boost/1.69.0 cmake/3.13.1
```


#### GCC

```bash
cd ~/downloads
wget ftp://ftp.lip6.fr/pub/gcc/releases/gcc-8.3.0/gcc-8.3.0.tar.gz
tar xzvf gcc-8.3.0.tar.gz
cd gcc-8.3.0/
./contrib/download_prerequisites
./configure --disable-multilib --enable-languages=c,c++
# following will take hours!
nohup make -j 4 >> install.log 2>&1 &
tail -F install.log
export DESTDIR="$HOME/.local/opt/" &&  make install
```

#### Boost Libraries

Husky project relies on Boost libraries that need to be separately built. Instead of manually installing Boost, try loading the module.

When `cmake` reports the error `Could NOT find Boost`, you can give the following _hints_ to `cmake` (if you manually built Boost).

```bash
    -DBOOST_ROOT=$HOME/.local/opt/boost \
    -DBOOST_LIBRARYDIR==$HOME/.local/opt/boost/lib \
    -DBOOST_INCLUDEDIR=$HOME/.local/opt/boost/include \
```

#### ZeroMQ

Both [libzmq] and [cppzmq] must be present.

##### libzmq

```bash
module load Autoconf/2.69
cd ~/downloads
wget https://github.com/zeromq/libzmq/archive/master.zip -O libzmq-source.zip
unzip libzmq-source.zip
mv libzmq-master/ ~/.local/opt/libzmq-source

cd ~/.local/opt/libzmq-source
./autogen.sh
module load Autoconf/2.69
mkdir release
cd release
cmake -G "Unix Makefiles" -D WITH_PERF_TOOL=OFF -D ZMQ_BUILD_TESTS=OFF -D CMAKE_BUILD_TYPE=Release ..
make
export DESTDIR="$HOME/.local/opt/" && make -j4 install
```

Following folders will get created:

- `$HOME/.local/opt/usr/local/lib64`
- `$HOME/.local/opt/usr/local/share/zmq`
- `$HOME/.local/opt/usr/local/share/cmake/ZeroMQ`

##### cppzmq

```bash
cd ~/downloads
wget https://github.com/zeromq/cppzmq/archive/master.zip -O cppzmq-source.zip
unzip cppzmq-source.zip
mv cppzmq-master/ cppzmq

cd cppzmq
mkdir build
cd build
# Hint to CMake to find ZeroMQ here
cmake -DCMAKE_PREFIX_PATH="$HOME/.local/opt/usr/local/" -DCPPZMQ_BUILD_TESTS=OFF ..
export DESTDIR="$HOME/.local/opt/" &&  make -j4 install
```

### GPerfTools

```bash
cd ~/downloads
wget https://github.com/gperftools/gperftools/archive/master.zip -O gperftools-source.zip
# wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.7/gperftools-2.7.tar.gz
unzip gperftools-source.zip
mv gperftools-master/ ~/.local/opt/gperftools
cd ~/.local/opt/gperftools
./autogen.sh
./configure
./configure --enable-frame-pointers
make
export DESTDIR="$HOME/.local/opt/" &&  make install
```

- [libunwind](http://www.nongnu.org/libunwind/)

```bash
cd ~/downloads
wget http://download.savannah.nongnu.org/releases/libunwind/libunwind-1.3.1.tar.gz
tar xzvf libunwind-1.3.1.tar.gz
mv libunwind-1.3.1 ~/.local/opt/
cd ~/.local/opt/libunwind-1.3.1/
./configure
make
export DESTDIR="$HOME/.local/opt/" &&  make install
## make install prefix=~/.local/opt (doesn't work)
```

### Summary of Installation Notes

```bash
module purge
module load gcc/7.2.0
module load cmake
module load boost/1.69.0
module load Autoconf/2.69

cd ~/downloads
wget https://github.com/husky-team/husky/archive/master.zip -O husky-source.zip
unzip husky-source.zip
mv -v husky-master ~/.local/opt/husky
export HUSKY_ROOT=~/.local/opt/husky

# Eigen
cd ~/downloads
wget http://bitbucket.org/eigen/eigen/get/3.3.7.tar.bz2
tar jxvf 3.3.7.tar.bz2
mv eigen-eigen-323c052e1731 ~/.local/opt/eigen

# Boost
module load boost/1.69.0

## ZeroMQ
## See above

## GPerfTools
## See above

# Husky
cd $HUSKY_ROOT
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release \
    -DEIGEN_INCLUDE_DIR=$HOME/.local/opt/eigen \
    -DCMAKE_PREFIX_PATH="$HOME/.local/opt/usr/local/" \
    .. >> install.log 2>&1 &
make help
make -j4 Master
make husky
```

Or, with more details to `cmake`:

```bash
export CC=$HOME/.local/opt/usr/local/bin/gcc
export CXX=$HOME/.local/opt/usr/local/bin/g++
cmake -DCMAKE_BUILD_TYPE=Release \
    -D CMAKE_C_COMPILER=$HOME/.local/opt/usr/local/bin/gcc \
    -D CMAKE_CXX_COMPILER=$HOME/.local/opt/usr/local/bin/g++ \
    -DEIGEN_INCLUDE_DIR=$HOME/.local/opt/eigen \
    -DCMAKE_PREFIX_PATH="$HOME/.local/opt/usr/local/" \
    ..

CC=$HOME/.local/opt/usr/local/bin/gcc CXX=$HOME/.local/opt/usr/local/bin/g++ make -j4 Master
CC=$HOME/.local/opt/usr/local/bin/gcc CXX=$HOME/.local/opt/usr/local/bin/g++ make -j4 husky
```

### Troubleshooting

#### C compiler cannot create executables

Make sure you have only just one and the latest `gcc` loaded and in path. Make sure `CC` and `CXX` flags are not pointing to OpenMPI.
