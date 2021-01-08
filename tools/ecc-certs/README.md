### Introduction
This directory contains the scripts and sample certificates that are tested to work with Azure IoTHub.

**IMPORTANT:** This script is provided with ease of use in mind. 
Private keys generated with this script are not password protected and are insecure, hence they should not be used in production.
Learn more about password protecting your root certificates from openssl user manual and modify commands on the makecerts.sh script accordingly.

## File and directory overview
makecerts.sh - Main script used to make root CA files and device certificates and keys. See below for instructions.
iotconnect-config - Configuration file, which will be sued to generate certificates and keys.
pem-to.sh - Utlity script that can be used to generate C constants from pem files.

certs/<subject>-crt.pem - Root CA Certificates with subject names used for file names
certs/<subject>-key.pem - Root CA Private Keys with subject names used for file names
certs/<subject>-crt.srl - Serial number intermediate files that openssl may use. 

CPID/<devname>-crt.pem - Device certificates per device.
CPID/<devname>-key.pem - Device private keys.
CPID/<devname>-c-snippets.txt - Generated snippets for testing certificates in C code

## makecerts.sh instructions

##### Initial Setup

* Obtain your CPID by copying it from the IoTConnect UI web page at Settings->Company Profile.
* Populate values in iotconnect-config per comments for individual fields in the actual file.

##### Creating a New Certificate:

* Ensure that initial setup steps are complete above.
* Create a new certificate, invoke *"./makecerts.sh root"*.
* To upload the certificate at IoTConnect, go to Device->Certificates page and upload the <subject>-crt.pem file generated per new subject name.
* The certificate status will show "pending" and needs proof of ownership. To start this verification procedure, copy the verification code displayed next to your certificate.
* run *"./makecerts.h verify __code__"*, with pasted __code__ displayed on the site.
* click the upload icon on the right side next to your certificate.
* Browse to the file named *__code__-verify-crt.pem* and click the *Verify* button.
* The certificate status should show *Verified*.
  
##### Creating a New Device Certificate and Key
 
* Create a new device certificate and private key by invoking "*./makecerts.sh device __deviceName__*"
* A device certificate and key will be generated in your CPID directory.
* Create a new device with the same name (*__deviceName__*) at IoTConnect web site. Be sure to use a device template that uses CA Certificate authentication.

  
   
   
  


