Implement iotcl_dra_json_config.h and iotcl_dra_json_config.c to parse the iotcDeviceConfig.json.
Use the json parsing patterns like the ones in #iotcl_dra_discovery.c:iotcl_dra_parse_discovery_json, iotcl_dra_identity.c:iotcl_dra_parse_response_and_configure_iotcl, iotc_c2d.c:iotcl_c2d_parse_json.

Regarding fields, return strduped values in IotclDraJsonConfigResult:
- "ver": must be "2.1". Fail if not.
- "pf" must be either IOTCL_PF_AWS_STR or IOTCL_PF_AZURE_STR from #iotcl.h
- "cpid": return and ensure non-empty, non-null
- "env": return into struct and ensure non-empty, non-null
- "uid": return as "duid" string in the struct and ensure non-empty, non-null
- "did": ignore, BUT return a new boolean value into the struct "dedicated_instance" which is true if and only if strlen(uid)==strlen("did")
- "at": ignore
- "sk": ignore
- "disc": ignore



----------- long form json:
{
  "ver": "2.1",
  "pf": "az",
  "cpid": "avtds",
  "env": "avnetpoc",
  "uid": "left2myowndevice",
  "did": "avtds-left2myowndevice",
  "at": 5,
  "sk": "cGFzc3dvcmRwYXNzd29yZAo=",
  "disc": "https://discovery.iotconnect.io/"
}

------ short form json:
{"ver":"2.1","pf":"aws","cpid":"48b14f8b0cb24d029c1573e36ee31e49","env":"prod","uid":"aBRITE","did":"48b14f8b0cb24d029c1573e36ee31e49-aBRITE","at":3,"disc":"https://discovery.iotconnect.io"}