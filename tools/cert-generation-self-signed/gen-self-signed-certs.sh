#!/bin/bash

set -e

if [ -z "$1" ]; then
  echo Need argument 1
  exit
fi

cn=$1
subj="/C=US/ST=IL/L=Chicago/O=IoTConnect/CN=${cn}"
days=36500 # 100 years
rsa_key_size=4096
ec_curve=prime256v1

openssl req -newkey rsa:${rsa_key_size} -days ${days} -nodes -x509 \
    -subj ${subj} -keyout ${cn}_rsa_key.pem -out ${cn}_rsa_crt.pem
openssl rsa -outform der -in ${cn}_rsa_key.pem -out ${cn}_rsa_key.der
openssl x509 -outform der -in ${cn}_rsa_crt.pem -out ${cn}_rsa_crt.der
xxd -i ${cn}_rsa_key.der ${cn}_rsa_key_der.c
xxd -i ${cn}_rsa_crt.der ${cn}_rsa_crt_der.c
sed 's#^\(.*\)$#"\1\\n"#g' ${cn}_rsa_key.pem > ${cn}_rsa_key_pem.c
sed 's#^\(.*\)$#"\1\\n"#g' ${cn}_rsa_crt.pem > ${cn}_rsa_crt_pem.c
fp=$(openssl x509 -noout -fingerprint -sha256 -inform pem -in ${cn}_rsa_crt.pem)
echo $fp | sed 's#:##g' > ${cn}_rsa_fp.txt


openssl ecparam -name ${ec_curve} -genkey -noout -out ${cn}_ec_key.pem
openssl req -new -days ${days} -nodes -x509 \
    -subj ${subj} -key ${cn}_ec_key.pem -out ${cn}_ec_crt.pem
openssl ec -outform der -in ${cn}_ec_key.pem -out ${cn}_ec_key.der
openssl x509 -outform der -in ${cn}_ec_crt.pem -out ${cn}_ec_crt.der
xxd -i ${cn}_ec_key.der ${cn}_ec_key_der.c
xxd -i ${cn}_ec_crt.der ${cn}_ec_crt_der.c
sed 's#^\(.*\)$#"\1\\n"#g' ${cn}_ec_key.pem > ${cn}_ec_key_pem.c
sed 's#^\(.*\)$#"\1\\n"#g' ${cn}_ec_crt.pem > ${cn}_ec_crt_pem.c
fp=$(openssl x509 -noout -fingerprint -sha256 -inform pem -in ${cn}_ec_crt.pem)
echo $fp | sed 's#:##g' > ${cn}_ec_fp.txt