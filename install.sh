#!/bin/bash

set -euo pipefail

if [[ ${EUID} -ne 0 ]]; then
    echo "This script must be run as root" 1>&2
    exit 1
fi

# cp -v interceptkb /usr/local/bin/
cp -v 95-pedal.rules /etc/udev/rules.d/
cp -v interceptkb.service /etc/systemd/system/
