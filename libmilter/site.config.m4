dnl Enable local CFLAGS and LDFLAGS
define(`confLDOPTS',`${LDFLAGS}')
define(`confOPTIMIZE',`${CFLAGS}')

dnl Enable libmilter with a pool of workers
APPENDDEF(`conf_libmilter_ENVDEF',`-D_FFR_WORKERS_POOL=1')
APPENDDEF(`conf_libmilter_ENVDEF',`-DMIN_WORKERS=4')

dnl Use poll instead of select
APPENDDEF(`conf_libmilter_ENVDEF',`-DSM_CONF_POLL=1')

dnl Enable IPv6
APPENDDEF(`conf_libmilter_ENVDEF',`-DNETINET6=1')
APPENDDEF(`conf_libmilter_ENVDEF', `-shared -pthread')

dnl Permissions
APPENDDEF(`confINCGRP',`root')
APPENDDEF(`confLIBGRP',`root')
APPENDDEF(`confMBINGRP',`root')
APPENDDEF(`confSBINGRP',`root')
APPENDDEF(`confUBINGRP',`root')
APPENDDEF(`confUBINOWN', `root')

dnl Allow stripping by giving write permission
define(`confGBINMODE',`2755')
define(`confLIBMODE',`644')

dnl Force libmilter to use stdbool.h for GCC23+
APPENDDEF(`confCCOPTS', `-DSM_CONF_STDBOOL_H=1')
