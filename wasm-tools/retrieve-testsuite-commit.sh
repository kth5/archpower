#!/bin/bash

TAG="$1"

if [ -z "$1" ]
  then
    echo "Usage: $0 <pkgver>"
    exit 1
fi

# For Wasm testsuite tests
CONTENTS_TESTSUITE=$(curl -s "https://api.github.com/repos/bytecodealliance/wasm-tools/contents/tests/testsuite?ref=v$TAG")

# Ensure that retrieved object is indeed a submodule
CONTENT_TYPE=$(echo "$CONTENTS_TESTSUITE" | jq -r '.type')
if [ "$CONTENT_TYPE" != 'submodule' ]; then
    echo "Path for testsuite is not a submodule"
    exit 1
fi

# Get the SHA of the submodule
SUBMODULE_SHA=$(echo "$CONTENTS_TESTSUITE" | jq -r '.sha')
echo "SHA for testsuite: $SUBMODULE_SHA"


# For component model tests
CONTENTS_COMPONENT_MODEL=$(curl -s "https://api.github.com/repos/bytecodealliance/wasm-tools/contents/tests/component-model?ref=v$TAG")

# Ensure that retrieved object is indeed a submodule
CONTENT_TYPE=$(echo "$CONTENTS_COMPONENT_MODEL" | jq -r '.type')
if [ "$CONTENT_TYPE" != 'submodule' ]; then
    echo "Path for component model is not a submodule"
    exit 1
fi

# Get the SHA of the submodule
SUBMODULE_SHA=$(echo "$CONTENTS_COMPONENT_MODEL" | jq -r '.sha')
echo "SHA for component-model: $SUBMODULE_SHA"

