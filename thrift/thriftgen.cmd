#!/bin/sh

#THRIFT_EXE=xrpcgen-0.9.1
THRIFT_EXE=thrift
THRIFT_VER=9

#Java
rm -f microdatabase-thrift/gen-java/*/*/*/*/*/*
#microdatabase

$THRIFT_EXE --gen java -o microdatabase-thrift microdatabase.thrift

#-------------------------------------------------------------------------------
#Java build & deploy

PROJECT_NAME=microdatabase-thrift
PROJECT_DIR=microdatabase-thrift
VERSION=1.0.0.0

#common variables
DEPLOY_JAR="$PROJECT_NAME""$THRIFT_VER"-"$VERSION".jar
DIST_DIR=dist

#build
ant -f $PROJECT_DIR/build.xml clean
ant -f $PROJECT_DIR/build.xml -Djavac.debug=false -Ddist.jar=dist/$DEPLOY_JAR jar

mkdir -p $DIST_DIR
cp -f $PROJECT_DIR/dist/$DEPLOY_JAR $DIST_DIR

#clean
ant -f $PROJECT_DIR/build.xml clean
rm -f microdatabase-thrift/gen-java/*/*/*/*/*/*
