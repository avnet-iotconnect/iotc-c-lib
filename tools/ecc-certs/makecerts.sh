#!/bin/bash
set -e

thisDir=$(dirname $0)
source "${thisDir}/iotconnect-config"

function usageAndExit {
  msg=$1
  if [[ -n "${msg}" ]]; then
    echo "Error: ${msg}"
  fi
  echo Usage:
  cat << END
$0 <command> ...
 command:
  root - Creates a root certificate per config file
  verify <code> - Creates a a verification certificate with supplied code as Common Name
  deice <name> - Creates an ECC device certificate and private key with specified name
END

  exit 250
}

function pem2c {
  pemfile=$1
  sed 's#^\(.*\)$#"\1\\r\\n"#g' $pemfile
}

function genSubject {
  local commonName=$1
  subject="/C=${X509_C}/ST=${X509_ST}/L=${X509_L}/O=${X509_O}/OU=${X509_OU}/CN=${commonName}"
  echo "$subject"
}
function root {
  openssl ecparam -name secp256k1 -genkey -noout -out "certs/${X509_CN}-key.pem"
  openssl req -new -x509 -key "certs/${X509_CN}-key.pem" -out "certs/${X509_CN}-crt.pem" \
    -days ${X509_VALIDITY} -subj "$(genSubject ${X509_CN})"
  echo "----------- Created Certificate -----------"
  openssl x509 -noout -subject -in "certs/${X509_CN}-crt.pem"
  echo "-------------------------------------------"
}

function verify {
  local code=$1
  if [[ -z "${code}" ]]; then
     usageAndExit "Verification code argument is required"
  fi
  openssl ecparam -name secp256k1 -genkey -noout -out certs/${code}-verify-key.pem
  openssl req -new -key certs/${code}-verify-key.pem -out certs/${code}-verify.csr \
    -subj "$(genSubject ${code})"
  openssl x509 -req \
    -in "certs/${code}-verify.csr" \
    -CA "certs/${X509_CN}-crt.pem" \
    -CAkey "certs/${X509_CN}-key.pem" \
    -CAcreateserial \
    -out "certs/${code}-verify-crt.pem" \
    -days 2 -sha256
  rm -f "certs/${code}-verify-key.pem" "certs/${code}-verify.csr"
}

function device {
  local name=$1
  if [[ -z "${name}" ]]; then
     usageAndExit "Device name argument is required"
  fi
  openssl ecparam -name secp256k1 -genkey -noout -out "${CPID}/${name}-key.pem"
  openssl req -new -key "${CPID}/${name}-key.pem" -out "${CPID}/${name}.csr" \
    -subj "$(genSubject "${CPID}-${name}")"

  openssl x509 -req \
    -in "${CPID}/${name}.csr" \
    -CA "certs/${X509_CN}-crt.pem" \
    -CAkey "certs/${X509_CN}-key.pem" \
    -set_serial 01 \
    -out "${CPID}/${name}-crt.pem" \
    -days ${X509_VALIDITY}

  f="${CPID}/${name}-c-snippets.txt"
  echo > $f
  pem2c "${CPID}/${name}-key.pem" >> $f
  pem2c "${CPID}/${name}-crt.pem" >> $f
  rm -f "${CPID}/${name}.csr"
}

mkdir -p certs
if [ -n "$CPID" ]; then
  mkdir -p "${CPID}"
fi

rm -f certs/*-verify-.*

command=$1
shift
if [ -z "$command" ]; then
  usageAndExit "Command is required"
fi
eval ${command} $@
echo Success