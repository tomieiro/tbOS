#ifndef TB_LUA_PORT_H
#define TB_LUA_PORT_H

#include <stdint.h>

#include "tb_libc.h"

#define luai_jmpbuf tb_jmpbuf
#define LUAI_THROW(L,c) tb_longjmp(&(c)->b, 1)
#define LUAI_TRY(L,c,a) if (tb_setjmp(&(c)->b) == 0) { a }

#define luai_makeseed(L) ((unsigned int)(((uintptr_t)(L) >> 4) ^ 0x9E3779B9u))

#ifdef lua_getlocaledecpoint
#undef lua_getlocaledecpoint
#endif
#define lua_getlocaledecpoint() '.'

#undef l_floor
#define l_floor(x) tb_floorf(x)

#undef lua_str2number
#define lua_str2number(s,p) tb_lua_str2number((s), (p))

#ifdef lua_strx2number
#undef lua_strx2number
#endif
#define lua_strx2number(s,p) tb_lua_strx2number((s), (p))
#define luai_nummod(L,a,b,m) { (void)(L); (m) = tb_fmodf((a), (b)); if (((m) > 0) ? ((b) < 0) : (((m) < 0) && ((b) > 0))) { (m) += (b); } }
#define luai_numpow(L,a,b) ((void)(L), tb_powf((a), (b)))

#undef lua_number2str
#define lua_number2str(s,sz,n) tb_lua_number2str((s), (sz), (n))

#undef lua_integer2str
#define lua_integer2str(s,sz,n) tb_lua_integer2str((s), (sz), (n))

#undef lua_pointer2str
#define lua_pointer2str(s,sz,p) tb_lua_pointer2str((s), (sz), (p))

#endif
