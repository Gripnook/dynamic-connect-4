#!/bin/bash

if cd gserver; then
    javac DC4Server.java &&\
    jar cfe DC4Server.jar DC4Server *.class &&\
    rm --interactive=never *.class &&\
    java -jar DC4Server.jar

    cd ..
fi
