#! /bin/bash

# Remove the "3.15.9" suffix from dependency names of libca.so
patchelf --replace-needed libCom.so.3.15.9 libCom.so libca.so

# Remove the "3.15.9" suffix from dependency names of libcas.so
patchelf --replace-needed libca.so.3.15.9 libca.so libcas.so
patchelf --replace-needed libgdd.so.3.15.9 libgdd.so libcas.so
patchelf --replace-needed libCom.so.3.15.9 libCom.so libcas.so

# Remove the "3.15.9" suffix from dependency names of libgdd.so
patchelf --replace-needed libCom.so.3.15.9 libCom.so libgdd.so

# Modify the RUNPATH for all the shared libraries to the current directory
patchelf --set-rpath '$ORIGIN' libca.so
patchelf --set-rpath '$ORIGIN' libcas.so
patchelf --set-rpath '$ORIGIN' libCom.so
patchelf --set-rpath '$ORIGIN' libgdd.so
