### Introduction
This directory contains a sample script that generates a a key and a matching self-signed certificate
using openssl. This script can be used with any posix shell compatible environment that has openssl, xxd and sed installed.

If you wish to try using CA Certificate based device authentication, follow the instructions in the
tools/cert-generation-ca directory.

### Usage
Invoke gen-self-signed-certs.sh <name>, where name will be used a Common Name (CN) when generating the 
certificate and will be used to name the files. For example:
```shell script
./gen-self-signed-certs.sh mydevice
```
This will generate a key and a certificate with 
both 4096-bit RSA and prime256v1 Elliptic Curve Cryptography (ECC) algorithms.

```text
mydevice_ec_crt.pem     ECC certificate in pem format
mydevice_ec_key.pem     ECC private key in pem format
mydevice_ec_fp.txt      SHA256 Fingerprint (Thumbprint) of the certificate
mydevice_ec_crt.der     ECC certificate in binary/der format
mydevice_ec_key.der     ECC private key in binary/der format
mydevice_ec_crt_pem.c   ECC certificate pem as c string
mydevice_ec_key_pem.c   ECC private key pem as c string
mydevice_ec_crt_der.c   ECC certificate der as c array
mydevice_ec_key_der.c   ECCe private key der as c array

mydevice_rsa_* ...      RSA based private key and cert files - same as above 
```

We recommend using ECC certificates. ECC has a smaller memory footprint, 
it is more secure and generally faster than than RSA. 
However not all devices support ECC out of the box, and RSA is more generally available.

When presenting Server CA certificates to your MQTT client, use the following:
- For AWS, use certificates form the [Amazon Trust Repository](https://www.amazontrust.com/repository/):
    - Amazon Root CA 3: If using ECC with ECDHE-ECDSA ciphers.
    - Amazon Root CA 1: If using RSA or ECC with ECDHE-RSA ciphers.
    - Starfield Services Root Certificate Authority - G2: If the device cannot process intermediates as root CAs.
- For Azure, use the 
[DigiCert Global Root G2](https://cacerts.digicert.com/DigiCertGlobalRootG2.crt.pem)
 from [this page](https://www.digicert.com/kb/digicert-root-certificates.htm).

To use a different size RSA keys or a different curve or certificate expiration, 
modify the script variables at the script heading.
 
