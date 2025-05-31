#!/usr/bin/bash

update_key() {
  key=$1
  unbound-anchor -v -a root.key ||
  unbound-anchor -v -a root.key

  unbound-host -v -f root.key -t DNSKEY . |
  sed 's/ (secure)//;t;d' |
  sed 's/ has / IN /' |
  sed 's/ record / /' |
  grep "257 3" \
  > "${key}"

  # Cleanup created root.key
  rm root.key
}

update_key trusted-key-$(date +%Y%m%d).notkey
