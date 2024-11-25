#ifndef RTDB_H
#define RTDB_H

#include "rtdb_vars.h"

// ASSIGN DURING INIT
void rtdb_assign_tU8S ( tU8S* VarAddrs, tU32 Indx, unitEnumT Unit, tU8 Min, tU8 Def, tU8 Max, char* Comment );
void rtdb_assign_tU16S( tU16S* VarAddrs, tU32 Indx, unitEnumT Unit, tU16 Min, tU16 Def, tU16 Max, char* Comment );
void rtdb_assign_tU32S( tU32S* VarAddrs, tU32 Indx, unitEnumT Unit, tU32 Min, tU32 Def, tU32 Max, char* Comment );

void rtdb_assign_tS8S ( tS8S* VarAddrs, tU32 Indx, unitEnumT Unit, tS8 Min, tS8 Def, tS8 Max, char* Comment );
void rtdb_assign_tS16S( tS16S* VarAddrs, tU32 Indx, unitEnumT Unit, tS16 Min, tS16 Def, tS16 Max, char* Comment );
void rtdb_assign_tS32S( tS32S* VarAddrs, tU32 Indx, unitEnumT Unit, tS32 Min, tS32 Def, tS32 Max, char* Comment );

void rtdb_assign_tES   ( tES*  VarAddrs, tU32 Indx, unitEnumT Unit, tU32 Min, tU32 Def, tU32 Max, char* Comment );
void rtdb_assign_tBS   ( tBS*  VarAddrs, tU32 Indx, unitEnumT Unit, tU8 Min, tU8 Def, tU8 Max, char* Comment );
void rtdb_assign_tF32S ( tF32S* VarAddrs, tU32 Indx, unitEnumT Unit, float Min, float Def, float Max, char* Comment );


// WRITE DURING RUNTIME
tU32 rtdb_write_tU8S ( tU8S* VarAddrs, tU8 NewVal );
tU32 rtdb_write_tU16S( tU16S* VarAddrs, tU16 NewVal );
tU32 rtdb_write_tU32S( tU32S* VarAddrs, tU32 NewVal );

tU32 rtdb_write_tS8S ( tS8S* VarAddrs, tS8 NewVal );
tU32 rtdb_write_tS16S( tS16S* VarAddrs, tS16 NewVal );
tU32 rtdb_write_tS32S( tS32S* VarAddrs, tS32 NewVal );

tU32 rtdb_write_tES   ( tES*  VarAddrs, tS32 NewVal );
tU32 rtdb_write_tBS   ( tBS*  VarAddrs, tS8 NewVal );
tU32 rtdb_write_tF32S ( tF32S* VarAddrs, float NewVal );

// OVERWRITE DURING RUNTIME
tU32 rtdb_overwrite_tU8S ( tU8S* VarAddrs, tU8 NewVal );
tU32 rtdb_overwrite_tU16S( tU16S* VarAddrs, tU16 NewVal );
tU32 rtdb_overwrite_tU32S( tU32S* VarAddrs, tU32 NewVal );

tU32 rtdb_overwrite_tS8S ( tS8S* VarAddrs, tS8 NewVal );
tU32 rtdb_overwrite_tS16S( tS16S* VarAddrs, tS16 NewVal );
tU32 rtdb_overwrite_tS32S( tS32S* VarAddrs, tS32 NewVal );

tU32 rtdb_overwrite_tES   ( tES*  VarAddrs, tS32 NewVal );
tU32 rtdb_overwrite_tBS   ( tBS*  VarAddrs, tS8 NewVal );
tU32 rtdb_overwrite_tF32S ( tF32S* VarAddrs, float NewVal );


// READ DURING RUNTIME
tU8  rtdb_read_tU8S ( tU8S* VarAddrs );
tU16 rtdb_read_tU16S( tU16S* VarAddrs );
tU32 rtdb_read_tU32S( tU32S* VarAddrs );

tS8  rtdb_read_tS8S ( tS8S* VarAddrs );
tS16 rtdb_read_tS16S( tS16S* VarAddrs );
tS32 rtdb_read_tS32S( tS32S* VarAddrs );

tE    rtdb_read_tES   ( tES*  VarAddrs );
tB    rtdb_read_tBS   ( tBS*  VarAddrs );
tF32  rtdb_read_tF32S ( tF32S* VarAddrs );


// AUXILLIARY FUNCTIONS
tU32 rtdb_checkCrc32 (void* VarAddrs, tU32 VarSize );
tU32 rtdb_calcCrc32  (void* VarAddrs, tU32 VarSize );

#endif
