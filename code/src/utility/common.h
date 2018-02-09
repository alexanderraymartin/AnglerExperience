#pragma once
#ifndef COMMON_H_
#define COMMON_H_

#define MAX(_A, _B) ((_A) > (_B) ? (_A) : (_B))
#define MIN(_A, _B) ((_A) < (_B) ? (_A) : (_B))
#define CLAMP(_A, _MIN, _MAX) ( _A > _MAX ? (_MAX) : (_A < _MIN ? (_MIN) : (_A) ) )

#define _STRIFY(_PD) #_PD
#define STRIFY(_PD) _STRIFY(_PD)

#define PING() {fprintf(stderr,"PING! (%d:%s)\n",__LINE__,__FILE__);}

#define UINT unsigned int
#define ULONG unsigned long
#define USHORT unsigned short
#define UCHAR unsigned char

#endif