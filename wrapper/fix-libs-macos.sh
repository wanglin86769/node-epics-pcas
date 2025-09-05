#!/bin/bash

# Step 1: Change the install name of each library to use @loader_path
install_name_tool -id "@loader_path/libcas.dylib"   libcas.dylib
install_name_tool -id "@loader_path/libca.dylib"   libca.dylib
install_name_tool -id "@loader_path/libgdd.dylib"  libgdd.dylib
install_name_tool -id "@loader_path/libCom.dylib"  libCom.dylib

# Step 2: Fix libcas dependencies
install_name_tool -change "/Users/wanglin/epics/base-3.15.9_wrapper/lib/darwin-x86/libca.3.15.9.dylib"  "@loader_path/libca.dylib"  libcas.dylib
install_name_tool -change "/Users/wanglin/epics/base-3.15.9_wrapper/lib/darwin-x86/libgdd.3.15.9.dylib" "@loader_path/libgdd.dylib" libcas.dylib
install_name_tool -change "/Users/wanglin/epics/base-3.15.9_wrapper/lib/darwin-x86/libCom.3.15.9.dylib" "@loader_path/libCom.dylib" libcas.dylib

# Step 3: Fix libca dependency
install_name_tool -change "/Users/wanglin/epics/base-3.15.9_wrapper/lib/darwin-x86/libCom.3.15.9.dylib" "@loader_path/libCom.dylib" libca.dylib

# Step 4: Fix libgdd dependency
install_name_tool -change "/Users/wanglin/epics/base-3.15.9_wrapper/lib/darwin-x86/libCom.3.15.9.dylib" "@loader_path/libCom.dylib" libgdd.dylib